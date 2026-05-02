#include "test_framework.hpp"
#include "HttpRequest.hpp"

#include <fstream>
#include <sstream>
#include <string>

// ------------------------------------------------------------------ helpers --

static std::string loadFile(const std::string& name) {
    std::ifstream ifs(("req/" + name).c_str(), std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << RED << "  [ERROR] cannot open: unit_test/req/" << name << RESET << std::endl;
        return "";
    }
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

// ---------------------------------------------------------------- OK tests --

static void test_OKCorrect() {
    beginTest("OKCorrect");
    HttpRequest req;
    req.parse(loadFile("OKCorrect.txt"));
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getMethod(), "GET");
    EXPECT_STREQ(req.getPath(), "/");
    passTest();
}

static void test_OKBodyCorrectChunk() {
    beginTest("OKBodyCorrectChunk");
    HttpRequest req;
    req.parse(loadFile("OKBodyCorrectChunk.txt"));
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getBody(), "12345abcde");
    passTest();
}

static void test_OKBodyCorrectChunkHexadecimal() {
    beginTest("OKBodyCorrectChunkHexadecimal");
    HttpRequest req;
    req.parse(loadFile("OKBodyCorrectChunkHexadecimal.txt"));
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    passTest();
}

static void test_OKWithQueryString() {
    beginTest("OKWithQueryString");
    HttpRequest req;
    req.parse("GET /search?q=hello&lang=en HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getPath(), "/search");
    EXPECT_STREQ(req.getQuery(), "q=hello&lang=en");
    passTest();
}

static void test_OKPathNormalization() {
    beginTest("OKPathNormalization");
    HttpRequest req;
    req.parse("GET /a/b/../c/./d HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getPath(), "/a/c/d");
    passTest();
}

static void test_OKHeaderCaseInsensitive() {
    beginTest("OKHeaderCaseInsensitive");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHOST: localhost\r\nContent-Type: text/plain\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getHeader("content-type"), "text/plain");
    passTest();
}

static void test_OKIncrementalParse() {
    beginTest("OKIncrementalParse");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\n");
    EXPECT_FALSE(req.isCompleted());
    EXPECT_FALSE(req.isError());
    req.parse("Host: localhost\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    passTest();
}

static void test_OKPostWithContentLength() {
    beginTest("OKPostWithContentLength");
    HttpRequest req;
    req.parse("POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getBody(), "hello");
    passTest();
}

static void test_OKCorrectNewLine() {
    beginTest("OKCorrectNewLine");
    HttpRequest req;
    req.parse("\r\n\r\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getPath(), "/");
    passTest();
}

static void test_OKHeaderOWSTrimmed() {
    beginTest("OKHeaderOWSTrimmed");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost:\t \t localhost:49200 \t \t\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getHeader("host"), "localhost:49200");
    passTest();
}

static void test_OKHeaderListMultipleLine() {
    beginTest("OKHeaderListMultipleLine");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost: localhost\r\nHoge: hoge\r\nHoge: fuga\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getHeader("hoge"), "hoge, fuga");
    passTest();
}

static void test_OKCommaInDquoteHeader() {
    beginTest("OKCommaInDquoteHeader");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost: localhost\r\nHoge: \"a, b,,c\"\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getHeader("hoge"), "\"a, b,,c\"");
    passTest();
}

static void test_OKChunkWithExtension() {
    beginTest("OKChunkWithExtension");
    HttpRequest req;
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5;name=value\r\nhello\r\n0\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getBody(), "hello");
    passTest();
}

static void test_OKTransferEncodingCaseInsensitive() {
    beginTest("OKTransferEncodingCaseInsensitive");
    HttpRequest req;
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: ChUnKeD\r\n\r\n5\r\nhello\r\n0\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getBody(), "hello");
    passTest();
}

