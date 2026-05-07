#ifndef TEST_FRAMEWORK_HPP
# define TEST_FRAMEWORK_HPP

# include <iostream>
# include <string>
# include <sstream>

static int g_test_total = 0;
static int g_test_failed = 0;
static std::string g_current_test;

# define RESET  "\033[0m"
# define GREEN  "\033[32m"
# define RED    "\033[31m"
# define YELLOW "\033[33m"

inline void beginTest(const std::string& name) {
    g_current_test = name;
    g_test_total++;
}

inline void passTest() {
    std::cout << GREEN << "  [PASS] " << RESET << g_current_test << std::endl;
}

inline void failTest(const std::string& msg) {
    std::cout << RED << "  [FAIL] " << RESET << g_current_test << ": " << msg << std::endl;
    g_test_failed++;
}

# define RUN_TEST(name) do { name(); } while (0)

# define EXPECT_TRUE(cond) do { \
    if (!(cond)) { \
        std::ostringstream _oss; \
        _oss << "expected true: " << #cond; \
        failTest(_oss.str()); return; \
    } \
} while (0)

# define EXPECT_FALSE(cond) do { \
    if ((cond)) { \
        std::ostringstream _oss; \
        _oss << "expected false: " << #cond; \
        failTest(_oss.str()); return; \
    } \
} while (0)

# define EXPECT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::ostringstream _oss; \
        _oss << #a << " == " << (a) << ", expected " << (b); \
        failTest(_oss.str()); return; \
    } \
} while (0)

# define EXPECT_STREQ(a, b) do { \
    if (std::string(a) != std::string(b)) { \
        std::ostringstream _oss; \
        _oss << #a << " == \"" << (a) << "\", expected \"" << (b) << "\""; \
        failTest(_oss.str()); return; \
    } \
} while (0)

inline int testSummary() {
    int passed = g_test_total - g_test_failed;
    std::cout << "\n--- Test Summary ---" << std::endl;
    std::cout << GREEN << "  Passed: " << passed << RESET << std::endl;
    if (g_test_failed > 0)
        std::cout << RED << "  Failed: " << g_test_failed << RESET << std::endl;
    std::cout << "  Total:  " << g_test_total << std::endl;
    return (g_test_failed > 0 ? 1 : 0);
}

#endif
