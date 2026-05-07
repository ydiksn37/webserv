# Webserv テストガイド

このドキュメントは、`configurations/` 配下のすべての設定ファイルを使って `webserv` の主要機能を確認するための手順です。

各テストは、次の観点を確認します。

- 設定ファイルが読み込めること
- `root test/www` 配下の静的ファイルを返せること
- location ごとの許可メソッドが効くこと
- custom error page が使われること
- upload / DELETE が動くこと
- CGI が `fork`, `pipe`, `execve` 経由で実行されること
- 複数 server / 複数 port / server_name の選択が動くこと

## 0. 共通準備

リポジトリのルートでビルドします。

```bash
make
```

サーバーは設定ファイルごとに起動します。複数の設定で同じ port を使うものがあるため、基本的には 1 つずつ起動してください。

```bash
./webserv configurations/test.conf
```

以降の `curl` コマンドは別ターミナルから実行します。サーバーを止めるときは、起動しているターミナルで `Ctrl-C` を押します。

実行環境により `php-cgi` が入っていない場合があります。その場合、PHP CGI の確認は失敗する可能性があります。Python CGI は `/usr/bin/python3` を使います。

## 1. 設定ファイル一覧

| 設定ファイル | Port | 主な確認内容 |
| --- | ---: | --- |
| `configurations/default.conf` | 8080 | 最小構成、GET のみ、custom 404 |
| `configurations/test.conf` | 8080 | GET、405、CGI、upload、DELETE、chunked body |
| `configurations/cgi.conf` | 8082 | CGI を中心に確認、upload も確認 |
| `configurations/example.conf` | 8080, 8081 | 複数 server、server_name、redirect、alias、upload、CGI、別 port |

## 2. default.conf

### 目的

最小構成で、`test/www` の静的ファイルを返せること、`/` が GET のみ許可されること、存在しないファイルで custom 404 が返ることを確認します。

### 起動

```bash
./webserv configurations/default.conf
```

### 静的ファイル GET

```bash
curl -v http://localhost:8080/
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `Webserv Test Page` が本文に含まれること
- `test/www/index.html` が配信されていること

### 存在しないファイル

```bash
curl -v http://localhost:8080/not-found
```

確認点:

- `HTTP/1.1 404 Not Found` が返ること
- `test/www/errors/404.html` の本文が返ること

### 許可されていないメソッド

```bash
curl -v -X POST http://localhost:8080/
```

確認点:

- `HTTP/1.1 405 Method Not Allowed` が返ること
- `Allow: GET` ヘッダーが含まれること

## 3. test.conf

### 目的

開発中の総合確認用です。GET、CGI、POST upload、DELETE、chunked body を 1 つの設定で確認します。

### 起動

```bash
./webserv configurations/test.conf
```

### 基本 GET

```bash
curl -v http://localhost:8080/
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `test/www/index.html` の内容が返ること

### 通常ファイル GET

```bash
curl -v http://localhost:8080/files/existing.txt
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `This file is available for GET and DELETE tests.` が本文に含まれること

### 存在しない通常ファイル

```bash
curl -v http://localhost:8080/path/to/nothing
```

確認点:

- `HTTP/1.1 404 Not Found` が返ること

### 許可されていないメソッド

`/` は GET のみ許可しています。

```bash
curl -v -X POST http://localhost:8080/
```

確認点:

- `HTTP/1.1 405 Method Not Allowed` が返ること
- `Allow: GET` ヘッダーが含まれること

### Python CGI

```bash
curl -v "http://localhost:8080/cgi-bin/hello.py?name=webserv"
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `Hello from Python CGI!` が本文に含まれること
- `QUERY_STRING: name=webserv` が本文に含まれること

このテストでは、設定の `cgi_path .py /usr/bin/python3` に従って Python interpreter が `execve` され、CGI の stdout が HTTP response として返ることを確認しています。

### 存在しない CGI

```bash
curl -v http://localhost:8080/cgi-bin/missing.py
```

確認点:

- `HTTP/1.1 404 Not Found` が返ること
- 空本文の `200 OK` にならないこと

### ファイルアップロード

`--data-binary @file` ではローカルファイル名は HTTP リクエストに含まれません。この実装では保存名を URL から決めるため、保存したいファイル名を URL に含めます。

```bash
printf 'hello upload\n' > /tmp/webserv_upload_payload.txt
curl -v -X POST --data-binary @/tmp/webserv_upload_payload.txt \
  http://localhost:8080/upload/webserv_upload_payload.txt
cmp /tmp/webserv_upload_payload.txt test/www/upload/webserv_upload_payload.txt
```

確認点:

- `HTTP/1.1 201 Created` が返ること
- `cmp` が何も出力せず終了すること
- `test/www/upload/webserv_upload_payload.txt` が作成されること

### DELETE

アップロードしたファイルを削除します。

```bash
curl -v -X DELETE http://localhost:8080/upload/webserv_upload_payload.txt
test ! -e test/www/upload/webserv_upload_payload.txt && echo deleted
```

確認点:

- `HTTP/1.1 204 No Content` が返ること
- `deleted` が表示されること

同じ URL をもう一度 DELETE すると、存在しないファイルの確認もできます。

```bash
curl -v -X DELETE http://localhost:8080/upload/webserv_upload_payload.txt
```

確認点:

- `HTTP/1.1 404 Not Found` が返ること

### Chunked body

HTTP request を手で組み立て、`Transfer-Encoding: chunked` の body が unchunk されて保存されることを確認します。

```bash
printf 'POST /upload/chunked.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nhello world\r\n0\r\n\r\n' | nc localhost 8080
```

確認点:

