#include "test_framework.hpp"
#include "engine.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

static Config loadEngineConfig() {
    Config cfg;
    cfg.loadFile("conf/engine.conf");
    return cfg;
}

static EngineResult runEngine(Config& cfg, const std::string& raw_request) {
    HttpRequest req;
    req.parse(raw_request);
    return engine(cfg, req, ListenEndpoint("0.0.0.0", 8181));
}

static std::string readFile(const std::string& path) {
    std::ifstream ifs(path.c_str(), std::ios::binary);
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

static bool fileExists(const std::string& path) {
    std::ifstream ifs(path.c_str(), std::ios::binary);
    return ifs.is_open();
}

static void test_GetIndexFile() {
    beginTest("GetIndexFile");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_FALSE(result.is_cgi);
    EXPECT_TRUE(res.find("HTTP/1.1 200 OK\r\n") == 0);
    EXPECT_TRUE(res.find("Content-Type: text/html") != std::string::npos);
    EXPECT_TRUE(res.find("\r\n\r\nEngine Index\n") != std::string::npos);
    passTest();
}

static void test_GetMissingUsesCustom404() {
    beginTest("GetMissingUsesCustom404");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "GET /missing HTTP/1.1\r\nHost: localhost\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_TRUE(res.find("HTTP/1.1 404 Not Found\r\n") == 0);
    EXPECT_TRUE(res.find("\r\n\r\nCustom 404\n") != std::string::npos);
    passTest();
}

static void test_MethodNotAllowedHasAllowHeader() {
    beginTest("MethodNotAllowedHasAllowHeader");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_TRUE(res.find("HTTP/1.1 405 Method Not Allowed\r\n") == 0);
    EXPECT_TRUE(res.find("Allow: GET\r\n") != std::string::npos);
    passTest();
}

static void test_NotImplementedMethodReturns501() {
    beginTest("NotImplementedMethodReturns501");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "HEAD / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_TRUE(res.find("HTTP/1.1 501 Not Implemented\r\n") == 0);
    passTest();
}

static void test_RedirectLocation() {
    beginTest("RedirectLocation");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "GET /old HTTP/1.1\r\nHost: localhost\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_TRUE(res.find("HTTP/1.1 302 Found\r\n") == 0);
    EXPECT_TRUE(res.find("Location: /new\r\n") != std::string::npos);
    passTest();
}

static void test_AutoindexDirectoryListing() {
    beginTest("AutoindexDirectoryListing");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "GET /auto/ HTTP/1.1\r\nHost: localhost\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_TRUE(res.find("HTTP/1.1 200 OK\r\n") == 0);
    EXPECT_TRUE(res.find("Index of /auto/") != std::string::npos);
    EXPECT_TRUE(res.find("list.txt") != std::string::npos);
    passTest();
}

static void test_UploadPostCreatesFile() {
    beginTest("UploadPostCreatesFile");
    std::remove("tmp_upload/uploaded.txt");

    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "POST /upload/uploaded.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 12\r\n\r\nhello upload");
    std::string res = result.response.serialize();
    EXPECT_TRUE(res.find("HTTP/1.1 201 Created\r\n") == 0);
    EXPECT_TRUE(fileExists("tmp_upload/uploaded.txt"));
    EXPECT_STREQ(readFile("tmp_upload/uploaded.txt"), "hello upload");

    std::remove("tmp_upload/uploaded.txt");
    passTest();
}

static void test_DeleteRemovesFile() {
    beginTest("DeleteRemovesFile");
    {
        std::ofstream ofs("www/files/delete_me.txt", std::ios::binary);
        ofs << "delete me";
    }
    EXPECT_TRUE(fileExists("www/files/delete_me.txt"));

    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "DELETE /files/delete_me.txt HTTP/1.1\r\nHost: localhost\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_TRUE(res.find("HTTP/1.1 204 No Content\r\n") == 0);
    EXPECT_FALSE(fileExists("www/files/delete_me.txt"));
    passTest();
}

static void test_CgiRouteDetectedByExtension() {
    beginTest("CgiRouteDetectedByExtension");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "GET /cgi-bin/hello.py?name=webserv HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(result.is_cgi);
    EXPECT_STREQ(result.bin_path, "/usr/bin/python3");
    EXPECT_STREQ(result.script_path, "www/cgi-bin/hello.py");
    passTest();
}

static void test_CgiMissingScriptReturns404() {
    beginTest("CgiMissingScriptReturns404");
    Config cfg = loadEngineConfig();
    EngineResult result = runEngine(cfg, "GET /cgi-bin/missing.py HTTP/1.1\r\nHost: localhost\r\n\r\n");
    std::string res = result.response.serialize();
    EXPECT_FALSE(result.is_cgi);
    EXPECT_TRUE(res.find("HTTP/1.1 404 Not Found\r\n") == 0);
    EXPECT_TRUE(res.find("\r\n\r\nCustom 404\n") != std::string::npos);
    passTest();
}

int main() {
    std::cout << YELLOW << "\n=== Engine Unit Tests ===" << RESET << std::endl;

    RUN_TEST(test_GetIndexFile);
    RUN_TEST(test_GetMissingUsesCustom404);
    RUN_TEST(test_MethodNotAllowedHasAllowHeader);
    RUN_TEST(test_NotImplementedMethodReturns501);
    RUN_TEST(test_RedirectLocation);
    RUN_TEST(test_AutoindexDirectoryListing);
    RUN_TEST(test_UploadPostCreatesFile);
    RUN_TEST(test_DeleteRemovesFile);
    RUN_TEST(test_CgiRouteDetectedByExtension);
    RUN_TEST(test_CgiMissingScriptReturns404);

    return testSummary();
}