static void test_OKHttp10WithoutHost() {
    beginTest("OKHttp10WithoutHost");
    HttpRequest req;
    req.parse("GET /legacy HTTP/1.0\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getVersion(), "HTTP/1.0");
    passTest();
}

// --------------------------------------------------------------- KO tests --

static void test_KOFormatNotExistHostHeader() {
    beginTest("KOFormatNotExistHostHeader");
    HttpRequest req;
    req.parse(loadFile("KOFormatNotExistHostHeader.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatNotExistMethod() {
    beginTest("KOFormatNotExistMethod");
    HttpRequest req;
    req.parse(loadFile("KOFormatNotExistMethod.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOURLTooLong() {
    beginTest("KOURLTooLong");
    HttpRequest req;
    req.parse(loadFile("KOURLTooLong.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 414);
    passTest();
}

static void test_KOVersionHTTP2() {
    beginTest("KOVersionHTTP2");
    HttpRequest req;
    req.parse(loadFile("KOVersionInvalidMajorUpper.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 505);
    passTest();
}

static void test_KOVersionInvalidPrefix() {
    beginTest("KOVersionInvalidPrefix");
    HttpRequest req;
    // HPPP/1.1 is not a recognized HTTP version → 505
    req.parse("GET / HPPP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 505);
    passTest();
}

static void test_KOContentLengthTooLong() {
    beginTest("KOContentLengthTooLong");
    HttpRequest req;
    req.parse(loadFile("KOContentLengthTooLong.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 413);
    passTest();
}

static void test_KOBodyChunkSizeTooLarge() {
    beginTest("KOBodyChunkSizeTooLarge");
    HttpRequest req;
    req.parse(loadFile("KOBodyChunkSizeTooLarge.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 413);
    passTest();
}

static void test_KOBodyInvalidChunkData() {
    beginTest("KOBodyInvalidChunkData");
    HttpRequest req;
    req.parse(loadFile("KOBodyInvalidChunkSizeLower.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFieldInvalidTransferEncoding() {
    beginTest("KOFieldInvalidTransferEncoding");
    HttpRequest req;
    req.parse(loadFile("KOFieldInvalidTransferEncoding.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 501);
    passTest();
}

static void test_KOFormatExistOBSfold() {
    beginTest("KOFormatExistOBSfold");
    HttpRequest req;
    req.parse(loadFile("KOFormatExistOBSfold.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOHeaderSPBeforeColon() {
    beginTest("KOHeaderSPBeforeColon");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost : localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOLowercaseMethod() {
    beginTest("KOLowercaseMethod");
    // RFC 7230: method tokens are case-sensitive and must be uppercase
    HttpRequest req;
    req.parse("get / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOMethodNotImplemented() {
    beginTest("KOMethodNotImplemented");
    HttpRequest req;
    req.parse("HEAD / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 501);
    passTest();
}

static void test_KOUnknownMethod() {
    beginTest("KOUnknownMethod");
    HttpRequest req;
    req.parse("HOGE / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 501);
    passTest();
}

static void test_KOHeaderBufferTooLarge() {
    beginTest("KOHeaderBufferTooLarge");
    HttpRequest req;
    // Header buffer > 16384 triggers 431
    std::string large_data = "GET / HTTP/1.1\r\nHost: localhost\r\nX-Big: ";
    large_data += std::string(16384, 'A');
    large_data += "\r\n\r\n";
    req.parse(large_data);
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 431);
    passTest();
}

static void test_KOURLTooLongInline() {
    beginTest("KOURLTooLongInline");
    HttpRequest req;
    std::string long_url = "/" + std::string(2049, 'a');
    req.parse("GET " + long_url + " HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 414);
    passTest();
}

static void test_KOChunkSizeNotHex() {
    beginTest("KOChunkSizeNotHex");
    HttpRequest req;
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nZZZZ\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatNotExistURL() {
    beginTest("KOFormatNotExistURL");
    HttpRequest req;
    req.parse(loadFile("KOFormatNotExistURL.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatNotExistVersion() {
    beginTest("KOFormatNotExistVersion");
    HttpRequest req;
    req.parse(loadFile("KOFormatNotExistVersion.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOBodyInvalidChunkSizeUpper() {
    beginTest("KOBodyInvalidChunkSizeUpper");
    // Declared chunk size smaller than actual data
    HttpRequest req;
    req.parse(loadFile("KOBodyInvalidChunkSizeUpper.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOBodyNotExistChunkSize() {
    beginTest("KOBodyNotExistChunkSize");
    HttpRequest req;
    req.parse(loadFile("KOBodyNotExistChunkSize.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOVersionMultipleDot() {
    beginTest("KOVersionMultipleDot");
    // HTTP/1.1.1 is not HTTP/1.1 or HTTP/1.0 → 505
    HttpRequest req;
    req.parse(loadFile("KOVersioExistMultipleDot.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 505);
    passTest();
}

static void test_KOVersionInvalidMinorLong() {
    beginTest("KOVersionInvalidMinorLong");
    HttpRequest req;
    req.parse(loadFile("KOVersioInvalidMinorLong.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 505);
    passTest();
}

static void test_KOVersionInvalidMajorLong() {
    beginTest("KOVersionInvalidMajorLong");
    HttpRequest req;
    req.parse(loadFile("KOVersionInvalidMajorLong.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 505);
    passTest();
}

static void test_KOVersionHTTP09() {
    beginTest("KOVersionHTTP09");
    HttpRequest req;
    req.parse(loadFile("KOVersionInvalidMajorLower.txt"));
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 505);
    passTest();
}

static void test_OKChunkSizeZero() {
    beginTest("OKChunkSizeZero");
    HttpRequest req;
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n");
    EXPECT_FALSE(req.isError());
    EXPECT_TRUE(req.isCompleted());
    EXPECT_STREQ(req.getBody(), "");
    passTest();
}

static void test_KOFormatExistSPBeforeRequestLine() {
    beginTest("KOFormatExistSPBeforeRequestLine");
    HttpRequest req;
    req.parse(" GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatExistSPBetweenMethodAndURL() {
    beginTest("KOFormatExistSPBetweenMethodAndURL");
    HttpRequest req;
    req.parse("GET  / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatExistSPBetweenURLAndVersion() {
    beginTest("KOFormatExistSPBetweenURLAndVersion");
    HttpRequest req;
    req.parse("GET /  HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatExistSPAfterVersion() {
    beginTest("KOFormatExistSPAfterVersion");
    HttpRequest req;
    req.parse("GET / HTTP/1.1 \r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatNotExistRequestLine() {
    beginTest("KOFormatNotExistRequestLine");
    HttpRequest req;
    req.parse("Host: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOFormatNotExistCRLFBetweenRequestLineAndHeader() {
    beginTest("KOFormatNotExistCRLFBetweenRequestLineAndHeader");
    HttpRequest req;
    req.parse("GET / HTTP/1.1Host: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOHeaderTabBeforeColon() {
    beginTest("KOHeaderTabBeforeColon");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost\t: localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOHeaderUnbalancedDquote() {
    beginTest("KOHeaderUnbalancedDquote");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost: \"localhost\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOMultipleHostHeaders() {
    beginTest("KOMultipleHostHeaders");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost: localhost\r\nHost: example.com\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOHostHeaderListValue() {
    beginTest("KOHostHeaderListValue");
    HttpRequest req;
    req.parse("GET / HTTP/1.1\r\nHost: localhost, example.com\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOContentLengthTransferEncodingConflict() {
    beginTest("KOContentLengthTransferEncodingConflict");
    HttpRequest req;
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOContentLengthDuplicateMismatch() {
    beginTest("KOContentLengthDuplicateMismatch");
    HttpRequest req;
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 6\r\n\r\nhello");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOInvalidContentLength() {
    beginTest("KOInvalidContentLength");
    HttpRequest req;
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: -1\r\n\r\n");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 400);
    passTest();
}

static void test_KOBodyExceedsCustomMaxSize() {
    beginTest("KOBodyExceedsCustomMaxSize");
    HttpRequest req;
    req.setMaxBodySize(4);
    req.parse("POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello");
    EXPECT_TRUE(req.isError());
    EXPECT_EQ(req.getErrorCode(), 413);
    passTest();
}

// ------------------------------------------------------------------ main --

int main() {
    std::cout << YELLOW << "\n=== HttpRequest Unit Tests ===" << RESET << std::endl;

    std::cout << "\n[ OK tests ]" << std::endl;
    RUN_TEST(test_OKCorrect);
    RUN_TEST(test_OKBodyCorrectChunk);
    RUN_TEST(test_OKBodyCorrectChunkHexadecimal);
    RUN_TEST(test_OKWithQueryString);
    RUN_TEST(test_OKPathNormalization);
    RUN_TEST(test_OKHeaderCaseInsensitive);
    RUN_TEST(test_OKIncrementalParse);
    RUN_TEST(test_OKPostWithContentLength);
    RUN_TEST(test_OKChunkSizeZero);
    RUN_TEST(test_OKCorrectNewLine);
    RUN_TEST(test_OKHeaderOWSTrimmed);
    RUN_TEST(test_OKHeaderListMultipleLine);
    RUN_TEST(test_OKCommaInDquoteHeader);
    RUN_TEST(test_OKChunkWithExtension);
    RUN_TEST(test_OKTransferEncodingCaseInsensitive);
    RUN_TEST(test_OKHttp10WithoutHost);

    std::cout << "\n[ KO tests ]" << std::endl;
    RUN_TEST(test_KOFormatNotExistHostHeader);
    RUN_TEST(test_KOFormatNotExistMethod);
    RUN_TEST(test_KOURLTooLong);
    RUN_TEST(test_KOVersionHTTP2);
    RUN_TEST(test_KOVersionInvalidPrefix);
    RUN_TEST(test_KOContentLengthTooLong);
    RUN_TEST(test_KOBodyChunkSizeTooLarge);
    RUN_TEST(test_KOBodyInvalidChunkData);
    RUN_TEST(test_KOFieldInvalidTransferEncoding);
    RUN_TEST(test_KOFormatExistOBSfold);
    RUN_TEST(test_KOHeaderSPBeforeColon);
    RUN_TEST(test_KOLowercaseMethod);
    RUN_TEST(test_KOMethodNotImplemented);
    RUN_TEST(test_KOUnknownMethod);
    RUN_TEST(test_KOHeaderBufferTooLarge);
    RUN_TEST(test_KOURLTooLongInline);
    RUN_TEST(test_KOChunkSizeNotHex);
    RUN_TEST(test_KOFormatNotExistURL);
    RUN_TEST(test_KOFormatNotExistVersion);
    RUN_TEST(test_KOBodyInvalidChunkSizeUpper);
    RUN_TEST(test_KOBodyNotExistChunkSize);
    RUN_TEST(test_KOVersionMultipleDot);
    RUN_TEST(test_KOVersionInvalidMinorLong);
    RUN_TEST(test_KOVersionInvalidMajorLong);
    RUN_TEST(test_KOVersionHTTP09);
    RUN_TEST(test_KOFormatExistSPBeforeRequestLine);
    RUN_TEST(test_KOFormatExistSPBetweenMethodAndURL);
    RUN_TEST(test_KOFormatExistSPBetweenURLAndVersion);
    RUN_TEST(test_KOFormatExistSPAfterVersion);
    RUN_TEST(test_KOFormatNotExistRequestLine);
    RUN_TEST(test_KOFormatNotExistCRLFBetweenRequestLineAndHeader);
    RUN_TEST(test_KOHeaderTabBeforeColon);
    RUN_TEST(test_KOHeaderUnbalancedDquote);
    RUN_TEST(test_KOMultipleHostHeaders);
    RUN_TEST(test_KOHostHeaderListValue);
    RUN_TEST(test_KOContentLengthTransferEncodingConflict);
    RUN_TEST(test_KOContentLengthDuplicateMismatch);
    RUN_TEST(test_KOInvalidContentLength);
    RUN_TEST(test_KOBodyExceedsCustomMaxSize);

    return testSummary();
}
