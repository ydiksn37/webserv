# Webserv テストガイド

このドキュメントでは、`webserv` の動作を検証するための様々な方法をまとめています。

## 0. 準備
サーバーを起動します。
```bash
make
./webserv configurations/default.conf
```

## 1. `curl` による基本テスト
レスポンスヘッダーとボディを詳細に確認できます。

### 基本的な GET リクエスト
```bash
curl -v http://localhost:8080/
```
*   **確認点**: ステータスコード `200 OK`、`Server: Webserv/1.0` ヘッダー、プレースホルダーHTML。

### 存在しないパス (404 Not Found)
```bash
curl -v http://localhost:8080/path/to/nothing
```
*   **確認点**: ステータスコード `404 Not Found`、デフォルトのエラーページが返ってくるか。

### 許可されていないメソッド (405 Method Not Allowed)
`default.conf` では `/` は GET のみ許可されています。
```bash
curl -v -X DELETE http://localhost:8080/
```
*   **確認点**: ステータスコード `405 Method Not Allowed`、`Allow: GET` ヘッダーが含まれているか。

## 2. `nc` (Netcat) による低レイヤテスト
HTTPリクエストを1行ずつ手動で送信し、サーバーの堅牢性を確認します。

### 手動 GET リクエスト
```bash
nc localhost 8080
GET / HTTP/1.1
Host: localhost
(Enterを2回押す)
```

### 不正なリクエストライン
```bash
nc localhost 8080
BAD_METHOD / HTTP/1.1
Host: localhost
(Enterを2回押す)
```
*   **確認点**: `400 Bad Request` または `501 Not Implemented` が適切に返るか。

## 3. 負荷テスト (`siege`)
複数の同時接続に対する安定性を確認します。

```bash
# 10クライアントで10秒間負荷をかける
siege -b -c 10 -t 10s http://localhost:8080/
```
*   **確認点**: `Availability` が 100% であること、サーバーがクラッシュしないこと。

## 4. 機能追加後のテスト項目 (将来用)
今後、機能が実装された際に以下のコマンドで検証できます。

### CGI (Python/PHP等) のテスト
```bash
curl -v http://localhost:8080/cgi-bin/test.py
```

### ファイルアップロードのテスト
```bash
curl -v -X POST --data-binary @large_file.txt http://localhost:8080/upload/
```

### ディレクトリリスティングのテスト
`autoindex on` の場所にアクセスして確認します。
```bash
curl -v http://localhost:8080/images/
```

---
**ヒント**: サーバー側のログ（標準出力）を確認しながらテストを行うと、どのイベント（EPOLLIN/EPOLLOUT）がトリガーされているか把握しやすくなります。
