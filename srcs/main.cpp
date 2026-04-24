#include "../includes/config/ServerContext.hpp"
#include "../includes/config/Config.hpp"

/**
 * @brief HTTPリクエストをシミュレートし、どの設定が適用されるかを出力する
 */
void simulateRequest(const Config& config, int port, const std::string& host, const std::string& uri) {
	std::cout << "\n[Request] Port: " << port << " | Host: " << host << " | URI: " << uri << std::endl;
	// 1. サーバーの特定 (Virtual Hosting対応)
	const ServerContext* server = config.getServer(port, host);
	if (server == NULL) {
		std::cout << "  -> Result: 404 Not Found (No Server on this Port)" << std::endl;
		return;
	}
	// server_nameのセットから代表として最初の名前を表示
	std::string primary_name = "(None)";
	if (!server->getServerNames().empty()) {
		primary_name = *server->getServerNames().begin();
	}
	std::cout << "  -> Matched Server : " << primary_name << " (Port: " << server->getPort() << ")" << std::endl;
	// 2. ロケーションの特定 (Longest Prefix Match)
	const LocationContext* location = config.matchLocation(server, uri);
	if (location == NULL) {
		std::cout << "  -> Matched Location: (None) -> 404 Not Found" << std::endl;
		return;
	}
	std::cout << "  -> Matched Location: " << location->getPath() << std::endl;
	// 3. 継承された設定値の確認 (Step 2の検証用)
	std::cout << "  -> Effective Config:" << std::endl;
	std::cout << "       |- Max Body Size: " << location->getClientMaxBodySize() << std::endl;
	if (!location->getErrorPages().empty()) {
		std::cout << "       |- Error Page 404: " << location->getErrorPages().find(404)->second << std::endl;
	}
	// 4. パス変換のシミュレーション
	if (!location->getAlias().empty()) {
		// Alias: マッチしたパス部分を物理パスに置き換える
		std::string remaining = uri.substr(location->getPath().length());
		std::cout << "  -> Target File    : " << location->getAlias() << remaining << " (Alias applied)" << std::endl;
	} else if (!location->getRoot().empty()) {
		// Root: ルートディレクトリの末尾にURIを連結する
		std::cout << "  -> Target File    : " << location->getRoot() << uri << " (Root applied)" << std::endl;
	} else {
		std::cout << "  -> Target File    : (No Root/Alias set)" << std::endl;
	}
}

int main(int argc, char **argv) {
	std::string config_file = (argc >= 2) ? argv[1] : "configurations/example.conf";

	Config config;
	std::cout << "Loading " << config_file << "..." << std::endl;
	if (!config.loadFile(config_file)) {
		std::cerr << "Failed to load config." << std::endl;
		return 1;
	}

	std::cout << "========================================" << std::endl;
	std::cout << "      Routing Engine Simulation         " << std::endl;
	std::cout << "========================================" << std::endl;

	// テストパターン: 複数ドメイン
	simulateRequest(config, 8080, "example.com", "/");
	simulateRequest(config, 8080, "www.example.com", "/");
	// テストパターン: デフォルトサーバーへのフォールバック
	simulateRequest(config, 8080, "unknown.host", "/");
	// テストパターン: Longest Prefix Match
	simulateRequest(config, 8080, "localhost", "/upload/images/test.png");
	// テストパターン: Aliasによるパス置換
	simulateRequest(config, 8080, "localhost", "/kapouet/pouic/toto.html");
	// テストパターン: ディレクトリ境界のチェック (/uploads は / にマッチすべき)
	simulateRequest(config, 8080, "localhost", "/uploads/dummy.txt");

	return 0;
}
