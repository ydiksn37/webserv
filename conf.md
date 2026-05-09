# Webserv Configuration Guide

このドキュメントは、このプロジェクトの `configurations/*.conf` の読み方と、各 directive が何を制御しているかを説明するためのメモです。

conf は nginx 風の構造です。大きく分けると、`server` が「待ち受けるサーバー設定」、`location` が「URL prefix ごとのルール」です。

## 基本構造

```conf
server {
    listen 8080;
    server_name localhost;
    root test/www;
    index index.html;

    location / {
        allow_methods GET;
    }
}
```

`server` ブロックは 1 つのサイト設定です。`listen` する port、`server_name`、`root`、`error_page`、`location` などをまとめます。

`location` ブロックは、特定の URL path に対する設定です。例えば `location /upload` は `/upload/a.txt` のような URI に適用されます。

## server ブロック

```conf
server {
    ...
}
```

`server` ブロックは thread ではなく、設定単位です。

この webserv はシングルスレッドですが、`epoll` で複数の listening socket や client socket を監視できます。そのため、複数の `server` ブロックを書いても thread が増えるわけではありません。

複数 `server` ブロックには主に 2 つの用途があります。

- 複数 port で待ち受ける
- 同じ port で `Host` ヘッダーに応じて設定を切り替える

例:

```conf
server {
    listen 8080;
    server_name site-a.local;
    root test/www/site-a;
}

server {
    listen 8080;
    server_name site-b.local;
    root test/www/site-b;
}
```

どちらも port 8080 ですが、request の `Host` ヘッダーが `site-b.local` なら 2 つ目の server が使われます。

```bash
curl -i -H 'Host: site-b.local' http://127.0.0.1:8080/
```

`server_name` がどれにも一致しない場合は、その port の最初の server が default server として使われます。

## listen

```conf
listen 8080;
listen 0.0.0.0:8080;
```

待ち受ける port を指定します。

この実装では `0.0.0.0:8080` のような形式も受け付けますが、実際に使うのは port 部分です。

`Epoll` は config 内の server から port 一覧を集め、重複を除いて listen socket を作ります。同じ port の server が複数あっても、listen socket は port ごとに 1 つです。

## server_name

```conf
server_name localhost example.com;
```

HTTP request の `Host` ヘッダーと照合する名前です。

同じ port に複数 `server` がある場合、`Host` と一致した `server_name` の server を使います。

例:

```conf
server {
    listen 8080;
    server_name site-a.local;
}

server {
    listen 8080;
    server_name site-b.local;
}
```

この場合:

```bash
curl -i -H 'Host: site-a.local' http://127.0.0.1:8080/
curl -i -H 'Host: site-b.local' http://127.0.0.1:8080/
```

で、それぞれ別の server 設定が使われます。

ブラウザで `http://127.0.0.1:8080/` にアクセスした場合、`Host` は通常 `127.0.0.1:8080` になるため、`site-b.local` には一致しません。確認する場合は `curl -H 'Host: ...'` を使うか、`/etc/hosts` に名前を追加します。

## root

```conf
root test/www;
```

URI を実ファイルパスに変換するときの基準ディレクトリです。

`root` の場合、基本的には次の形になります。

```txt
root + request URI
```

例:

```conf
root test/www;
```

に対して:

```http
GET /index.html
```

なら、実際に探すファイルは:

```txt
test/www/index.html
```

です。

## index

```conf
index index.html index.htm;
```

リクエスト先が directory だった場合に探す default file です。

例:

```http
GET /
```

で `test/www/` が directory の場合、`index.html`、`index.htm` の順に探します。

見つかればその file を返します。見つからない場合、`autoindex on` なら directory listing を返し、`autoindex off` なら `403 Forbidden` を返します。

## client_max_body_size

```conf
client_max_body_size 10485760;
client_max_body_size 2M;
```

request body の最大サイズです。

POST body や chunked body がこのサイズを超えると、`413 Payload Too Large` になります。

この実装では `k`, `m`, `g` の suffix に対応しています。

## error_page

```conf
error_page 404 test/www/errors/404.html;
error_page 500 502 503 504 test/www/errors/50x.html;
```

指定した status code のときに返す custom error page です。

例:

```conf
error_page 404 test/www/errors/404.html;
```

なら、404 が発生したときに default error page ではなく `test/www/errors/404.html` の内容を body として返します。

## location ブロック

```conf
location /upload {
    allow_methods GET POST DELETE;
}
```

`location` は、特定の URL prefix に対する設定です。

この実装では longest prefix match で location を選びます。

## Longest Prefix Match

longest prefix match とは、request URI に前方一致する `location` をすべて探し、その中で一番長く一致したものを使うことです。

例:

```conf
location / {
    root test/www;
}

location /upload {
    upload_enable on;
}

location /upload/images {
    autoindex on;
}
```

request:

```http
GET /upload/file.txt
```

これは `/` と `/upload` に一致します。一番長く一致するのは `/upload` なので、`location /upload` が使われます。

request:

```http
GET /upload/images/a.png
```

これは `/`、`/upload`、`/upload/images` に一致します。一番長い `/upload/images` が使われます。

この実装では boundary check もあります。

```conf
location /api {
    ...
}
```

の場合:

- `/api`
- `/api/`
- `/api/users`

は一致します。

一方:

- `/apiary`
- `/apix`