- `HTTP/1.1 201 Created` が返ること
- `test/www/upload/chunked.txt` に `hello world` が保存されること

確認後に削除する場合:

```bash
curl -v -X DELETE http://localhost:8080/upload/chunked.txt
```

## 4. cgi.conf

### 目的

CGI を中心に確認する設定です。`test.conf` と違い port 8082 を使うため、port 8080 の設定と分けて確認しやすいです。

### 起動

```bash
./webserv configurations/cgi.conf
```

### 静的ファイル GET

```bash
curl -v http://localhost:8082/
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `Webserv Test Page` が本文に含まれること

### Python CGI GET

```bash
curl -v "http://localhost:8082/cgi-bin/hello.py?review=cgi"
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `Hello from Python CGI!` が本文に含まれること
- `QUERY_STRING: review=cgi` が本文に含まれること

### Python CGI POST

```bash
curl -v -X POST --data 'hello cgi body' \
  "http://localhost:8082/cgi-bin/hello.py?mode=post"
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `REQUEST_METHOD: POST` が本文に含まれること

このテストでは、client body が CGI stdin に渡され、CGI process が POST として実行されることを確認します。

### PHP CGI

```bash
curl -v "http://localhost:8082/cgi-bin/hello.php?review=php"
```

確認点:

- `php-cgi` がインストールされていれば `HTTP/1.1 200 OK` が返ること
- `Hello from PHP CGI!` が本文に含まれること

環境に `/usr/bin/php-cgi` がない場合、このテストは失敗する可能性があります。その場合は Python CGI の結果を主な CGI 確認として使ってください。

### CGI upload location

```bash
curl -v --data 'from cgi.conf upload route' \
  http://localhost:8082/upload/cgi_conf_upload.txt
cat test/www/upload/cgi_conf_upload.txt
```

確認点:

- `HTTP/1.1 201 Created` が返ること
- `cat` で `from cgi.conf upload route` が表示されること

確認後に削除します。

```bash
curl -v -X DELETE http://localhost:8082/upload/cgi_conf_upload.txt
```

## 5. example.conf

### 目的

レビューで見せるための総合設定です。複数 server、同一 port の `server_name` 選択、別 port、redirect、alias、upload、CGI を確認できます。

### 起動

```bash
./webserv configurations/example.conf
```

### localhost server

`Host: localhost` は最初の server block に一致します。

```bash
curl -v http://localhost:8080/
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `Webserv Test Page` が本文に含まれること

### server_name による server 選択

`example.conf` には同じ port 8080 に複数 server があります。`Host` ヘッダーで server を選択できることを確認します。

```bash
curl -v -H 'Host: example.com' http://127.0.0.1:8080/
curl -v -H 'Host: www.example.com' http://127.0.0.1:8080/
curl -v -H 'Host: mytest.local' http://127.0.0.1:8080/
```

確認点:

- いずれも `HTTP/1.1 200 OK` が返ること
- server_name が一致しない場合は、その port の最初の server が default server として使われること

default server の確認:

```bash
curl -v -H 'Host: unknown.local' http://127.0.0.1:8080/
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- port 8080 の最初の server block が使われること

### 別 port の server

`example.conf` は port 8081 でも listen します。

```bash
curl -v -H 'Host: webserv.local' http://127.0.0.1:8081/
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- port 8081 の server block が使われること

### Redirect

`/old-page` は `/new-page.html` へ redirect します。

```bash
curl -v -H 'Host: www.example.com' http://127.0.0.1:8080/old-page
```

確認点:

- `HTTP/1.1 301 Moved Permanently` が返ること
- `Location: /new-page.html` ヘッダーが含まれること

### Alias

`/kapouet` は `test/www/kapouet` に alias されています。URL の location prefix を外して、alias 先のファイルを探すことを確認します。

```bash
curl -v -H 'Host: www.example.com' http://127.0.0.1:8080/kapouet/pouic.txt
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `This file is served through the /kapouet alias.` が本文に含まれること

### Upload

```bash
curl -v -H 'Host: www.example.com' --data 'example upload' \
  http://127.0.0.1:8080/upload/example_upload.txt
cat test/www/upload/example_upload.txt
```

確認点:

- `HTTP/1.1 201 Created` が返ること
- `cat` で `example upload` が表示されること

確認後に削除します。

```bash
curl -v -H 'Host: www.example.com' -X DELETE \
  http://127.0.0.1:8080/upload/example_upload.txt
```

### CGI

```bash
curl -v -H 'Host: www.example.com' \
  "http://127.0.0.1:8080/cgi-bin/hello.py?from=example"
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `Hello from Python CGI!` が本文に含まれること
- `QUERY_STRING: from=example` が本文に含まれること

## 6. 負荷テスト

`siege` が入っている環境では、単純な GET に対して負荷をかけます。これはサーバーが複数リクエストでクラッシュしないことを確認するための簡易テストです。

```bash
./webserv configurations/test.conf
```

別ターミナル:

```bash
siege -b -c 10 -t 10s http://localhost:8080/
```

確認点:

- Availability が 100% に近いこと
- サーバーがクラッシュしないこと

`siege` がない場合は、簡易的に `curl` を繰り返します。

```bash
for i in $(seq 1 100); do curl -s -o /dev/null -w "%{http_code}\n" http://localhost:8080/; done
```

確認点:

- `200` が連続して表示されること
- サーバーが落ちないこと

## 7. ユニットテスト

実装変更後は、手動テストとは別にユニットテストも実行します。

```bash
make test
```

確認点:

- HTTP request tests がすべて pass すること
- Config tests がすべて pass すること
- Engine tests がすべて pass すること
