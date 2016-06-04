#include "integer.hpp"

#include <iostream>
#include <cstdio>

using namespace mod;

class TestContext {
  bool _abort_on_failure = false;
  std::string _name;
  unsigned _fail_count = 0;
  unsigned _pass_count = 0;

public:
  TestContext(const char *name) : _name(name) {}

  void check_failed(const char *msg, unsigned line, const char *file,
                    const char *function) {
    std::printf("[%s] %s (line %d, function %s)\n", _name.c_str(), msg, line,
                function);
    if (_abort_on_failure)
      std::abort();
    _fail_count++;
  }

  void check_passed() { _pass_count++; }

  ~TestContext() {
    std::printf("[%s] %s: %d failed, %d passed\n", _name.c_str(),
                _fail_count ? "FAILED" : "SUCCESS", _fail_count, _pass_count);
  }
};

#define ASSERT_TRUE(v)                                                         \
  do {                                                                         \
    const char *msg = #v " failed (expected true, was false)";                 \
    if (!(v))                                                                  \
      __ctx.check_failed(msg, __LINE__, __FILE__, __func__);                   \
    else                                                                       \
      __ctx.check_passed();                                                    \
  } while (false)

#define ASSERT_FALSE(v)                                                        \
  do {                                                                         \
    const char *msg = "Negate(" #v ") failed (expected false, was true)";      \
    if (v)                                                                     \
      __ctx.check_failed(msg, __LINE__, __FILE__, __func__);                   \
    else                                                                       \
      __ctx.check_passed();                                                    \
  } while (false)

#define ASSERT_EQ(v0, v1)                                                      \
  do {                                                                         \
    const char *msg = "equality failed, expected " #v0 " == " #v1;             \
    if ((v0) != (v1))                                                          \
      __ctx.check_failed(msg, __LINE__, __FILE__, __func__);                   \
    else                                                                       \
      __ctx.check_passed();                                                    \
  } while (false)

#define TEST(name) TestContext __ctx(name)

typedef Integer<6, 64> Int64;

static void test_additions() {
  TEST("Add");

  {
    Int64 a("00u"), b("001"), c("001");

    ASSERT_EQ(a.add(b).add(c).write(), "1u");
    ASSERT_EQ(b.add(a).add(c).write(), "1u");
    ASSERT_EQ(a.add(b.add(c)).write(), "1u");
  }

  {
    Int64 bin_57("111001"), bin_133("10000101"), bin_190("10111110");
    ASSERT_TRUE(bin_57.admits(57));
    ASSERT_TRUE(bin_133.admits(133));
    ASSERT_TRUE(bin_190.admits(190));
    auto sum_57_133 = bin_57.add(bin_133);
    ASSERT_EQ(sum_57_133, bin_190);

    ASSERT_TRUE(sum_57_133.admits(190));

    ASSERT_FALSE(sum_57_133.admits(191));
    ASSERT_FALSE(sum_57_133.admits(2191));
    ASSERT_FALSE(sum_57_133.admits(382));
    ASSERT_FALSE(sum_57_133.admits(95));
  }

  {
    // Similar to above, but now we have unknown bits.

    Int64 bin_57("11u0u1"), bin_133("u000010u");
    ASSERT_TRUE(bin_57.admits(57));
    ASSERT_TRUE(bin_133.admits(133));
    auto sum_57_133 = bin_57.add(bin_133);

    ASSERT_TRUE(sum_57_133.admits(190));

    // sum_57_133 is more precise than uuuuuuuu, but textually it "looks like"
    // uuuuuuuu.  E.g. uuuuuuuu as written admits both 200 and 95, but
    // sum_57_133 does not.
    ASSERT_EQ(sum_57_133.write(), "uuuuuuuu");

    ASSERT_FALSE(sum_57_133.admits(2191));
    ASSERT_FALSE(sum_57_133.admits(200));
    ASSERT_FALSE(sum_57_133.admits(95));
  }
}

int main() {
  test_additions();
}
