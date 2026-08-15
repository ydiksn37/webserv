*このプロジェクトは、42のカリキュラムの一環としてyukusano、sonakamu、ssawaによって作成されました。*

# Webserv

[English Version](README.md)

## Description

Webservは、C++98で記述された小規模なHTTP/1.1サーバーです。ネットワークプログラミング、リクエストの解析、ルーティング、およびレスポンス生成というコアコンセプトに焦点を当てつつ、本番環境のWebサーバーに期待される動作のサブセットを実装しています。

このサーバーでは以下のことが可能です：

- 設定されたドキュメントルートから静的ファイルを提供する。
- 複数のポートをリッスンする。
- ポートと`Host`ヘッダーによってサーバーブロックを選択する。
- リクエストを最も長い互換性のある`location`プレフィックスにマッチングさせる。
- `GET`、`POST`、および`DELETE`メソッドを処理する。
- デフォルトおよびカスタムのエラーページを生成する。
- locationごとに許可されたメソッドを強制する。
- 設定可能なクライアントのボディサイズ制限を強制する。
- チャンク化されたリクエストボディをデコードする。
- `autoindex`が有効な場合、ディレクトリのリスト表示（ディレクトリリスティング）を生成する。
- アップロードされたリクエストボディを設定されたアップロードディレクトリに保存する。
- ファイル拡張子に基づいてCGIスクリプトを実行する。

実装は以下の個別のモジュールに分かれています：

- `srcs/config`: 設定のトークナイズ、構文解析、バリデーション、サーバー/locationのルックアップ。
- `srcs/Http`: HTTPリクエストの解析とレスポンスのシリアライズ。
- `srcs/engine`: ルート解決、メソッドのディスパッチ、静的ファイル、アップロード、削除、CGIの選択。
- `srcs/eventloop`: ソケット、epollイベントループ、クライアントの状態、CGIパイプの統合。

## Instructions

リポジトリのルートからサーバーをビルドします：

```sh
make
```

明示的な設定ファイルを指定して実行します：

```sh
./webserv configurations/test.conf
```

引数なしで実行し、デフォルトの設定を使用します：

```sh
./webserv
```

デフォルトのパスは以下の通りです：

```text
configurations/default.conf
```

利用可能な設定ファイルの例：

- `configurations/default.conf`: ポート`8080`上の最小限の静的サーバー。
- `configurations/test.conf`: 静的ファイル、CGI、アップロード、削除、チャンクリクエストボディのテスト用。
- `configurations/cgi.conf`: ポート`8082`上のCGIに特化したサーバー。
- `configurations/example.conf`: 複数のサーバーブロック、複数のポート、エイリアス、リダイレクト、アップロード、CGI。
- `configurations/listen_ip.conf`: 複数のリッスンIP、適切なIPアドレスの選択、ソケットの作成。

リクエストの例：

```sh
curl -v http://localhost:8080/
curl -v http://localhost:8080/files/existing.txt
curl -v -X POST --data-binary @test/upload/webserv_upload_payload.txt \
  http://localhost:8080/upload/webserv_upload_payload.txt
curl -v -X DELETE http://localhost:8080/upload/webserv_upload_payload.txt
curl -v "http://localhost:8080/cgi-bin/hello.py?name=webserv"
```

チャンクアップロードの例：

```sh
printf 'POST /upload/chunked.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nhello world\r\n0\r\n\r\n' | nc localhost 8080
```

`Ctrl-C`でサーバーを停止します。

## Configuration

設定ファイルはnginxにインスパイアされた構文を使用し、`server`および`location`ブロックを持ちます。サポートされているディレクティブは以下の通りです：

- `listen`
- `server_name`
- `root`
- `index`
- `client_max_body_size`
- `error_page`
- `allow_methods` または `allow_method`
- `autoindex`
- `return`
- `alias`
- `upload_enable`
- `upload_store`
- `cgi_extension`
- `cgi_path`

Locationsは、location内で上書きされない限り、`root`、`index`、`client_max_body_size`、`error_page`などの選択されたサーバーレベルの設定を継承します。

## Notes

このプロジェクトは42のLinux評価環境で実行されることを想定しています。イベントループにはLinuxの`epoll`を使用しています。

CGIの例は、一般的なインタープリタのパス用に設定されています：

- Python: `/usr/bin/python3`
- PHP CGI: `/usr/bin/php-cgi`

インタープリタが設定されたパスにインストールされていない場合、設定が調整されるまで対応するCGIリクエストは失敗します。

## Resources

プロジェクトの設計および検証中に使用した参考資料：
- [JUN's blog](https://jun-networks.hatenablog.com/entry/2022/12/05/234522)
- [RFC 9110, HTTP Semantics.](https://tex2e.github.io/rfc-translater/html/rfc9110.html)
- [RFC 9112, HTTP/1.1.](https://tex2e.github.io/rfc-translater/html/rfc9112.html)
- [RFC 6265, HTTP State Management Mechanism.](https://datatracker.ietf.org/doc/html/rfc6265)
- [What is HttpRequest HttpResponse?](Https://qiita.com/minateru/items/8693538bbd0768855266)
- [List of HTTP Status Codes](https://qiita.com/takuo_maeda/items/9cff0b03e74f8f600eee)
- `socket`, `bind`, `listen`, `accept`, `epoll`, `fcntl`, `read`, `write`, `pipe`, `fork`, `dup2`, および `execve` のLinuxマニュアルページ。
- CGI/1.1環境変数の規則。
- nginxドキュメントおよび設定構造、locationマッチング、静的ファイルの提供、リダイレクト、エラーページに関する観察された動作。

AIアシスタンスはレビュー指向のタスクに使用されました：要件に対するプロジェクトの確認、リスク領域の特定、テスト戦略の議論、ドキュメントの起草。実装と最終的な動作はチームによってレビューおよび検証されました。