は一致しません。文字列としては `/api` で始まりますが、`/api` の次が `/` ではないためです。

## allow_methods / allow_method

```conf
allow_methods GET POST DELETE;
allow_method GET;
```

その location で許可する HTTP method を指定します。

この実装では `GET`, `POST`, `DELETE` に対応しています。

許可されていない method が来た場合、`405 Method Not Allowed` を返します。また、`Allow` header に許可 method を入れます。

## autoindex

```conf
autoindex on;
autoindex off;
```

directory に index file がない場合、directory listing を HTML で返すかどうかを制御します。

`autoindex on` なら一覧を返します。`autoindex off` なら `403 Forbidden` です。

## return

```conf
return 301 /new-page.html;
```

その location に来た request を redirect します。

例:

```conf
location /old-page {
    return 301 /new-page.html;
}
```

に対して:

```http
GET /old-page
```

なら:

```http
HTTP/1.1 301 Moved Permanently
Location: /new-page.html
```

を返します。

## root と alias の違い

`root` と `alias` はどちらも URL を実ファイルパスに変換するための設定ですが、URI の扱い方が違います。

### root

```conf
location /images {
    root test/www/assets;
}
```

request:

```http
GET /images/logo.png
```

実ファイル:

```txt
test/www/assets/images/logo.png
```

`root` は request URI 全体を root の後ろに足します。

```txt
root + /images/logo.png
```

### alias

```conf
location /images {
    alias test/www/assets;
}
```

request:

```http
GET /images/logo.png
```

実ファイル:

```txt
test/www/assets/logo.png
```

`alias` は location prefix を alias 先に置き換えます。

```txt
alias + request URI から location prefix を除いた部分
```

つまり:

```txt
alias: test/www/assets
request URI: /images/logo.png
location prefix: /images
残り: /logo.png

test/www/assets + /logo.png
= test/www/assets/logo.png
```

この実装でも同じ考え方です。

```cpp
if (!alias.empty()) {
    return alias + req_path.substr(loc_path.length());
}
return root + req_path;
```

## upload_enable

```conf
upload_enable on;
```

その location で POST upload を許可するかどうかです。

`upload_enable off` または未指定の場合、通常の POST upload は `403 Forbidden` になります。

CGI location の POST は CGI として処理されるため、upload とは別です。

## upload_store

```conf
upload_store test/www/upload;
```

upload された request body を保存する directory です。

例:

```http
POST /upload/a.txt
```

なら:

```txt
test/www/upload/a.txt
```

に保存します。

この実装では multipart の `filename` は解析しません。URI の最後の segment を保存ファイル名として使います。

例:

```http
POST /upload/report.txt
```

なら、保存名は `report.txt` です。

```http
POST /upload/
```

のようにファイル名が URL にない場合、保存先ファイル名を決められないため `400 Bad Request` を返します。

## cgi_extension

```conf
cgi_extension .py .php;
```

CGI として扱う拡張子を指定します。

ただし、実際にどの interpreter を使うかは `cgi_path` で決まります。

## cgi_path

```conf
cgi_path .py /usr/bin/python3;
cgi_path .php /usr/bin/php-cgi;
```

拡張子ごとに、どの interpreter で実行するかを指定します。

例:

```conf
cgi_path .py /usr/bin/python3;
```

に対して:

```http
GET /cgi-bin/hello.py
```

なら、`hello.py` を `/usr/bin/python3` で `execve` します。

CGI 実行時には、request method、query string、content length、content type、script filename などを環境変数に入れます。CGI の stdout は webserv が読み取り、HTTP response として client に返します。

## 継承

`server` に書いた一部の設定は、`location` に初期値として継承されます。

継承されるもの:

- `root`
- `index`
- `client_max_body_size`
- `error_page`

例:

```conf
server {
    root test/www;
    index index.html;

    location / {
        allow_methods GET;
    }
}
```

この場合、`location /` に `root` や `index` が書かれていなくても、server の `root test/www` と `index index.html` を使います。

`location` 側に同じ directive を書いた場合は、location の値が優先されます。

## 代表例

### CGI location

```conf
location /cgi-bin {
    allow_methods GET POST;
    root test/www;
    cgi_extension .py .php;
    cgi_path .py /usr/bin/python3;
    cgi_path .php /usr/bin/php-cgi;
}
```

意味:

- `/cgi-bin/...` の URL に適用される
- GET と POST を許可
- 実ファイルは `test/www/cgi-bin/...` から探す
- `.py` は Python CGI として実行
- `.php` は PHP CGI として実行

### Upload location

```conf
location /upload {
    allow_methods GET POST DELETE;
    upload_enable on;
    upload_store test/www/upload;
}
```

意味:

- `/upload/...` の URL に適用される
- GET, POST, DELETE を許可
- POST body を `test/www/upload` に保存
- DELETE で該当ファイルを削除できる

## configurations 配下の用途

### default.conf

最小構成です。port 8080 で `test/www` を配信し、`/` は GET のみ許可します。

### test.conf

開発・確認用の総合設定です。GET、CGI、upload、DELETE を確認できます。

### cgi.conf

CGI 確認用です。port 8082 を使うため、port 8080 の設定と分けて試しやすいです。

### example.conf

レビュー用の総合例です。複数 server、同一 port の `server_name` 選択、別 port、redirect、alias、upload、CGI を確認できます。
