# Webserv テストガイド

このドキュメントでは、`webserv` の主要機能を `curl` で確認する手順をまとめています。

## 0. 準備

リポジトリのルートでビルドし、テスト用設定でサーバーを起動します。

```bash
make
./webserv configurations/test.conf
```

以降のコマンドは別ターミナルから実行します。`configurations/test.conf` は `localhost:8080` で GET、CGI、POST upload、DELETE を確認できる設定です。

## 1. 基本 GET

```bash
curl -i http://localhost:8080/
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- `test/www/index.html` の内容が返ること

## 2. 存在しない通常ファイル

```bash
curl -i http://localhost:8080/path/to/nothing
```

確認点:

- `HTTP/1.1 404 Not Found` が返ること

## 3. 許可されていないメソッド

`/` は GET のみ許可しています。

```bash
curl -i -X POST http://localhost:8080/
```

確認点:

- `HTTP/1.1 405 Method Not Allowed` が返ること
- `Allow: GET` ヘッダーが含まれること

## 4. CGI

### 存在する CGI

```bash
curl -i "http://localhost:8080/cgi-bin/hello.py?name=webserv"
```

確認点:

- `HTTP/1.1 200 OK` が返ること
- Python CGI の出力が本文に含まれること

### 存在しない CGI

```bash
curl -i http://localhost:8080/cgi-bin/rrr.py
```

確認点:

- `HTTP/1.1 404 Not Found` が返ること
- 空本文の `200 OK` にならないこと

## 5. ファイルアップロード

`--data-binary @file` ではローカルファイル名は HTTP リクエストに含まれません。保存名は URL に含めて送ります。

```bash
printf 'hello upload\n' > /tmp/webserv_upload_payload.txt
curl -i --data-binary @/tmp/webserv_upload_payload.txt \
  http://localhost:8080/upload/webserv_upload_payload.txt
cmp /tmp/webserv_upload_payload.txt test/www/upload/webserv_upload_payload.txt
```

確認点:

- `HTTP/1.1 201 Created` が返ること
- `cmp` が何も出力せず終了すること
- `test/www/upload/webserv_upload_payload.txt` が作成されること

## 6. DELETE

アップロードしたファイルを削除します。

```bash
curl -i -X DELETE http://localhost:8080/upload/webserv_upload_payload.txt
test ! -e test/www/upload/webserv_upload_payload.txt && echo deleted
```

確認点:

- `HTTP/1.1 204 No Content` が返ること
- `deleted` が表示されること

同じ URL をもう一度 DELETE すると、存在しないファイルの確認もできます。

```bash
curl -i -X DELETE http://localhost:8080/upload/webserv_upload_payload.txt
```

確認点:

- `HTTP/1.1 404 Not Found` が返ること

## 7. Chunked body

手動で試す場合は、次のように HTTP リクエスト全体を送ります。

```bash
printf 'POST /upload/chunked.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nhello world\r\n0\r\n\r\n' | nc localhost 8080
```

確認点:

- `HTTP/1.1 201 Created` が返ること
- `test/www/upload/chunked.txt` に `hello world` が保存されること

確認後に削除する場合:

```bash
curl -i -X DELETE http://localhost:8080/upload/chunked.txt
```

## 8. 負荷テスト

`siege` が入っている環境では、単純な GET に対して負荷をかけます。

```bash
siege -b -c 10 -t 10s http://localhost:8080/
```

確認点:

- Availability が 100% に近いこと
- サーバーがクラッシュしないこと
