#include "test_framework.hpp"
#include "Config.hpp"

#include <stdexcept>
#include <string>

// ---------------------------------------------------------------- loadFile --

static void test_LoadValidSimpleServer() {
    beginTest("LoadValidSimpleServer");
    Config cfg;
    cfg.loadFile("../configurations/default.conf");
    EXPECT_EQ(cfg.getServers().size(), (size_t)1);
    EXPECT_EQ(cfg.getServers()[0].getPort(), 8080);
    passTest();
}

static void test_LoadValidExampleConf() {
    beginTest("LoadValidExampleConf");
    Config cfg;
    bool threw = false;
    try { cfg.loadFile("../configurations/example.conf"); }
    catch (const std::exception&) { threw = true; }
    EXPECT_FALSE(threw);
    passTest();
}

static void test_LoadNonExistentFile() {
    beginTest("LoadNonExistentFile");
    Config cfg;
    bool threw = false;
    try { cfg.loadFile("../configurations/does_not_exist.conf"); }
    catch (const Config::ConfigException&) { threw = true; }
    EXPECT_TRUE(threw);
    passTest();
}

static void test_LoadEmptyConf() {
    beginTest("LoadEmptyConf");
    // create a temp empty conf string and parse via tokenize
    Config cfg;
    bool threw = false;
    try { cfg.loadFile("conf/empty.conf"); }
    catch (const Config::ConfigException&) { threw = true; }
    EXPECT_TRUE(threw);
    passTest();
}

static void test_LoadInvalidDirective() {
    beginTest("LoadInvalidDirective");
    Config cfg;
    bool threw = false;
    try { cfg.loadFile("conf/invalid_directive.conf"); }
    catch (const Config::ConfigException&) { threw = true; }
    EXPECT_TRUE(threw);
    passTest();
}

static void test_LoadInvalidPort() {
    beginTest("LoadInvalidPort");
    Config cfg;
    bool threw = false;
    try { cfg.loadFile("conf/invalid_port.conf"); }
    catch (const Config::ConfigException&) { threw = true; }
    EXPECT_TRUE(threw);
    passTest();
}

static void test_LoadMissingClosingBrace() {
    beginTest("LoadMissingClosingBrace");
    Config cfg;
    bool threw = false;
    try { cfg.loadFile("conf/missing_brace.conf"); }
    catch (const Config::ConfigException&) { threw = true; }
    EXPECT_TRUE(threw);
    passTest();
}

static void test_LoadMultipleServers() {
    beginTest("LoadMultipleServers");
    Config cfg;
    cfg.loadFile("conf/multi_server.conf");
    EXPECT_EQ(cfg.getServers().size(), (size_t)2);
    EXPECT_EQ(cfg.getServers()[0].getPort(), 8080);
    EXPECT_EQ(cfg.getServers()[1].getPort(), 9090);
    passTest();
}

static void test_LoadServerWithAllDirectives() {
    beginTest("LoadServerWithAllDirectives");
    Config cfg;
    cfg.loadFile("conf/full_server.conf");
    const ServerContext& s = cfg.getServers()[0];
    EXPECT_EQ(s.getPort(), 7070);
    EXPECT_TRUE(s.getIsServerNameIncluded("mysite.com"));
    EXPECT_EQ(s.getClientMaxBodySize(), (size_t)(2 * 1024 * 1024));
    passTest();
}

static void test_LoadCgiConf() {
    beginTest("LoadCgiConf");
    Config cfg;
    bool threw = false;
    try { cfg.loadFile("../configurations/cgi.conf"); }
    catch (const std::exception& e) { threw = true; }
    EXPECT_FALSE(threw);
    passTest();
}

// ---------------------------------------------------------------- getServer --

static void test_GetServerByPort() {
    beginTest("GetServerByPort");
    Config cfg;
    cfg.loadFile("conf/multi_server.conf");
    const ServerContext* s = cfg.getServer(8080, "");
    EXPECT_TRUE(s != NULL);
    EXPECT_EQ(s->getPort(), 8080);
    passTest();
}

static void test_GetServerNotFound() {
    beginTest("GetServerNotFound");
    Config cfg;
    cfg.loadFile("conf/multi_server.conf");
    const ServerContext* s = cfg.getServer(9999, "");
    EXPECT_TRUE(s == NULL);
    passTest();
}

static void test_GetServerByServerName() {
    beginTest("GetServerByServerName");
    Config cfg;
    cfg.loadFile("conf/named_servers.conf");
    const ServerContext* s = cfg.getServer(8080, "site-b.com");
    EXPECT_TRUE(s != NULL);
    EXPECT_TRUE(s->getIsServerNameIncluded("site-b.com"));
    passTest();
}

static void test_GetServerDefaultWhenNoNameMatch() {
    beginTest("GetServerDefaultWhenNoNameMatch");
    // When no server_name matches, return the first server on that port
    Config cfg;
    cfg.loadFile("conf/named_servers.conf");
    const ServerContext* s = cfg.getServer(8080, "unknown.example.com");
    EXPECT_TRUE(s != NULL);
    EXPECT_TRUE(s->getIsServerNameIncluded("site-a.com"));
    passTest();
}

static void test_GetServerMultipleNamesOnSamePort() {
    beginTest("GetServerMultipleNamesOnSamePort");
    Config cfg;
    cfg.loadFile("conf/named_servers.conf");
    const ServerContext* sa = cfg.getServer(8080, "site-a.com");
    const ServerContext* sb = cfg.getServer(8080, "site-b.com");
    EXPECT_TRUE(sa != NULL);
    EXPECT_TRUE(sb != NULL);
    EXPECT_TRUE(sa != sb);
    passTest();
}

