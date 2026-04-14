// 1. SortIncludes: Never (順番が維持されるか)
#include <iostream>
#include "test1/test.hpp"
#include "test2/test.hpp"
#include "test3/test.hpp"

// 2. AlignConsecutiveMacros: true (バックスラッシュや値が揃うか)
#define SHORT_MACRO          10
#define VERY_LONG_MACRO_NAME 42
#define MULTI_LINE_MACRO(x)                                                                        \
  do {                                                                                             \
    foo(x);                                                                                        \
  } while (0)

// 3. AccessModifierOffset: -2 & IndentWidth: 2 (publicが0、中身が2になるか)
class FormatTest {
public:
  FormatTest() : _val(0), _name("test") {
  }  // 4. AllowShortFunctions: None (改行されるか)

  // 5. BreakConstructorInitializers: BeforeComma (カンマの前で改行されるか)
  FormatTest(int v, std::string n) : _val(v), _name(n) {
  }

  // 6. PointerAlignment: Left (int* p になるか)
  void test_pointers(int* p, double& r);

private:
  int _val;
  std::string _name;
};

// 7. BinPackParameters: false (引数が1行1つになるか)
void long_function_name(int param1, std::string param2, double param3, bool param4) {
  // 8. AllowShortIfStatements/Loops: Never (1行にまとめられないか)
  if (param4)
    return;

  // 9. AlignTrailingComments: true (コメントが縦に揃うか)
  int a = 1;    // comment A
  int bbb = 2;  // comment B

  // 10. MaxEmptyLinesToKeep: 1 (無駄な空行が消えるか)

  std::cout << "test" << std::endl;
}

// 11. BinPackArguments: false (呼び出し側も1行1引数になるか)
int main() {
  long_function_name(10, "hello world", 3.14, true);
  return 0;
}
