# Webserv 自己テストガイド (eval.me.md)

このドキュメントは、`eval.md` の評価項目に基づき、自分で動作確認を行うための手順書です。
すべてのテストは `docker-compose up --build` でサーバーが起動していることを前提としています。

---

## 1. 設定ファイル (Configuration)

### 異なるインターフェースとポートに複数のウェブサイト
`configurations/example.conf` を使用します。

```bash
# ポート 8080 (site-localhost) の確認
curl -i http://localhost:8080/

# ポート 8081 (site-webserv-local) の確認
curl -i http://localhost:8081/
```
*   **期待結果**: それぞれ異なる `index.html` の内容が返ってくること。

### デフォルトのエラーページ (404変更テスト)
1. `configurations/example.conf` の `error_page 404 test/www/errors/404.html;` を `test/www/errors/50x.html` などに書き換えてサーバー再起動。
2. 存在しないページにアクセス。
```bash
curl -i http://localhost:8080/non-existent-page
```
*   **期待結果**: ステータス `404` と、変更した方のエラーページの内容が表示されること。

---

## 2. クライアントボディサイズの制限

### 制限テスト
1. `configurations/example.conf` の `location /upload` ブロックの `client_max_body_size` を `10` に設定して再起動。
2. 短いボディと長いボディを送信。

```bash
# 制限内 (成功)
curl -v -X POST -H "Content-Type: text/plain" --data "12345" http://localhost:8080/upload/short.txt

# 制限超過 (失敗)
curl -v -X POST -H "Content-Type: text/plain" --data "this is too long body" http://localhost:8080/upload/long.txt
```
*   **期待結果**: 制限超過時に **`413 Payload Too Large`** が返ってくること。

---

## 3. ルートとデフォルトファイル (Root & Index)

### 異なるディレクトリへのマッピング (Alias)
```bash
curl -v -H "Host: www.example.com" http://localhost:8080/kapouet/pouic.txt
```
*   **期待結果**: `test/www/kapouet/pouic.txt` の内容が返ってくること。

### デフォルトファイル (Index)
```bash
curl -v http://localhost:8080/
```
*   **期待結果**: `test/www/site-localhost/index.html` の内容が返ってくること。

---

## 4. メソッドの制限 (Allow Methods)

### 許可されていないメソッド (GETのみの場所にDELETE)
```bash
curl -v -X DELETE -H "Host: www.example.com" http://localhost:8080/index.html
```
*   **期待結果**: **`405 Method Not Allowed`** が返り、ヘッダーに **`Allow: GET`** が含まれていること。

### 許可されているメソッド (DELETEの実行)
※ `location /upload` に `root test/www;` を設定しておく必要があります。

```bash
# 作成
curl -v -X POST -d "hello" http://localhost:8080/upload/test.txt

# 削除
curl -v -X DELETE http://localhost:8080/upload/test.txt
```
*   **期待結果**: 削除時に `204 No Content` または `200 OK` が返り、ファイルが消えること。

---

## 5. 基本チェック (Basic checks)

### GET, POST, DELETE リクエストの基本動作
```bash
# GET: ファイルの取得
curl -v http://localhost:8080/files/existing.txt

# POST: 新規ファイルのアップロード
curl -v -X POST -d "hello basic test" http://localhost:8080/upload/basic.txt

# DELETE: ファイルの削除
curl -v -X DELETE http://localhost:8080/upload/basic.txt
```
*   **期待結果**: すべて適切なステータスコード（200, 201, 204など）が返り、動作すること。

### 未知のメソッド (UNKNOWN)
```bash
curl -v -X UNKNOWN http://localhost:8080/
```
*   **期待結果**: サーバーがクラッシュせず、`501 Not Implemented` または `405 Method Not Allowed` が返ること。

---

## 6. CGIのチェック

### GET と POST のテスト
```bash
# GET (Query String)
curl -v "http://localhost:8082/cgi-bin/hello.py?name=test"

# POST (Body)
curl -v -X POST -d "name=test" http://localhost:8082/cgi-bin/hello.py
```
*   **期待結果**: Pythonスクリプトが実行され、適切なHTMLが返ってくること。

### 無限ループ/エラーのCGI
`test/www/cgi-bin/timeout.py` を使用。
```bash
curl -v http://localhost:8082/cgi-bin/timeout.py
```
*   **期待結果**: サーバーがクラッシュせず、タイムアウト後に **`504 Gateway Timeout`** が返ること。

---

## 7. ブラウザでの確認 (Check with a browser)

### ヘッダーと静的サイトの確認
1. ブラウザで `http://localhost:8080/` を開く。
2. 開発者ツールの「ネットワーク」タブを確認。
*   **確認事項**: リクエスト/レスポンスヘッダーが正しく表示され、CSSや画像（もしあれば）を含めサイトが正しく表示されること。

### ディレクトリリスティング (Autoindex)
```bash
# configurations/example.conf では / に autoindex on; が設定されている
curl -v http://localhost:8080/files
```
*   **期待結果**: ファイル一覧のHTMLが返ってくること。

### リダイレクト
```bash
curl -v -H "Host: www.example.com" http://localhost:8080/old-page
```
*   **期待結果**: `301 Moved Permanently` が返り、`Location: /new-page.html` が含まれていること。

---

## 8. ポートの問題 (Port issues)

### 同一ポートでの複数ウェブサイト (Virtual Host)
```bash
# Hostヘッダーを変えて同じポート(8080)にアクセス
curl -i -H "Host: localhost" http://localhost:8080/
curl -i -H "Host: example.com" http://localhost:8080/
curl -i -H "Host: www.example.com" http://localhost:8080/
```
*   **期待結果**: 各 Host ごとに設定された異なる `root` の内容が返ってくること。

### 同一ポートへの複数プログラムのバインド
1. `webserv` を起動した状態で、別のターミナルから同じ設定で `webserv` を起動しようとする。
```bash
docker-compose exec webserv ./webserv configurations/example.conf
```
*   **期待結果**: `bind` エラーが発生し、2つ目のプログラムが正常に起動しないこと（または一貫したエラー処理が行われること）。

---

## 9. Siege によるストレステスト

```bash
# 10クライアント、10秒間のテスト
siege -c 10 -t 10S http://localhost:8080/
```
*   **期待結果**: Availability が 99.5% 以上であること。