// -------------------------------------------------------------- matchLocation --

static void test_MatchLocationExact() {
    beginTest("MatchLocationExact");
    Config cfg;
    cfg.loadFile("conf/locations.conf");
    const ServerContext* s = cfg.getServer(8080, "");
    EXPECT_TRUE(s != NULL);
    const LocationContext* loc = cfg.matchLocation(s, "/api/users");
    EXPECT_TRUE(loc != NULL);
    EXPECT_STREQ(loc->getPath(), "/api");
    passTest();
}

static void test_MatchLocationLongestPrefix() {
    beginTest("MatchLocationLongestPrefix");
    Config cfg;
    cfg.loadFile("conf/locations.conf");
    const ServerContext* s = cfg.getServer(8080, "");
    const LocationContext* loc = cfg.matchLocation(s, "/api/v2/items");
    EXPECT_TRUE(loc != NULL);
    EXPECT_STREQ(loc->getPath(), "/api/v2");
    passTest();
}

static void test_MatchLocationRoot() {
    beginTest("MatchLocationRoot");
    Config cfg;
    cfg.loadFile("conf/locations.conf");
    const ServerContext* s = cfg.getServer(8080, "");
    const LocationContext* loc = cfg.matchLocation(s, "/other/path");
    EXPECT_TRUE(loc != NULL);
    EXPECT_STREQ(loc->getPath(), "/");
    passTest();
}

static void test_MatchLocationNoBoundaryFalsePositive() {
    beginTest("MatchLocationNoBoundaryFalsePositive");
    // /api should NOT match /apiary (boundary check)
    Config cfg;
    cfg.loadFile("conf/locations.conf");
    const ServerContext* s = cfg.getServer(8080, "");
    const LocationContext* loc = cfg.matchLocation(s, "/apiary");
    EXPECT_TRUE(loc != NULL);
    EXPECT_STREQ(loc->getPath(), "/");
    passTest();
}

static void test_MatchLocationNullServer() {
    beginTest("MatchLocationNullServer");
    Config cfg;
    const LocationContext* loc = cfg.matchLocation(NULL, "/");
    EXPECT_TRUE(loc == NULL);
    passTest();
}

// ------------------------------------------------------------ tokenize / misc --

static void test_TokenizeComment() {
    beginTest("TokenizeComment");
    Config cfg;
    // Config with comment - should be ignored
    cfg.loadFile("conf/with_comment.conf");
    EXPECT_EQ(cfg.getServers().size(), (size_t)1);
    EXPECT_EQ(cfg.getServers()[0].getPort(), 8080);
    passTest();
}

static void test_ClientMaxBodySizeWithSuffix() {
    beginTest("ClientMaxBodySizeWithSuffix");
    Config cfg;
    cfg.loadFile("conf/full_server.conf");
    const ServerContext& s = cfg.getServers()[0];
    EXPECT_EQ(s.getClientMaxBodySize(), (size_t)(2 * 1024 * 1024));
    passTest();
}

static void test_LocationInheritsServerRoot() {
    beginTest("LocationInheritsServerRoot");
    Config cfg;
    cfg.loadFile("conf/inherit.conf");
    const ServerContext* s = cfg.getServer(8080, "");
    EXPECT_TRUE(s != NULL);
    const LocationContext* loc = cfg.matchLocation(s, "/");
    EXPECT_TRUE(loc != NULL);
    EXPECT_STREQ(loc->getRoot(), "/var/www/html");
    passTest();
}

static void test_LocationOverridesServerRoot() {
    beginTest("LocationOverridesServerRoot");
    Config cfg;
    cfg.loadFile("conf/inherit.conf");
    const ServerContext* s = cfg.getServer(8080, "");
    const LocationContext* loc = cfg.matchLocation(s, "/static");
    EXPECT_TRUE(loc != NULL);
    EXPECT_STREQ(loc->getRoot(), "/var/www/static");
    passTest();
}

// ------------------------------------------------------------------- main --

int main() {
    std::cout << YELLOW << "\n=== Config Unit Tests ===" << RESET << std::endl;

    std::cout << "\n[ loadFile tests ]" << std::endl;
    RUN_TEST(test_LoadValidSimpleServer);
    RUN_TEST(test_LoadValidExampleConf);
    RUN_TEST(test_LoadNonExistentFile);
    RUN_TEST(test_LoadEmptyConf);
    RUN_TEST(test_LoadInvalidDirective);
    RUN_TEST(test_LoadInvalidPort);
    RUN_TEST(test_LoadMissingClosingBrace);
    RUN_TEST(test_LoadMultipleServers);
    RUN_TEST(test_LoadServerWithAllDirectives);
    RUN_TEST(test_LoadCgiConf);
    RUN_TEST(test_TokenizeComment);
    RUN_TEST(test_ClientMaxBodySizeWithSuffix);

    std::cout << "\n[ getServer tests ]" << std::endl;
    RUN_TEST(test_GetServerByPort);
    RUN_TEST(test_GetServerNotFound);
    RUN_TEST(test_GetServerByServerName);
    RUN_TEST(test_GetServerDefaultWhenNoNameMatch);
    RUN_TEST(test_GetServerMultipleNamesOnSamePort);

    std::cout << "\n[ matchLocation tests ]" << std::endl;
    RUN_TEST(test_MatchLocationExact);
    RUN_TEST(test_MatchLocationLongestPrefix);
    RUN_TEST(test_MatchLocationRoot);
    RUN_TEST(test_MatchLocationNoBoundaryFalsePositive);
    RUN_TEST(test_MatchLocationNullServer);
    RUN_TEST(test_LocationInheritsServerRoot);
    RUN_TEST(test_LocationOverridesServerRoot);

    return testSummary();
}
