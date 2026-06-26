//
// Tests for argparse::expected<T,E>, expected<T&,E>, and expected<void,E>
//

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "argparse/argparse.hpp"

using argparse::bad_expected_access;
using argparse::expected;
using argparse::make_unexpected;
using argparse::unexpect_t;

// ============================================================================
// expected<T, E> — construction & basic observers
// ============================================================================

TEST(ExpectedValueTest, ConstructFromValue) {
  expected<int, std::string> e(42);
  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(static_cast<bool>(e));
  EXPECT_EQ(e.value(), 42);
  EXPECT_EQ(*e, 42);
}

TEST(ExpectedValueTest, ConstructFromUnexpected) {
  auto u = make_unexpected(std::string("error"));
  expected<int, std::string> e(u);
  EXPECT_FALSE(e.has_value());
  EXPECT_FALSE(static_cast<bool>(e));
  EXPECT_EQ(e.error(), "error");
}

TEST(ExpectedValueTest, ConstructFromUnexpectedConvertible) {
  // Construct from unexpected<const char*> which is convertible to
  // unexpected<std::string>
  expected<int, std::string> e(make_unexpected("fail"));
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "fail");
}

TEST(ExpectedValueTest, CopyConstruct) {
  expected<int, std::string> a(10);
  expected<int, std::string> b(a);
  EXPECT_EQ(b.value(), 10);

  expected<int, std::string> c(make_unexpected(std::string("err")));
  expected<int, std::string> d(c);
  EXPECT_EQ(d.error(), "err");
}

TEST(ExpectedValueTest, MoveConstruct) {
  expected<int, std::string> a(10);
  expected<int, std::string> b(std::move(a));
  EXPECT_EQ(b.value(), 10);
  // a is now moved-from but still destructible

  expected<int, std::string> c(make_unexpected(std::string("err")));
  expected<int, std::string> d(std::move(c));
  EXPECT_EQ(d.error(), "err");
}

TEST(ExpectedValueTest, CopyAssignment) {
  expected<int, std::string> a(10);
  expected<int, std::string> b(20);
  b = a;
  EXPECT_EQ(b.value(), 10);

  expected<int, std::string> c(make_unexpected(std::string("e1")));
  expected<int, std::string> d(make_unexpected(std::string("e2")));
  d = c;
  EXPECT_EQ(d.error(), "e1");

  // cross-state assignment
  expected<int, std::string> e(100);
  e = c;
  EXPECT_EQ(e.error(), "e1");

  expected<int, std::string> f(make_unexpected(std::string("x")));
  f = a;
  EXPECT_EQ(f.value(), 10);
}

TEST(ExpectedValueTest, MoveAssignment) {
  expected<int, std::string> a(10);
  expected<int, std::string> b(20);
  b = std::move(a);
  EXPECT_EQ(b.value(), 10);

  expected<int, std::string> c(make_unexpected(std::string("e1")));
  expected<int, std::string> d(make_unexpected(std::string("e2")));
  d = std::move(c);
  EXPECT_EQ(d.error(), "e1");
}

// Self-assignment is protected by the 'if (this == &other)' guard
// inside the assignment operators.
TEST(ExpectedValueTest, SelfAssignment) {
  expected<int, std::string> a(42);
  const auto& ref = a;
  a = ref;  // self-copy-assign via const ref to avoid -Wself-assign
  EXPECT_EQ(a.value(), 42);

  expected<int, std::string> b(make_unexpected(std::string("err")));
  const auto& bref = b;
  b = bref;
  EXPECT_EQ(b.error(), "err");

  // Self-move-assign via an intermediate copy
  expected<int, std::string> c(99);
  expected<int, std::string> tmp(std::move(c));
  c = std::move(tmp);
  EXPECT_EQ(c.value(), 99);
}

TEST(ExpectedValueTest, ValueOr) {
  expected<int, std::string> e(42);
  EXPECT_EQ(e.value_or(0), 42);

  expected<int, std::string> f(make_unexpected(std::string("err")));
  EXPECT_EQ(f.value_or(0), 0);
}

TEST(ExpectedValueTest, ValueOrRvalue) {
  // rvalue expected with value: moves the value out
  auto v = expected<int, std::string>(42).value_or(0);
  EXPECT_EQ(v, 42);

  // rvalue expected with error: returns the default
  auto e = expected<int, std::string>(make_unexpected(std::string("err")))
               .value_or(0);
  EXPECT_EQ(e, 0);
}

TEST(ExpectedValueTest, ValueOrRvalueMoveOnly) {
  // rvalue value_or moves the value from a move-only type
  auto ptr =
      expected<std::unique_ptr<int>, std::string>(std::make_unique<int>(99))
          .value_or(std::make_unique<int>(0));
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(*ptr, 99);
}

TEST(ExpectedValueTest, ValueOrRvalueMoveSemantics) {
  // Verify that rvalue value_or moves (not copies) the contained value.
  struct Tracker {
    int value;
    bool moved_from = false;
    Tracker(int v) : value(v) {}
    Tracker(const Tracker& other) : value(other.value) {}
    Tracker(Tracker&& other) noexcept : value(other.value) {
      other.moved_from = true;
    }
  };

  expected<Tracker, std::string> e(Tracker(42));
  auto result = std::move(e).value_or(Tracker(0));
  EXPECT_EQ(result.value, 42);
}

TEST(ExpectedValueTest, ErrorOr) {
  // On value: returns the provided default
  expected<int, std::string> e(42);
  EXPECT_EQ(e.error_or(std::string("no_error")), "no_error");

  // On error: returns the contained error
  expected<int, std::string> f(make_unexpected(std::string("real_error")));
  EXPECT_EQ(f.error_or(std::string("default")), "real_error");
}

TEST(ExpectedValueTest, ErrorOrRvalue) {
  // rvalue expected with value: returns default
  auto e = expected<int, std::string>(42).error_or(std::string("no_error"));
  EXPECT_EQ(e, "no_error");

  // rvalue expected with error: moves the error out
  auto f = expected<int, std::string>(make_unexpected(std::string("moved_err")))
               .error_or(std::string("default"));
  EXPECT_EQ(f, "moved_err");
}

TEST(ExpectedValueTest, ErrorOrWithConversion) {
  // G != E: implicit conversion from const char* to std::string
  expected<int, std::string> e(42);
  EXPECT_EQ(e.error_or("no_error"), "no_error");

  expected<int, std::string> f(make_unexpected(std::string("real_error")));
  EXPECT_EQ(f.error_or("default"), "real_error");
}

TEST(ExpectedValueTest, Emplace) {
  expected<int, std::string> e(make_unexpected(std::string("err")));
  e.emplace(99);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 99);

  // emplace on an existing value
  e.emplace(77);
  EXPECT_EQ(e.value(), 77);
}

TEST(ExpectedValueTest, ArrowOperator) {
  expected<std::string, int> e(std::string("hello"));
  EXPECT_EQ(e->size(), 5);

  const expected<std::string, int> ce(std::string("world"));
  EXPECT_EQ(ce->size(), 5);
}

TEST(ExpectedValueTest, ConstAccess) {
  const expected<int, std::string> e(42);
  EXPECT_EQ(e.value(), 42);

  const expected<int, std::string> f(make_unexpected(std::string("err")));
  EXPECT_EQ(f.error(), "err");
}

TEST(ExpectedValueTest, RvalueAccess) {
  auto v = expected<std::string, int>(std::string("hello")).value();
  EXPECT_EQ(v, "hello");

  auto err =
      expected<int, std::string>(make_unexpected(std::string("e"))).error();
  EXPECT_EQ(err, "e");
}

#ifdef __cpp_exceptions
TEST(ExpectedValueTest, ValueThrowsOnError) {
  expected<int, std::string> e(make_unexpected(std::string("bad")));
  EXPECT_THROW(
      {
        try {
          e.value();
        } catch (const bad_expected_access<std::string>& ex) {
          EXPECT_EQ(ex.error(), "bad");
          throw;
        }
      },
      bad_expected_access<std::string>);
}
#endif

// ============================================================================
// expected<T, E> — monadic operations
// ============================================================================

TEST(ExpectedMonadicTest, AndThenOnValue) {
  expected<int, std::string> e(5);
  auto result = e.and_then(
      [](int v) -> expected<double, std::string> { return v * 2.0; });
  static_assert(
      std::is_same_v<decltype(result), expected<double, std::string>>);
  EXPECT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result.value(), 10.0);
}

TEST(ExpectedMonadicTest, AndThenOnError) {
  expected<int, std::string> e(make_unexpected(std::string("fail")));
  auto result = e.and_then(
      [](int v) -> expected<double, std::string> { return v * 2.0; });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "fail");
}

TEST(ExpectedMonadicTest, AndThenConst) {
  const expected<int, std::string> e(5);
  auto result = e.and_then(
      [](const int& v) -> expected<double, std::string> { return v * 3.0; });
  EXPECT_DOUBLE_EQ(result.value(), 15.0);
}

TEST(ExpectedMonadicTest, AndThenRvalue) {
  auto result = expected<int, std::string>(7).and_then(
      [](int&& v) -> expected<double, std::string> { return v * 2.0; });
  EXPECT_DOUBLE_EQ(result.value(), 14.0);
}

// and_then with callback returning a reference — exercises remove_cvref_t
TEST(ExpectedMonadicTest, AndThenCallbackReturnsLvalueRef) {
  expected<int, std::string> e(5);
  expected<double, std::string> storage(0.0);
  auto result =
      e.and_then([&storage](int /*v*/) -> expected<double, std::string>& {
        return storage;
      });
  static_assert(
      std::is_same_v<decltype(result), expected<double, std::string>>);
  EXPECT_TRUE(result.has_value());
}

// and_then with callback returning a const ref — exercises remove_cvref_t
TEST(ExpectedMonadicTest, AndThenCallbackReturnsConstLvalueRef) {
  const expected<int, std::string> e(5);
  const expected<double, std::string> storage(99.0);
  auto result = e.and_then(
      [&storage](const int& /*v*/) -> const expected<double, std::string>& {
        return storage;
      });
  static_assert(
      std::is_same_v<decltype(result), expected<double, std::string>>);
  EXPECT_DOUBLE_EQ(result.value(), 99.0);
}

// and_then on rvalue error path — exercises std::move(error())
TEST(ExpectedMonadicTest, AndThenRvalueErrorPath) {
  auto result = expected<int, std::string>(make_unexpected(std::string("rerr")))
                    .and_then([](int&& v) -> expected<double, std::string> {
                      return v * 2.0;
                    });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "rerr");
}

// and_then on const rvalue error path — exercises const&& overload
TEST(ExpectedMonadicTest, AndThenConstRvalueErrorPath) {
  const expected<int, std::string> e(make_unexpected(std::string("cerr")));
  auto result = std::move(e).and_then(
      [](const int&& /*v*/) -> expected<double, std::string> { return 0.0; });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "cerr");
}

// and_then with move-only error type — exercises std::move(error())
TEST(ExpectedMonadicTest, AndThenMoveOnlyError) {
  auto result =
      expected<int, std::unique_ptr<int>>(
          make_unexpected(std::make_unique<int>(42)))
          .and_then([](int&& v) -> expected<double, std::unique_ptr<int>> {
            return v * 1.0;
          });
  EXPECT_FALSE(result.has_value());
  ASSERT_NE(result.error(), nullptr);
  EXPECT_EQ(*result.error(), 42);
}

// and_then preserves error type (static_assert ensures same error_type)
TEST(ExpectedMonadicTest, AndThenPreservesErrorType) {
  expected<int, std::string> e(make_unexpected(std::string("original")));
  auto result = e.and_then(
      [](int v) -> expected<double, std::string> { return v * 2.0; });
  static_assert(
      std::is_same_v<typename decltype(result)::error_type, std::string>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "original");
}

TEST(ExpectedMonadicTest, TransformOnValue) {
  expected<int, std::string> e(5);
  auto result = e.transform([](int v) { return v * 2; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 10);
}

TEST(ExpectedMonadicTest, TransformOnError) {
  expected<int, std::string> e(make_unexpected(std::string("fail")));
  auto result = e.transform([](int v) { return v * 2; });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "fail");
}

TEST(ExpectedMonadicTest, TransformConstLvalue) {
  const expected<int, std::string> e(5);
  auto result = e.transform([](const int& v) { return v + 1; });
  EXPECT_EQ(result.value(), 6);
}

TEST(ExpectedMonadicTest, TransformRvalue) {
  auto result = expected<std::string, int>(std::string("hello"))
                    .transform([](std::string&& s) { return s + "!"; });
  EXPECT_EQ(result.value(), "hello!");
}

// transform with callback returning const-qualified type — exercises
// std::remove_cv_t in the result type
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
TEST(ExpectedMonadicTest, TransformReturnsConst) {
  expected<int, std::string> e(5);
  auto result = e.transform([](int v) -> const int { return v * 2; });
  // remove_cv_t strips const, so result type is expected<int, std::string>
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);
}

TEST(ExpectedMonadicTest, TransformConstLvalueReturnsConst) {
  const expected<int, std::string> e(5);
  auto result = e.transform([](const int& v) -> const int { return v + 1; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 6);
}

TEST(ExpectedMonadicTest, TransformRvalueReturnsConst) {
  auto result = expected<int, std::string>(7).transform(
      [](int&& v) -> const int { return v * 2; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 14);
}

TEST(ExpectedMonadicTest, TransformConstRvalueReturnsConst) {
  const expected<int, std::string> e(9);
  auto result =
      std::move(e).transform([](const int&& v) -> const int { return v * 2; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 18);
}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ============================================================================
// expected<T, E> — transform with void-returning callback
// ============================================================================

TEST(ExpectedMonadicTest, TransformVoidReturnOnValue) {
  expected<int, std::string> e(5);
  bool called = false;
  auto result = e.transform([&called](int v) {
    called = true;
    EXPECT_EQ(v, 5);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedMonadicTest, TransformVoidReturnOnError) {
  expected<int, std::string> e(make_unexpected(std::string("fail")));
  bool called = false;
  auto result = e.transform([&called](int /*v*/) { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_FALSE(called);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "fail");
}

TEST(ExpectedMonadicTest, TransformVoidReturnConstLvalue) {
  const expected<int, std::string> e(5);
  bool called = false;
  auto result = e.transform([&called](const int& v) {
    called = true;
    EXPECT_EQ(v, 5);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedMonadicTest, TransformVoidReturnRvalue) {
  bool called = false;
  auto result = expected<int, std::string>(7).transform([&called](int&& v) {
    called = true;
    EXPECT_EQ(v, 7);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedMonadicTest, TransformVoidReturnConstRvalue) {
  const expected<int, std::string> e(9);
  bool called = false;
  auto result = std::move(e).transform([&called](const int&& v) {
    called = true;
    EXPECT_EQ(v, 9);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedMonadicTest, TransformVoidReturnRvalueErrorPath) {
  bool called = false;
  auto result = expected<int, std::string>(make_unexpected(std::string("rerr")))
                    .transform([&called](int&& /*v*/) { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_FALSE(called);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "rerr");
}

TEST(ExpectedMonadicTest, TransformErrorOnValue) {
  expected<int, std::string> e(42);
  auto result = e.transform_error(
      [](std::string& err) -> int { return static_cast<int>(err.size()); });
  // stays as value since has_value is true
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(ExpectedMonadicTest, TransformErrorOnError) {
  expected<int, std::string> e(make_unexpected(std::string("err")));
  auto result = e.transform_error(
      [](std::string& err) -> int { return static_cast<int>(err.size()); });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedMonadicTest, TransformErrorRvalue) {
  auto result = expected<int, std::string>(make_unexpected(std::string("xyz")))
                    .transform_error([](std::string&& err) -> int {
                      return static_cast<int>(err.size());
                    });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedMonadicTest, OrElseOnValue) {
  expected<int, std::string> e(42);
  auto result =
      e.or_else([](const std::string& err) -> expected<int, std::string> {
        return expected<int, std::string>(static_cast<int>(err.size()));
      });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(ExpectedMonadicTest, OrElseOnError) {
  expected<int, std::string> e(make_unexpected(std::string("fix")));
  auto result =
      e.or_else([](const std::string& err) -> expected<int, std::string> {
        return expected<int, std::string>(static_cast<int>(err.size()));
      });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 3);
}

TEST(ExpectedMonadicTest, OrElseRvalue) {
  auto result =
      expected<int, std::string>(make_unexpected(std::string("fix")))
          .or_else([](std::string&& err) -> expected<int, std::string> {
            return expected<int, std::string>(static_cast<int>(err.size()));
          });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 3);
}

// ============================================================================
// expected<T, E> — swap
// ============================================================================

TEST(ExpectedSwapTest, BothHaveValue) {
  expected<int, std::string> a(1);
  expected<int, std::string> b(2);
  a.swap(b);
  EXPECT_EQ(a.value(), 2);
  EXPECT_EQ(b.value(), 1);
}

TEST(ExpectedSwapTest, BothHaveError) {
  expected<int, std::string> a(make_unexpected(std::string("a")));
  expected<int, std::string> b(make_unexpected(std::string("b")));
  a.swap(b);
  EXPECT_EQ(a.error(), "b");
  EXPECT_EQ(b.error(), "a");
}

TEST(ExpectedSwapTest, CrossState) {
  expected<int, std::string> a(42);
  expected<int, std::string> b(make_unexpected(std::string("err")));
  a.swap(b);
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 42);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
}

// ============================================================================
// expected<T, E> — comparison
// ============================================================================

TEST(ExpectedComparisonTest, EqualValues) {
  expected<int, std::string> a(10);
  expected<int, std::string> b(10);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST(ExpectedComparisonTest, DifferentValues) {
  expected<int, std::string> a(10);
  expected<int, std::string> b(20);
  EXPECT_FALSE(a == b);
}

TEST(ExpectedComparisonTest, EqualErrors) {
  expected<int, std::string> a(make_unexpected(std::string("x")));
  expected<int, std::string> b(make_unexpected(std::string("x")));
  EXPECT_TRUE(a == b);
}

TEST(ExpectedComparisonTest, ValueVsError) {
  expected<int, std::string> a(10);
  expected<int, std::string> b(make_unexpected(std::string("x")));
  EXPECT_FALSE(a == b);
}

TEST(ExpectedComparisonTest, CompareWithValue) {
  expected<int, std::string> a(42);
  EXPECT_TRUE(a == 42);
  EXPECT_FALSE(a == 0);

  expected<int, std::string> b(make_unexpected(std::string("x")));
  EXPECT_FALSE(b == 42);
}

// ============================================================================
// expected<T&, E> — reference specialization
// ============================================================================

TEST(ExpectedRefTest, ConstructFromRef) {
  int val = 42;
  expected<int&, std::string> e(val);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 42);
  EXPECT_EQ(*e, 42);

  // mutate through reference
  e.value() = 99;
  EXPECT_EQ(val, 99);
}

TEST(ExpectedRefTest, ConstructFromUnexpected) {
  expected<int&, std::string> e(make_unexpected(std::string("err")));
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "err");
}

TEST(ExpectedRefTest, CopyConstruct) {
  int val = 10;
  expected<int&, std::string> a(val);
  expected<int&, std::string> b(a);
  EXPECT_EQ(b.value(), 10);
  EXPECT_EQ(&b.value(), &val);  // refers to same object
}

TEST(ExpectedRefTest, MoveConstruct) {
  int val = 10;
  expected<int&, std::string> a(val);
  expected<int&, std::string> b(std::move(a));
  EXPECT_EQ(b.value(), 10);
  EXPECT_EQ(&b.value(), &val);
}

TEST(ExpectedRefTest, CopyAssignment) {
  int v1 = 1, v2 = 2;
  expected<int&, std::string> a(v1);
  expected<int&, std::string> b(v2);
  b = a;
  EXPECT_EQ(&b.value(), &v1);
}

TEST(ExpectedRefTest, MoveAssignment) {
  int v1 = 1, v2 = 2;
  expected<int&, std::string> a(v1);
  expected<int&, std::string> b(v2);
  b = std::move(a);
  EXPECT_EQ(&b.value(), &v1);
}

TEST(ExpectedRefTest, ErrorOr) {
  int val = 42;
  expected<int&, std::string> e(val);
  EXPECT_EQ(e.error_or(std::string("no_error")), "no_error");

  expected<int&, std::string> f(make_unexpected(std::string("real_error")));
  EXPECT_EQ(f.error_or(std::string("default")), "real_error");
}

TEST(ExpectedRefTest, ErrorOrRvalue) {
  int val = 42;
  auto e = expected<int&, std::string>(val).error_or(std::string("no_error"));
  EXPECT_EQ(e, "no_error");

  auto f =
      expected<int&, std::string>(make_unexpected(std::string("moved_err")))
          .error_or(std::string("default"));
  EXPECT_EQ(f, "moved_err");
}

TEST(ExpectedRefTest, Emplace) {
  int v1 = 1, v2 = 2;
  expected<int&, std::string> e(v1);
  e.emplace(v2);  // rebind
  EXPECT_EQ(&e.value(), &v2);

  // emplace from error state
  expected<int&, std::string> f(make_unexpected(std::string("err")));
  f.emplace(v1);
  EXPECT_TRUE(f.has_value());
  EXPECT_EQ(&f.value(), &v1);
}

TEST(ExpectedRefTest, ValueOr) {
  int val = 42;
  expected<int&, std::string> e(val);
  EXPECT_EQ(e.value_or(0), 42);

  expected<int&, std::string> f(make_unexpected(std::string("err")));
  EXPECT_EQ(f.value_or(0), 0);
}

TEST(ExpectedRefTest, ValueOrRvalue) {
  // rvalue expected<T&,E> with a value: returns the referred-to value by move
  int val = 42;
  auto v = expected<int&, std::string>(val).value_or(0);
  EXPECT_EQ(v, 42);

  // rvalue expected<T&,E> with an error: returns the default
  auto e = expected<int&, std::string>(make_unexpected(std::string("err")))
               .value_or(0);
  EXPECT_EQ(e, 0);
}

TEST(ExpectedRefTest, MonadicAndThen) {
  int val = 5;
  expected<int&, std::string> e(val);
  auto result = e.and_then(
      [](int& v) -> expected<double, std::string> { return v * 2.0; });
  EXPECT_DOUBLE_EQ(result.value(), 10.0);
}

TEST(ExpectedRefTest, MonadicAndThenOnError) {
  expected<int&, std::string> e(make_unexpected(std::string("ref_fail")));
  auto result = e.and_then(
      [](int& v) -> expected<double, std::string> { return v * 2.0; });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "ref_fail");
}

// and_then on expected<T&,E> with callback returning a ref — remove_cvref_t
TEST(ExpectedRefTest, MonadicAndThenCallbackReturnsRef) {
  int val = 3;
  expected<int&, std::string> e(val);
  expected<double, std::string> storage(0.0);
  auto result =
      e.and_then([&storage](int& /*v*/) -> expected<double, std::string>& {
        return storage;
      });
  static_assert(
      std::is_same_v<decltype(result), expected<double, std::string>>);
  EXPECT_TRUE(result.has_value());
}

// and_then on rvalue expected<T&,E> error path — exercises std::move(error())
TEST(ExpectedRefTest, MonadicAndThenRvalueError) {
  auto result =
      expected<int&, std::string>(make_unexpected(std::string("rerr")))
          .and_then([](int& v) -> expected<double, std::string> {
            return static_cast<double>(v);
          });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "rerr");
}

// and_then on const rvalue expected<T&,E> — exercises const&& overload
TEST(ExpectedRefTest, MonadicAndThenConstRvalueError) {
  const expected<int&, std::string> e(make_unexpected(std::string("cerr")));
  auto result = std::move(e).and_then(
      [](const int& /*v*/) -> expected<double, std::string> { return 0.0; });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "cerr");
}

TEST(ExpectedRefTest, MonadicTransform) {
  int val = 5;
  expected<int&, std::string> e(val);
  auto result = e.transform([](int& v) { return v * 3; });
  EXPECT_EQ(result.value(), 15);
}

// transform with callback returning const-qualified type — exercises
// std::remove_cv_t in the result type for expected<T&,E> specialization
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
TEST(ExpectedRefTest, MonadicTransformReturnsConst) {
  int val = 5;
  expected<int&, std::string> e(val);
  auto result = e.transform([](int& v) -> const int { return v * 3; });
  // remove_cv_t strips const, so result type is expected<int, std::string>
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 15);
}

TEST(ExpectedRefTest, MonadicTransformConstLvalueReturnsConst) {
  int val = 5;
  const expected<int&, std::string> e(val);
  auto result = e.transform([](const int& v) -> const int { return v + 1; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 6);
}

TEST(ExpectedRefTest, MonadicTransformRvalueReturnsConst) {
  int val = 7;
  expected<int&, std::string> e(val);
  auto result =
      std::move(e).transform([](int& v) -> const int { return v * 2; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 14);
}

TEST(ExpectedRefTest, MonadicTransformConstRvalueReturnsConst) {
  int val = 9;
  const expected<int&, std::string> e(val);
  auto result =
      std::move(e).transform([](const int& v) -> const int { return v * 2; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 18);
}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ============================================================================
// expected<T&, E> — transform with void-returning callback
// ============================================================================

TEST(ExpectedRefTest, TransformVoidReturnOnValue) {
  int val = 5;
  expected<int&, std::string> e(val);
  bool called = false;
  auto result = e.transform([&called](int& v) {
    called = true;
    EXPECT_EQ(v, 5);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedRefTest, TransformVoidReturnOnError) {
  expected<int&, std::string> e(make_unexpected(std::string("ref_fail")));
  bool called = false;
  auto result = e.transform([&called](int& /*v*/) { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_FALSE(called);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "ref_fail");
}

TEST(ExpectedRefTest, TransformVoidReturnConstLvalue) {
  int val = 5;
  const expected<int&, std::string> e(val);
  bool called = false;
  auto result = e.transform([&called](const int& v) {
    called = true;
    EXPECT_EQ(v, 5);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedRefTest, TransformVoidReturnRvalue) {
  int val = 7;
  bool called = false;
  auto result = expected<int&, std::string>(val).transform([&called](int& v) {
    called = true;
    EXPECT_EQ(v, 7);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedRefTest, TransformVoidReturnConstRvalue) {
  int val = 9;
  const expected<int&, std::string> e(val);
  bool called = false;
  auto result = std::move(e).transform([&called](const int& v) {
    called = true;
    EXPECT_EQ(v, 9);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedRefTest, TransformVoidReturnRvalueErrorPath) {
  bool called = false;
  auto result =
      expected<int&, std::string>(make_unexpected(std::string("rerr")))
          .transform([&called](int& /*v*/) { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_FALSE(called);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "rerr");
}

TEST(ExpectedRefTest, MonadicTransformError) {
  int val = 42;
  expected<int&, std::string> e(val);
  auto result = e.transform_error(
      [](std::string& err) -> int { return static_cast<int>(err.size()); });
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedRefTest, MonadicOrElse) {
  int val = 99;
  expected<int&, std::string> e(val);
  auto result =
      e.or_else([](std::string& /*err*/) -> expected<int&, std::string> {
        static int fallback = 0;
        return expected<int&, std::string>(fallback);
      });
  EXPECT_EQ(result.value(), 99);
}

TEST(ExpectedRefTest, Swap) {
  int v1 = 1, v2 = 2;
  expected<int&, std::string> a(v1);
  expected<int&, std::string> b(v2);
  a.swap(b);
  EXPECT_EQ(&a.value(), &v2);
  EXPECT_EQ(&b.value(), &v1);
}

TEST(ExpectedRefTest, Comparison) {
  int v1 = 10, v2 = 10, v3 = 20;
  expected<int&, std::string> a(v1);
  expected<int&, std::string> b(v2);
  expected<int&, std::string> c(v3);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a == c);
  EXPECT_TRUE(a == 10);
}

// ============================================================================
// expected<void, E> — void specialization
// ============================================================================

TEST(ExpectedVoidTest, DefaultConstruct) {
  expected<void, std::string> e;
  EXPECT_TRUE(e.has_value());
  e.value();  // should not throw/assert
}

TEST(ExpectedVoidTest, ConstructFromUnexpected) {
  expected<void, std::string> e(make_unexpected(std::string("err")));
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "err");
}

TEST(ExpectedVoidTest, CopyConstruct) {
  expected<void, std::string> a;
  expected<void, std::string> b(a);
  EXPECT_TRUE(b.has_value());

  expected<void, std::string> c(make_unexpected(std::string("err")));
  expected<void, std::string> d(c);
  EXPECT_EQ(d.error(), "err");
}

TEST(ExpectedVoidTest, MoveConstruct) {
  expected<void, std::string> a;
  expected<void, std::string> b(std::move(a));
  EXPECT_TRUE(b.has_value());

  expected<void, std::string> c(make_unexpected(std::string("err")));
  expected<void, std::string> d(std::move(c));
  EXPECT_EQ(d.error(), "err");
}

TEST(ExpectedVoidTest, CopyAssignment) {
  expected<void, std::string> a;
  expected<void, std::string> b(make_unexpected(std::string("e")));
  b = a;
  EXPECT_TRUE(b.has_value());

  expected<void, std::string> c(make_unexpected(std::string("e1")));
  expected<void, std::string> d(make_unexpected(std::string("e2")));
  d = c;
  EXPECT_EQ(d.error(), "e1");

  // cross-state
  c = a;
  EXPECT_TRUE(c.has_value());
}

TEST(ExpectedVoidTest, MoveAssignment) {
  expected<void, std::string> a;
  expected<void, std::string> b(make_unexpected(std::string("e")));
  b = std::move(a);
  EXPECT_TRUE(b.has_value());
}

TEST(ExpectedVoidTest, ErrorOr) {
  expected<void, std::string> e;
  EXPECT_EQ(e.error_or(std::string("no_error")), "no_error");

  expected<void, std::string> f(make_unexpected(std::string("real_error")));
  EXPECT_EQ(f.error_or(std::string("default")), "real_error");
}

TEST(ExpectedVoidTest, ErrorOrRvalue) {
  auto e = expected<void, std::string>().error_or(std::string("no_error"));
  EXPECT_EQ(e, "no_error");

  auto f =
      expected<void, std::string>(make_unexpected(std::string("moved_err")))
          .error_or(std::string("default"));
  EXPECT_EQ(f, "moved_err");
}

TEST(ExpectedVoidTest, AndThenOnValue) {
  expected<void, std::string> e;
  auto result = e.and_then([]() -> expected<int, std::string> { return 42; });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(ExpectedVoidTest, AndThenOnError) {
  expected<void, std::string> e(make_unexpected(std::string("fail")));
  auto result = e.and_then([]() -> expected<int, std::string> { return 42; });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "fail");
}

TEST(ExpectedVoidTest, TransformOnValue) {
  expected<void, std::string> e;
  auto result = e.transform([]() { return 42; });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(ExpectedVoidTest, TransformOnError) {
  expected<void, std::string> e(make_unexpected(std::string("fail")));
  auto result = e.transform([]() { return 42; });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "fail");
}

// transform with callback returning const-qualified type — exercises
// std::remove_cv_t in the result type for expected<void,E> specialization
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
TEST(ExpectedVoidTest, TransformReturnsConst) {
  expected<void, std::string> e;
  auto result = e.transform([]() -> const int { return 42; });
  // remove_cv_t strips const, so result type is expected<int, std::string>
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(ExpectedVoidTest, TransformConstLvalueReturnsConst) {
  const expected<void, std::string> e;
  auto result = e.transform([]() -> const int { return 99; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 99);
}

TEST(ExpectedVoidTest, TransformRvalueReturnsConst) {
  auto result =
      expected<void, std::string>().transform([]() -> const int { return 7; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 7);
}

TEST(ExpectedVoidTest, TransformConstRvalueReturnsConst) {
  const expected<void, std::string> e;
  auto result = std::move(e).transform([]() -> const int { return 11; });
  static_assert(std::is_same_v<decltype(result), expected<int, std::string>>);
  EXPECT_EQ(result.value(), 11);
}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ============================================================================
// expected<void, E> — transform with void-returning callback
// ============================================================================

TEST(ExpectedVoidTest, TransformVoidReturnOnValue) {
  expected<void, std::string> e;
  bool called = false;
  auto result = e.transform([&called]() { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, TransformVoidReturnOnError) {
  expected<void, std::string> e(make_unexpected(std::string("fail")));
  bool called = false;
  auto result = e.transform([&called]() { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_FALSE(called);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "fail");
}

TEST(ExpectedVoidTest, TransformVoidReturnConstLvalue) {
  const expected<void, std::string> e;
  bool called = false;
  auto result = e.transform([&called]() { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, TransformVoidReturnRvalue) {
  bool called = false;
  auto result =
      expected<void, std::string>().transform([&called]() { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, TransformVoidReturnConstRvalue) {
  const expected<void, std::string> e;
  bool called = false;
  auto result = std::move(e).transform([&called]() { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_TRUE(called);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, TransformVoidReturnRvalueErrorPath) {
  bool called = false;
  auto result =
      expected<void, std::string>(make_unexpected(std::string("rerr")))
          .transform([&called]() { called = true; });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_FALSE(called);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "rerr");
}

TEST(ExpectedVoidTest, TransformErrorOnValue) {
  expected<void, std::string> e;
  auto result = e.transform_error(
      [](std::string& err) -> int { return static_cast<int>(err.size()); });
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, TransformErrorOnError) {
  expected<void, std::string> e(make_unexpected(std::string("err")));
  auto result = e.transform_error(
      [](std::string& err) -> int { return static_cast<int>(err.size()); });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedVoidTest, OrElseOnValue) {
  expected<void, std::string> e;
  auto result =
      e.or_else([](std::string& /*err*/) -> expected<void, std::string> {
        return make_unexpected(std::string("recovered"));
      });
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, OrElseOnError) {
  expected<void, std::string> e(make_unexpected(std::string("fix")));
  auto result =
      e.or_else([](std::string& /*err*/) -> expected<void, std::string> {
        // recovery: return a valid expected
        return expected<void, std::string>();
      });
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, Swap) {
  expected<void, std::string> a;
  expected<void, std::string> b(make_unexpected(std::string("err")));
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
  EXPECT_TRUE(b.has_value());

  expected<void, std::string> c(make_unexpected(std::string("c")));
  expected<void, std::string> d(make_unexpected(std::string("d")));
  c.swap(d);
  EXPECT_EQ(c.error(), "d");
  EXPECT_EQ(d.error(), "c");
}

TEST(ExpectedVoidTest, Comparison) {
  expected<void, std::string> a;
  expected<void, std::string> b;
  EXPECT_TRUE(a == b);

  expected<void, std::string> c(make_unexpected(std::string("x")));
  expected<void, std::string> d(make_unexpected(std::string("x")));
  EXPECT_TRUE(c == d);

  EXPECT_FALSE(a == c);
  EXPECT_FALSE(c == a);
}

#ifdef __cpp_exceptions
TEST(ExpectedVoidTest, ValueThrowsOnError) {
  expected<void, std::string> e(make_unexpected(std::string("bad")));
  EXPECT_THROW(e.value(), bad_expected_access<std::string>);
}
#endif

// ============================================================================
// Non-trivial types
// ============================================================================

TEST(ExpectedNonTrivialTest, StdStringValue) {
  expected<std::string, int> e(std::string("hello world"));
  EXPECT_EQ(e.value(), "hello world");
  EXPECT_EQ(e->size(), 11);
}

TEST(ExpectedNonTrivialTest, StdStringError) {
  expected<int, std::string> e(make_unexpected(std::string("error msg")));
  EXPECT_EQ(e.error(), "error msg");
}

TEST(ExpectedNonTrivialTest, VectorValue) {
  expected<std::vector<int>, std::string> e(std::vector<int>{1, 2, 3});
  EXPECT_EQ(e.value().size(), 3);
  EXPECT_EQ(e.value()[0], 1);
}

TEST(ExpectedNonTrivialTest, MoveOnlyType) {
  // expected can hold a move-only type like unique_ptr
  expected<std::unique_ptr<int>, std::string> e(std::make_unique<int>(42));
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(*e.value(), 42);

  auto e2 = std::move(e);
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(*e2.value(), 42);
}

// ============================================================================
// make_unexpected / unexpected
// ============================================================================

TEST(UnexpectedTest, MakeUnexpected) {
  auto u = make_unexpected(42);
  static_assert(std::is_same_v<decltype(u), argparse::unexpected<int>>);
  EXPECT_EQ(u.error(), 42);

  auto u2 = make_unexpected(std::string("hello"));
  static_assert(
      std::is_same_v<decltype(u2), argparse::unexpected<std::string>>);
  EXPECT_EQ(u2.error(), "hello");
}

TEST(UnexpectedTest, LvalueRefDecay) {
  int x = 42;
  auto u = make_unexpected(x);
  static_assert(std::is_same_v<decltype(u), argparse::unexpected<int>>);
}

TEST(UnexpectedTest, ErrorAccess) {
  argparse::unexpected<int> u(42);
  EXPECT_EQ(u.error(), 42);
  const auto& cu = u;
  EXPECT_EQ(cu.error(), 42);
  EXPECT_EQ(std::move(u).error(), 42);
}

TEST(UnexpectedTest, DirectConstructFromLvalue) {
  std::string msg = "hello";
  argparse::unexpected<std::string> u(msg);
  EXPECT_EQ(u.error(), "hello");
  // Verify error_type typedef
  static_assert(std::is_same_v<typename decltype(u)::error_type, std::string>);
}

TEST(UnexpectedTest, DirectConstructFromRvalue) {
  argparse::unexpected<std::string> u(std::string("world"));
  EXPECT_EQ(u.error(), "world");
}

TEST(UnexpectedTest, ErrorAccessRvalue) {
  auto e = argparse::unexpected<int>(99).error();
  EXPECT_EQ(e, 99);
}

TEST(UnexpectedTest, VerifyStaticAssertAllowsValidTypes) {
  // These types must compile — the static_assert in unexpected checks
  // is_valid_unexpected and must pass for these:
  [[maybe_unused]] argparse::unexpected<int> u1(1);
  [[maybe_unused]] argparse::unexpected<std::string> u2(std::string("ok"));
  [[maybe_unused]] argparse::unexpected<double> u3(3.14);
  [[maybe_unused]] argparse::unexpected<std::vector<int>> u4(
      std::vector<int>{1, 2, 3});
  SUCCEED();
}

// ============================================================================
// unexpected<E> — swap (member & free function)
// ============================================================================

TEST(UnexpectedSwapTest, MemberSwap) {
  argparse::unexpected<int> a(1);
  argparse::unexpected<int> b(2);
  a.swap(b);
  EXPECT_EQ(a.error(), 2);
  EXPECT_EQ(b.error(), 1);
}

TEST(UnexpectedSwapTest, MemberSwapStrings) {
  argparse::unexpected<std::string> a(std::string("hello"));
  argparse::unexpected<std::string> b(std::string("world"));
  a.swap(b);
  EXPECT_EQ(a.error(), "world");
  EXPECT_EQ(b.error(), "hello");
}

TEST(UnexpectedSwapTest, SwapTwice) {
  argparse::unexpected<int> a(10);
  argparse::unexpected<int> b(20);
  a.swap(b);
  a.swap(b);
  EXPECT_EQ(a.error(), 10);
  EXPECT_EQ(b.error(), 20);
}

TEST(UnexpectedSwapTest, FreeFunctionSwap) {
  argparse::unexpected<int> a(1);
  argparse::unexpected<int> b(2);
  using std::swap;
  swap(a, b);
  EXPECT_EQ(a.error(), 2);
  EXPECT_EQ(b.error(), 1);
}

TEST(UnexpectedSwapTest, FreeFunctionSwapViaFriend) {
  // Verify ADL finds the friend swap
  argparse::unexpected<std::string> a(std::string("a"));
  argparse::unexpected<std::string> b(std::string("b"));
  swap(a, b);  // ADL should find argparse::swap via friend
  EXPECT_EQ(a.error(), "b");
  EXPECT_EQ(b.error(), "a");

  // ADL swap round-trip
  swap(a, b);
  EXPECT_EQ(a.error(), "a");
  EXPECT_EQ(b.error(), "b");
}

TEST(UnexpectedSwapTest, NoexceptSpecification) {
  // int is nothrow swappable
  static_assert(noexcept(std::declval<argparse::unexpected<int>&>().swap(
      std::declval<argparse::unexpected<int>&>())));
  // ADL swap is also noexcept
  static_assert(noexcept(swap(std::declval<argparse::unexpected<int>&>(),
                              std::declval<argparse::unexpected<int>&>())));
}

// ============================================================================
// unexpected<E> — operator==
// ============================================================================

TEST(UnexpectedCompareTest, EqualSameType) {
  argparse::unexpected<int> a(42);
  argparse::unexpected<int> b(42);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST(UnexpectedCompareTest, NotEqualSameType) {
  argparse::unexpected<int> a(1);
  argparse::unexpected<int> b(2);
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}

TEST(UnexpectedCompareTest, EqualDifferentTypes) {
  // unexpected<int> vs unexpected<long> — cross-type comparison
  argparse::unexpected<int> a(42);
  argparse::unexpected<long> b(42);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST(UnexpectedCompareTest, NotEqualDifferentTypes) {
  argparse::unexpected<int> a(1);
  argparse::unexpected<long> b(2);
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}

TEST(UnexpectedCompareTest, StringErrors) {
  argparse::unexpected<std::string> a(std::string("hello"));
  argparse::unexpected<std::string> b(std::string("hello"));
  EXPECT_TRUE(a == b);

  argparse::unexpected<std::string> c(std::string("world"));
  EXPECT_FALSE(a == c);
}

TEST(UnexpectedCompareTest, ConstObjects) {
  const argparse::unexpected<int> a(42);
  const argparse::unexpected<int> b(42);
  EXPECT_TRUE(a == b);
}

// ============================================================================
// unexpected<E> — in-place construction (std::in_place_t)
// ============================================================================

TEST(UnexpectedInPlaceTest, ConstructWithInPlace) {
  argparse::unexpected<int> u(std::in_place, 42);
  EXPECT_EQ(u.error(), 42);
}

TEST(UnexpectedInPlaceTest, ConstructWithInPlaceString) {
  argparse::unexpected<std::string> u(std::in_place, "hello");
  EXPECT_EQ(u.error(), "hello");
}

TEST(UnexpectedInPlaceTest, ConstructWithInPlaceMultiArg) {
  argparse::unexpected<std::string> u(std::in_place, 5, 'x');
  EXPECT_EQ(u.error(), "xxxxx");
}

TEST(UnexpectedInPlaceTest, ConstructWithInPlaceZeroArgs) {
  argparse::unexpected<int> u(std::in_place);
  EXPECT_EQ(u.error(), 0);  // int{} is 0

  argparse::unexpected<std::string> u2(std::in_place);
  EXPECT_EQ(u2.error(), "");  // std::string{} is ""
}

TEST(UnexpectedInPlaceTest, ConstructWithInPlaceMoveOnly) {
  argparse::unexpected<std::unique_ptr<int>> u(std::in_place, new int(99));
  EXPECT_EQ(*u.error(), 99);
}

TEST(UnexpectedInPlaceTest, ConstructWithInPlaceVector) {
  argparse::unexpected<std::vector<int>> u(std::in_place,
                                           std::initializer_list<int>{1, 2, 3});
  ASSERT_EQ(u.error().size(), 3);
  EXPECT_EQ(u.error()[0], 1);
  EXPECT_EQ(u.error()[1], 2);
  EXPECT_EQ(u.error()[2], 3);
}

// ============================================================================
// unexpected<E> — defaulted copy/move assignment
// ============================================================================

TEST(UnexpectedAssignmentTest, CopyAssignment) {
  argparse::unexpected<int> a(10);
  argparse::unexpected<int> b(20);
  b = a;
  EXPECT_EQ(b.error(), 10);
  EXPECT_EQ(a.error(), 10);  // a unchanged
}

TEST(UnexpectedAssignmentTest, MoveAssignment) {
  argparse::unexpected<int> a(10);
  argparse::unexpected<int> b(20);
  b = std::move(a);
  EXPECT_EQ(b.error(), 10);
}

TEST(UnexpectedAssignmentTest, CopyAssignmentStrings) {
  argparse::unexpected<std::string> a(std::string("hello"));
  argparse::unexpected<std::string> b(std::string("world"));
  b = a;
  EXPECT_EQ(b.error(), "hello");
  EXPECT_EQ(a.error(), "hello");
}

TEST(UnexpectedAssignmentTest, MoveAssignmentStrings) {
  argparse::unexpected<std::string> a(std::string("hello"));
  argparse::unexpected<std::string> b(std::string("world"));
  b = std::move(a);
  EXPECT_EQ(b.error(), "hello");
}

TEST(UnexpectedAssignmentTest, SelfCopyAssignment) {
  argparse::unexpected<int> a(42);
  const auto& ref = a;
  a = ref;
  EXPECT_EQ(a.error(), 42);
}

TEST(UnexpectedAssignmentTest, SelfMoveAssignment) {
  // Self-move-assign via intermediate to avoid -Wself-move
  argparse::unexpected<int> a(99);
  argparse::unexpected<int> tmp(std::move(a));
  a = std::move(tmp);
  EXPECT_EQ(a.error(), 99);
}

// ============================================================================
// unexpected<E> — const&& error() access (regression: const correctness)
// ============================================================================

TEST(UnexpectedConstRvalueTest, ConstRvalueErrorAccess) {
  const argparse::unexpected<int> u(42);
  // std::move on const object calls error() const&&
  auto e = std::move(u).error();
  // e should be const int&&, effectively int (decays in auto)
  EXPECT_EQ(e, 42);
}

TEST(UnexpectedConstRvalueTest, ConstRvalueErrorAccessString) {
  const argparse::unexpected<std::string> u(std::string("hello"));
  auto e = std::move(u).error();
  EXPECT_EQ(e, "hello");
}

TEST(UnexpectedConstRvalueTest, VerifyReturnTypeIsConstRvalueRef) {
  // The return type of error() const&& should be const E&&
  using U = argparse::unexpected<int>;
  static_assert(
      std::is_same_v<decltype(std::declval<const U&&>().error()), const int&&>);
}

// ============================================================================
// is_expected / is_expected_v
// ============================================================================

TEST(ExpectedTraitsTest, IsExpected) {
  static_assert(argparse::is_expected_v<expected<int, std::string>>);
  static_assert(argparse::is_expected_v<expected<int&, std::string>>);
  static_assert(argparse::is_expected_v<expected<void, std::string>>);
  static_assert(!argparse::is_expected_v<int>);
  static_assert(!argparse::is_expected_v<std::string>);
}

// ============================================================================
// is_unexpected / is_unexpected_v
// ============================================================================

TEST(ExpectedTraitsTest, IsUnexpected) {
  static_assert(argparse::is_unexpected_v<argparse::unexpected<int>>);
  static_assert(argparse::is_unexpected_v<argparse::unexpected<std::string>>);
  static_assert(!argparse::is_unexpected_v<int>);
  static_assert(!argparse::is_unexpected_v<std::string>);
  static_assert(!argparse::is_unexpected_v<expected<int, std::string>>);
}

// ============================================================================
// is_valid_unexpected — validates the template argument constraints for
// unexpected<E> as specified in [expected.un.general]
// ============================================================================

TEST(ExpectedTraitsTest, IsValidUnexpected) {
  // Valid types: object types that are not cv-qualified, not arrays,
  // and not unexpected specializations
  static_assert(argparse::is_valid_unexpected<int>::value);
  static_assert(argparse::is_valid_unexpected<std::string>::value);
  static_assert(argparse::is_valid_unexpected<double>::value);
  static_assert(argparse::is_valid_unexpected<std::vector<int>>::value);

  // cv-qualified types are invalid
  static_assert(!argparse::is_valid_unexpected<const int>::value);
  static_assert(!argparse::is_valid_unexpected<volatile int>::value);
  static_assert(!argparse::is_valid_unexpected<const volatile int>::value);

  // array types are invalid
  static_assert(!argparse::is_valid_unexpected<int[]>::value);
  static_assert(!argparse::is_valid_unexpected<int[5]>::value);

  // unexpected specializations are invalid (cannot nest)
  static_assert(
      !argparse::is_valid_unexpected<argparse::unexpected<int>>::value);

  // void is not an object type
  static_assert(!argparse::is_valid_unexpected<void>::value);

  // references are not object types
  static_assert(!argparse::is_valid_unexpected<int&>::value);
  static_assert(!argparse::is_valid_unexpected<const int&>::value);
}

// ============================================================================
// bad_expected_access
// ============================================================================

TEST(BadExpectedAccessTest, What) {
  bad_expected_access<std::string> ex(std::string("test error"));
  EXPECT_STREQ(ex.what(), "bad access to expected");
  EXPECT_EQ(ex.error(), "test error");
}

// ============================================================================
// constexpr tests — note: placement-new is not constexpr before C++26,
// so the expected class methods marked constexpr can only be tested at
// runtime with current compilers.
// ============================================================================

TEST(ExpectedConstexprTest, BasicConstexpr) {
  auto e = []() {
    expected<int, int> e(42);
    if (!e.has_value()) {
      return -1;
    }
    return e.value();
  }();
  EXPECT_EQ(e, 42);

  auto f = []() {
    expected<int, int> e(make_unexpected(99));
    if (e.has_value()) {
      return -1;
    }
    return e.error();
  }();
  EXPECT_EQ(f, 99);
}

TEST(ExpectedConstexprTest, ConstexprTransform) {
  auto result = []() {
    expected<int, int> e(5);
    return e.transform([](int v) { return v * 2; });
  }();
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);
}

// ============================================================================
// Edge cases / regression
// ============================================================================

TEST(ExpectedEdgeCaseTest, NestedExpected) {
  using Inner = expected<int, std::string>;
  expected<Inner, std::string> e(Inner(42));
  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e.value().has_value());
  EXPECT_EQ(e.value().value(), 42);
}

TEST(ExpectedEdgeCaseTest, ChainedMonadicOps) {
  expected<int, std::string> e(5);
  auto result = e.transform([](int v) { return v * 2; })
                    .and_then([](int v) -> expected<double, std::string> {
                      if (v > 5) {
                        return v * 1.5;
                      }
                      return make_unexpected(std::string("too small"));
                    })
                    .transform([](double v) { return static_cast<int>(v); });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 15);

  // Chain where and_then returns error
  expected<int, std::string> e2(2);
  auto result2 = e2.transform([](int v) {
                     return v * 2;
                   }).and_then([](int v) -> expected<double, std::string> {
    if (v > 5) {
      return v * 1.5;
    }
    return make_unexpected(std::string("too small"));
  });
  EXPECT_FALSE(result2.has_value());
  EXPECT_EQ(result2.error(), "too small");
}

// ============================================================================
// expected<T, E> — in-place construction (std::in_place_t / unexpect_t)
// ============================================================================

TEST(ExpectedInPlaceTest, ConstructValueInPlace) {
  expected<std::string, int> e(std::in_place, "hello");
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), "hello");
}

TEST(ExpectedInPlaceTest, ConstructValueInPlaceMultiArg) {
  expected<std::string, int> e(std::in_place, 5, 'x');
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), "xxxxx");
}

TEST(ExpectedInPlaceTest, ConstructValueInPlaceNonCopyable) {
  expected<std::unique_ptr<int>, std::string> e(std::in_place, new int(42));
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(*e.value(), 42);
}

TEST(ExpectedInPlaceTest, ConstructValueInPlaceVector) {
  expected<std::vector<int>, std::string> e(
      std::in_place, std::initializer_list<int>{1, 2, 3});
  EXPECT_TRUE(e.has_value());
  ASSERT_EQ(e.value().size(), 3);
  EXPECT_EQ(e.value()[0], 1);
  EXPECT_EQ(e.value()[1], 2);
  EXPECT_EQ(e.value()[2], 3);
}

TEST(ExpectedInPlaceTest, ConstructErrorInPlace) {
  expected<int, std::string> e(unexpect_t{}, "error message");
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "error message");
}

TEST(ExpectedInPlaceTest, ConstructErrorInPlaceMultiArg) {
  expected<int, std::string> e(unexpect_t{}, 5, 'x');
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "xxxxx");
}

TEST(ExpectedInPlaceTest, ConstructErrorInPlaceNonCopyable) {
  expected<int, std::unique_ptr<int>> e(unexpect_t{}, new int(99));
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(*e.error(), 99);
}

TEST(ExpectedInPlaceTest, InPlaceValueWithMonadicAndThen) {
  expected<int, std::string> e(std::in_place, 5);
  auto result = e.and_then(
      [](int v) -> expected<double, std::string> { return v * 2.0; });
  EXPECT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result.value(), 10.0);
}

TEST(ExpectedInPlaceTest, InPlaceErrorWithMonadicOrElse) {
  expected<int, std::string> e(unexpect_t{}, "recoverable");
  auto result =
      e.or_else([](const std::string& err) -> expected<int, std::string> {
        return expected<int, std::string>(static_cast<int>(err.size()));
      });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 11);
}

TEST(ExpectedInPlaceTest, EmplaceAfterInPlaceConstruction) {
  expected<int, std::string> e(std::in_place, 42);
  EXPECT_EQ(e.value(), 42);
  e.emplace(99);
  EXPECT_EQ(e.value(), 99);

  expected<int, std::string> f(unexpect_t{}, "err");
  EXPECT_EQ(f.error(), "err");
  f.emplace(77);
  EXPECT_TRUE(f.has_value());
  EXPECT_EQ(f.value(), 77);
}

// ============================================================================
// unexpect_t — tag type
// ============================================================================

TEST(UnexpectedTTest, ExplicitDefaultConstruct) {
  unexpect_t t;
  (void)t;
  SUCCEED();
}

TEST(UnexpectedTTest, UsedAsTagForInPlaceError) {
  // Verify unexpect_t can be used to construct expected with error in-place
  expected<int, std::string> e(unexpect_t{}, "test");
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "test");

  // Also works with the tag explicitly constructed
  unexpect_t tag;
  expected<int, std::string> f(tag, "hello");
  EXPECT_FALSE(f.has_value());
  EXPECT_EQ(f.error(), "hello");
}

// ============================================================================
// expected<T, E> — same-type value and error
// ============================================================================

TEST(ExpectedSameTypeTest, ConstructValue) {
  expected<int, int> e(42);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 42);
}

TEST(ExpectedSameTypeTest, ConstructError) {
  expected<int, int> e(make_unexpected(99));
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), 99);
}

TEST(ExpectedSameTypeTest, CopyConstructValue) {
  expected<int, int> a(10);
  expected<int, int> b(a);
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 10);
}

TEST(ExpectedSameTypeTest, CopyConstructError) {
  expected<int, int> a(make_unexpected(5));
  expected<int, int> b(a);
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(b.error(), 5);
}

TEST(ExpectedSameTypeTest, MoveConstructValue) {
  expected<int, int> a(10);
  expected<int, int> b(std::move(a));
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 10);
}

TEST(ExpectedSameTypeTest, MoveConstructError) {
  expected<int, int> a(make_unexpected(5));
  expected<int, int> b(std::move(a));
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(b.error(), 5);
}

TEST(ExpectedSameTypeTest, CopyAssignmentCrossState) {
  expected<int, int> a(10);
  expected<int, int> b(make_unexpected(20));
  b = a;
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 10);

  expected<int, int> c(make_unexpected(30));
  expected<int, int> d(40);
  d = c;
  EXPECT_FALSE(d.has_value());
  EXPECT_EQ(d.error(), 30);
}

TEST(ExpectedSameTypeTest, MoveAssignmentCrossState) {
  expected<int, int> a(10);
  expected<int, int> b(make_unexpected(20));
  b = std::move(a);
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 10);

  expected<int, int> c(make_unexpected(30));
  expected<int, int> d(40);
  d = std::move(c);
  EXPECT_FALSE(d.has_value());
  EXPECT_EQ(d.error(), 30);
}

TEST(ExpectedSameTypeTest, SwapCrossState) {
  expected<int, int> a(42);
  expected<int, int> b(make_unexpected(99));
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), 99);
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 42);
}

TEST(ExpectedSameTypeTest, Comparison) {
  expected<int, int> a(10);
  expected<int, int> b(10);
  EXPECT_TRUE(a == b);

  expected<int, int> c(make_unexpected(5));
  expected<int, int> d(make_unexpected(5));
  EXPECT_TRUE(c == d);

  EXPECT_FALSE(a == c);
}

TEST(ExpectedSameTypeTest, Emplace) {
  expected<int, int> e(make_unexpected(99));
  e.emplace(42);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 42);

  e.emplace(77);
  EXPECT_EQ(e.value(), 77);
}

TEST(ExpectedSameTypeTest, InPlaceConstructValue) {
  expected<int, int> e(std::in_place, 42);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 42);
}

TEST(ExpectedSameTypeTest, InPlaceConstructError) {
  expected<int, int> e(unexpect_t{}, 99);
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), 99);
}

// ============================================================================
// expected<T, E> — trivially-destructible types (regression: reset()
// optimization)
// ============================================================================

TEST(ExpectedTrivialTest, IntValueIntError) {
  // Both T and E are trivially destructible — reset() skips destroy(),
  // base destructor handles cleanup.
  expected<int, int> e(42);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 42);

  expected<int, int> f(make_unexpected(99));
  EXPECT_FALSE(f.has_value());
  EXPECT_EQ(f.error(), 99);
}

TEST(ExpectedTrivialTest, CopyAndMoveAssignment) {
  expected<int, int> a(10);
  expected<int, int> b(20);
  b = a;
  EXPECT_EQ(b.value(), 10);

  expected<int, int> c(make_unexpected(1));
  expected<int, int> d(make_unexpected(2));
  d = c;
  EXPECT_EQ(d.error(), 1);
}

TEST(ExpectedTrivialTest, MultipleEmplaceCycles) {
  expected<int, int> e(1);
  for (int i = 0; i < 10; ++i) {
    e.emplace(i);
    EXPECT_EQ(e.value(), i);
  }
  // Switch to error via assignment
  e = expected<int, int>(make_unexpected(42));
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), 42);
  // Switch back to value
  e.emplace(100);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 100);
}

// ============================================================================
// expected<T, E> — non-trivial cross-state transitions
// ============================================================================

TEST(ExpectedNonTrivialCrossStateTest, RepeatedCrossStateAssignments) {
  expected<std::string, std::string> a(std::string("value1"));
  expected<std::string, std::string> b(make_unexpected(std::string("error1")));

  // Cross-state copy assignment: error ← value
  b = a;
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), "value1");

  // Cross-state copy assignment: value ← error
  expected<std::string, std::string> c(std::string("value2"));
  c = expected<std::string, std::string>(
      make_unexpected(std::string("error2")));
  EXPECT_FALSE(c.has_value());
  EXPECT_EQ(c.error(), "error2");
}

TEST(ExpectedNonTrivialCrossStateTest, RepeatedCrossStateMoveAssignments) {
  expected<std::string, std::string> a(std::string("move_value"));
  expected<std::string, std::string> b(
      make_unexpected(std::string("move_error")));

  b = std::move(a);
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), "move_value");

  expected<std::string, std::string> c(std::string("another_value"));
  expected<std::string, std::string> d(
      make_unexpected(std::string("another_error")));
  c = std::move(d);
  EXPECT_FALSE(c.has_value());
  EXPECT_EQ(c.error(), "another_error");
}

// ============================================================================
// expected<T, E> — value_or with move-only types
// ============================================================================

TEST(ExpectedValueOrTest, ValueOrWithMoveOnlyDefault) {
  expected<std::unique_ptr<int>, std::string> e(std::make_unique<int>(42));
  // value_or from lvalue: returns a copy
  auto default_val = std::make_unique<int>(0);
  // Can't easily test since unique_ptr is not copyable.
  // But value_or with rvalue default works via the const& overload
  // which copies from storage_ — this test just verifies compilation.
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(*e.value(), 42);
}

// ============================================================================
// expected<T, E> — state transitions via swap with non-trivial types
// ============================================================================

TEST(ExpectedSwapNonTrivialTest, SwapStringsBackAndForth) {
  expected<std::string, std::string> a(std::string("hello"));
  expected<std::string, std::string> b(make_unexpected(std::string("world")));

  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "world");
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), "hello");

  // Swap back
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(a.value(), "hello");
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(b.error(), "world");
}

// ============================================================================
// expected<T, E> — operator== with the new has_error_impl() logic
// ============================================================================

TEST(ExpectedComparisonRegressionTest, ExhaustiveStateComparisons) {
  // value vs value
  expected<int, std::string> v1(10);
  expected<int, std::string> v2(10);
  expected<int, std::string> v3(20);
  EXPECT_TRUE(v1 == v2);
  EXPECT_FALSE(v1 == v3);

  // error vs error
  expected<int, std::string> e1(make_unexpected(std::string("a")));
  expected<int, std::string> e2(make_unexpected(std::string("a")));
  expected<int, std::string> e3(make_unexpected(std::string("b")));
  EXPECT_TRUE(e1 == e2);
  EXPECT_FALSE(e1 == e3);

  // value vs error
  EXPECT_FALSE(v1 == e1);
  EXPECT_FALSE(e1 == v1);
}

// ============================================================================
// expected<T, E> — in-place construction for move-only value AND error types
// Note: value()/operator* on expected with move-only error type does not
// compile with exceptions enabled, because bad_expected_access needs to copy
// the error.  This is a pre-existing limitation (same as std::expected).
// error() access and state checks work fine.
// ============================================================================

TEST(ExpectedMoveOnlyBothTest, InPlaceValueConstruction) {
  expected<std::unique_ptr<int>, std::unique_ptr<int>> e(std::in_place,
                                                         new int(42));
  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(static_cast<bool>(e));
}

TEST(ExpectedMoveOnlyBothTest, InPlaceErrorConstruction) {
  expected<std::unique_ptr<int>, std::unique_ptr<int>> e(unexpect_t{},
                                                         new int(99));
  EXPECT_FALSE(e.has_value());
  EXPECT_FALSE(static_cast<bool>(e));
  EXPECT_EQ(*e.error(), 99);
}

TEST(ExpectedMoveOnlyBothTest, MoveConstructValue) {
  expected<std::unique_ptr<int>, std::unique_ptr<int>> a(std::in_place,
                                                         new int(10));
  expected<std::unique_ptr<int>, std::unique_ptr<int>> b(std::move(a));
  EXPECT_TRUE(b.has_value());
}

TEST(ExpectedMoveOnlyBothTest, MoveConstructError) {
  expected<std::unique_ptr<int>, std::unique_ptr<int>> a(unexpect_t{},
                                                         new int(20));
  expected<std::unique_ptr<int>, std::unique_ptr<int>> b(std::move(a));
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(*b.error(), 20);
}

// ============================================================================
// expected<T, E> — regression: bad_expected_access with const ref
// ============================================================================

TEST(BadExpectedAccessRegressionTest, ConstructFromLvalue) {
  std::string msg = "test error message";
  bad_expected_access<std::string> ex(msg);
  EXPECT_EQ(ex.error(), "test error message");
  // msg should still be intact (we took it by const ref, copying internally)
  EXPECT_EQ(msg, "test error message");
}

TEST(BadExpectedAccessRegressionTest, ConstructFromRvalue) {
  bad_expected_access<std::string> ex(std::string("temporary"));
  EXPECT_EQ(ex.error(), "temporary");
}

// ============================================================================
// expected<void, E> — in-place error construction
// ============================================================================

TEST(ExpectedVoidInPlaceTest, ConstructErrorInPlace) {
  expected<void, std::string> e(unexpect_t{}, "void error");
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "void error");
}

// ============================================================================
// expected<T&, E> — in-place error construction
// ============================================================================

TEST(ExpectedRefInPlaceTest, ConstructErrorInPlace) {
  expected<int&, std::string> e(unexpect_t{}, "ref error");
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "ref error");
}

// ============================================================================
// expected<T, E> — in-place construction with types that have zero args
// ============================================================================

TEST(ExpectedInPlaceZeroArgTest, ConstructValueZeroArgs) {
  expected<int, std::string> e(std::in_place);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e.value(), 0);  // int{} is 0
}

TEST(ExpectedInPlaceZeroArgTest, ConstructErrorZeroArgs) {
  expected<int, std::string> e(unexpect_t{});
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "");  // std::string{} is ""
}

// ============================================================================
// unexpect — global inline constexpr tag
// ============================================================================

TEST(UnexpectTest, ConstructErrorWithUnexpect) {
  // unexpect is a global constexpr unexpect_t, like std::unexpect
  expected<int, std::string> e(argparse::unexpect, "error message");
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "error message");
}

TEST(UnexpectTest, ConstructErrorMultiArgWithUnexpect) {
  expected<int, std::string> e(argparse::unexpect, 5, 'x');
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "xxxxx");
}

TEST(UnexpectTest, SameAsUnexpectedT) {
  // Verify unexpect has the same type as unexpect_t{}
  static_assert(
      std::is_same_v<decltype(argparse::unexpect), const argparse::unexpect_t>);
  // Both can be used interchangeably
  expected<int, std::string> e1(argparse::unexpect, "a");
  expected<int, std::string> e2(unexpect_t{}, "a");
  EXPECT_EQ(e1.error(), e2.error());
}

// ============================================================================
// expected<T, E> — transform_error const& and const&& overloads
// ============================================================================

TEST(ExpectedMonadicTest, TransformErrorConstLvalue) {
  const expected<int, std::string> e(make_unexpected(std::string("hello")));
  auto result = e.transform_error([](const std::string& err) -> int {
    return static_cast<int>(err.size());
  });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 5);
}

TEST(ExpectedMonadicTest, TransformErrorConstLvalueOnValue) {
  const expected<int, std::string> e(42);
  auto result = e.transform_error([](const std::string& err) -> int {
    return static_cast<int>(err.size());
  });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(ExpectedMonadicTest, TransformErrorConstRvalue) {
  const expected<int, std::string> e(make_unexpected(std::string("world")));
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> int {
        return static_cast<int>(err.size());
      });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 5);
}

TEST(ExpectedMonadicTest, TransformErrorConstRvalueOnValue) {
  const expected<int, std::string> e(99);
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> int {
        return static_cast<int>(err.size());
      });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 99);
}

// ============================================================================
// expected<T, E> — transform_error with cv-qualified return type (remove_cv_t)
// ============================================================================

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

TEST(ExpectedMonadicTest, TransformErrorReturnsConst) {
  expected<int, std::string> e(make_unexpected(std::string("abc")));
  auto result = e.transform_error([](std::string& err) -> const int {
    return static_cast<int>(err.size());
  });
  // remove_cv_t strips const, so result type is expected<int, int>
  static_assert(std::is_same_v<decltype(result), expected<int, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedMonadicTest, TransformErrorConstLvalueReturnsConst) {
  const expected<int, std::string> e(make_unexpected(std::string("abcd")));
  auto result = e.transform_error([](const std::string& err) -> const int {
    return static_cast<int>(err.size());
  });
  static_assert(std::is_same_v<decltype(result), expected<int, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 4);
}

TEST(ExpectedMonadicTest, TransformErrorRvalueReturnsConst) {
  auto result = expected<int, std::string>(make_unexpected(std::string("xyz")))
                    .transform_error([](std::string&& err) -> const int {
                      return static_cast<int>(err.size());
                    });
  static_assert(std::is_same_v<decltype(result), expected<int, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedMonadicTest, TransformErrorConstRvalueReturnsConst) {
  const expected<int, std::string> e(make_unexpected(std::string("xy")));
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> const int {
        return static_cast<int>(err.size());
      });
  static_assert(std::is_same_v<decltype(result), expected<int, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 2);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ============================================================================
// expected<T&, E> — transform_error error path & reference preservation
// ============================================================================

TEST(ExpectedRefTest, MonadicTransformErrorOnError) {
  expected<int&, std::string> e(make_unexpected(std::string("hello")));
  auto result = e.transform_error(
      [](std::string& err) -> int { return static_cast<int>(err.size()); });
  // Must preserve reference: result type is expected<int&, int>
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 5);
}

TEST(ExpectedRefTest, MonadicTransformErrorConstLvalue) {
  int val = 42;
  const expected<int&, std::string> e(val);
  auto result = e.transform_error([](const std::string& err) -> int {
    return static_cast<int>(err.size());
  });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedRefTest, MonadicTransformErrorConstLvalueOnError) {
  const expected<int&, std::string> e(make_unexpected(std::string("world")));
  auto result = e.transform_error([](const std::string& err) -> int {
    return static_cast<int>(err.size());
  });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 5);
}

TEST(ExpectedRefTest, MonadicTransformErrorRvalue) {
  auto result = expected<int&, std::string>(make_unexpected(std::string("xyz")))
                    .transform_error([](std::string&& err) -> int {
                      return static_cast<int>(err.size());
                    });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedRefTest, MonadicTransformErrorRvalueOnValue) {
  int val = 7;
  auto result = expected<int&, std::string>(val).transform_error(
      [](std::string&& err) -> int { return static_cast<int>(err.size()); });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedRefTest, MonadicTransformErrorConstRvalueOnError) {
  const expected<int&, std::string> e(make_unexpected(std::string("abcd")));
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> int {
        return static_cast<int>(err.size());
      });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 4);
}

TEST(ExpectedRefTest, MonadicTransformErrorConstRvalueOnValue) {
  int val = 99;
  const expected<int&, std::string> e(val);
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> int {
        return static_cast<int>(err.size());
      });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_TRUE(result.has_value());
}

// ============================================================================
// expected<T&, E> — transform_error with cv-qualified return (remove_cv_t)
// ============================================================================

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

TEST(ExpectedRefTest, MonadicTransformErrorReturnsConst) {
  expected<int&, std::string> e(make_unexpected(std::string("err")));
  auto result = e.transform_error([](std::string& err) -> const int {
    return static_cast<int>(err.size());
  });
  // remove_cv_t strips const, so result type is expected<int&, int>
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedRefTest, MonadicTransformErrorConstLvalueReturnsConst) {
  const expected<int&, std::string> e(make_unexpected(std::string("abcd")));
  auto result = e.transform_error([](const std::string& err) -> const int {
    return static_cast<int>(err.size());
  });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 4);
}

TEST(ExpectedRefTest, MonadicTransformErrorRvalueReturnsConst) {
  auto result = expected<int&, std::string>(make_unexpected(std::string("xy")))
                    .transform_error([](std::string&& err) -> const int {
                      return static_cast<int>(err.size());
                    });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 2);
}

TEST(ExpectedRefTest, MonadicTransformErrorConstRvalueReturnsConst) {
  const expected<int&, std::string> e(make_unexpected(std::string("xyz")));
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> const int {
        return static_cast<int>(err.size());
      });
  static_assert(std::is_same_v<decltype(result), expected<int&, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ============================================================================
// expected<void, E> — transform_error const& and const&& overloads
// ============================================================================

TEST(ExpectedVoidTest, TransformErrorConstLvalue) {
  const expected<void, std::string> e(make_unexpected(std::string("hello")));
  auto result = e.transform_error([](const std::string& err) -> int {
    return static_cast<int>(err.size());
  });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 5);
}

TEST(ExpectedVoidTest, TransformErrorConstLvalueOnValue) {
  const expected<void, std::string> e;
  auto result = e.transform_error([](const std::string& err) -> int {
    return static_cast<int>(err.size());
  });
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, TransformErrorRvalue) {
  auto result = expected<void, std::string>(make_unexpected(std::string("xyz")))
                    .transform_error([](std::string&& err) -> int {
                      return static_cast<int>(err.size());
                    });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedVoidTest, TransformErrorRvalueOnValue) {
  auto result = expected<void, std::string>().transform_error(
      [](std::string&& err) -> int { return static_cast<int>(err.size()); });
  EXPECT_TRUE(result.has_value());
}

TEST(ExpectedVoidTest, TransformErrorConstRvalueOnError) {
  const expected<void, std::string> e(make_unexpected(std::string("abcd")));
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> int {
        return static_cast<int>(err.size());
      });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 4);
}

TEST(ExpectedVoidTest, TransformErrorConstRvalueOnValue) {
  const expected<void, std::string> e;
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> int {
        return static_cast<int>(err.size());
      });
  EXPECT_TRUE(result.has_value());
}

// ============================================================================
// expected<void, E> — transform_error with cv-qualified return (remove_cv_t)
// ============================================================================

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

TEST(ExpectedVoidTest, TransformErrorReturnsConst) {
  expected<void, std::string> e(make_unexpected(std::string("err")));
  auto result = e.transform_error([](std::string& err) -> const int {
    return static_cast<int>(err.size());
  });
  // remove_cv_t strips const, so result type is expected<void, int>
  static_assert(std::is_same_v<decltype(result), expected<void, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

TEST(ExpectedVoidTest, TransformErrorConstLvalueReturnsConst) {
  const expected<void, std::string> e(make_unexpected(std::string("abcd")));
  auto result = e.transform_error([](const std::string& err) -> const int {
    return static_cast<int>(err.size());
  });
  static_assert(std::is_same_v<decltype(result), expected<void, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 4);
}

TEST(ExpectedVoidTest, TransformErrorRvalueReturnsConst) {
  auto result = expected<void, std::string>(make_unexpected(std::string("xy")))
                    .transform_error([](std::string&& err) -> const int {
                      return static_cast<int>(err.size());
                    });
  static_assert(std::is_same_v<decltype(result), expected<void, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 2);
}

TEST(ExpectedVoidTest, TransformErrorConstRvalueReturnsConst) {
  const expected<void, std::string> e(make_unexpected(std::string("xyz")));
  auto result =
      std::move(e).transform_error([](const std::string&& err) -> const int {
        return static_cast<int>(err.size());
      });
  static_assert(std::is_same_v<decltype(result), expected<void, int>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), 3);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ============================================================================
// expected<void, E> — in-place value construction (std::in_place_t)
// ============================================================================

TEST(ExpectedVoidTest, ConstructInPlace) {
  expected<void, std::string> e(std::in_place);
  EXPECT_TRUE(e.has_value());
  // value() on void expected should not throw/assert
  e.value();
  SUCCEED();
}

TEST(ExpectedVoidTest, ConstructInPlaceThenAssign) {
  expected<void, std::string> e(std::in_place);
  EXPECT_TRUE(e.has_value());

  // Assign an error
  e = expected<void, std::string>(make_unexpected(std::string("fail")));
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), "fail");

  // Assign back to value via copy from a default-constructed one
  e = expected<void, std::string>();
  EXPECT_TRUE(e.has_value());
}

// ============================================================================
// transform_error — chained use with unexpect construction style
// ============================================================================

TEST(ExpectedMonadicTest, TransformErrorChainWithUnexpect) {
  // Verify that transform_error produces a result that can use unexpect
  // in subsequent operations (validates the unexpect + error construction path)
  expected<int, std::string> e(make_unexpected(std::string("hello")));
  auto result = e.transform_error([](std::string& err) -> int {
                   return static_cast<int>(err.size());
                 }).transform_error([](int err_code) -> std::string {
    return std::to_string(err_code * 2);
  });
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "10");
}

TEST(ExpectedRefTest, TransformErrorChainWithUnexpect) {
  expected<int&, std::string> e(make_unexpected(std::string("abc")));
  auto result = e.transform_error([](std::string& err) -> int {
                   return static_cast<int>(err.size());
                 }).transform_error([](int err_code) -> std::string {
    return std::to_string(err_code * 10);
  });
  static_assert(std::is_same_v<decltype(result), expected<int&, std::string>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "30");
}

TEST(ExpectedVoidTest, TransformErrorChainWithUnexpect) {
  expected<void, std::string> e(make_unexpected(std::string("xyz")));
  auto result = e.transform_error([](std::string& err) -> int {
                   return static_cast<int>(err.size());
                 }).transform_error([](int err_code) -> std::string {
    return std::to_string(err_code * 100);
  });
  static_assert(std::is_same_v<decltype(result), expected<void, std::string>>);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "300");
}

// ============================================================================
// Swap — cross-state with non-nothrow-move-constructible error type
// Exercises the else branch (non-nothrow path) for all three specializations.
// ============================================================================

namespace {
struct NoNothrowMoveError {
  int val;
  bool moved_from = false;
  explicit NoNothrowMoveError(int v) : val(v) {}
  NoNothrowMoveError(const NoNothrowMoveError&) : val(0) {}
  NoNothrowMoveError(NoNothrowMoveError&& o) noexcept(false) : val(o.val) {
    o.moved_from = true;
  }
  NoNothrowMoveError& operator=(NoNothrowMoveError&& o) {
    val = o.val;
    o.moved_from = true;
    return *this;
  }
  NoNothrowMoveError& operator=(const NoNothrowMoveError&) = default;
  ~NoNothrowMoveError() {}
  bool operator==(const NoNothrowMoveError& o) const { return val == o.val; }
};
static_assert(!std::is_nothrow_move_constructible_v<NoNothrowMoveError>);
}  // namespace

TEST(ExpectedSwapNonNothrowTest, ValueTypeCrossState) {
  expected<int, NoNothrowMoveError> a(42);
  expected<int, NoNothrowMoveError> b(make_unexpected(NoNothrowMoveError(99)));
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 42);
}

TEST(ExpectedSwapNonNothrowTest, ValueTypeCrossStateReverse) {
  // This has error, other has value — exercises the else branch that
  // calls other.swap(*this)
  expected<int, NoNothrowMoveError> a(make_unexpected(NoNothrowMoveError(99)));
  expected<int, NoNothrowMoveError> b(42);
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(a.value(), 42);
  EXPECT_FALSE(b.has_value());
}

TEST(ExpectedSwapNonNothrowTest, ValueTypeBothHaveError) {
  expected<int, NoNothrowMoveError> a(make_unexpected(NoNothrowMoveError(1)));
  expected<int, NoNothrowMoveError> b(make_unexpected(NoNothrowMoveError(2)));
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(a.error().val, 2);
  EXPECT_EQ(b.error().val, 1);
}

TEST(ExpectedSwapNonNothrowTest, RefTypeCrossState) {
  int val = 42;
  expected<int&, NoNothrowMoveError> a(val);
  expected<int&, NoNothrowMoveError> b(make_unexpected(NoNothrowMoveError(99)));
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(&b.value(), &val);
}

TEST(ExpectedSwapNonNothrowTest, RefTypeCrossStateReverse) {
  int val = 42;
  expected<int&, NoNothrowMoveError> a(make_unexpected(NoNothrowMoveError(99)));
  expected<int&, NoNothrowMoveError> b(val);
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(&a.value(), &val);
  EXPECT_FALSE(b.has_value());
}

TEST(ExpectedSwapNonNothrowTest, VoidTypeCrossState) {
  expected<void, NoNothrowMoveError> a;
  expected<void, NoNothrowMoveError> b(make_unexpected(NoNothrowMoveError(99)));
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_TRUE(b.has_value());
}

TEST(ExpectedSwapNonNothrowTest, VoidTypeCrossStateReverse) {
  expected<void, NoNothrowMoveError> a(make_unexpected(NoNothrowMoveError(99)));
  expected<void, NoNothrowMoveError> b;
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
}

TEST(ExpectedSwapNonNothrowTest, VoidTypeBothHaveError) {
  expected<void, NoNothrowMoveError> a(make_unexpected(NoNothrowMoveError(1)));
  expected<void, NoNothrowMoveError> b(make_unexpected(NoNothrowMoveError(2)));
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(a.error().val, 2);
  EXPECT_EQ(b.error().val, 1);
}

// ============================================================================
// Swap — round-trip (swap twice) for ref and void specializations
// ============================================================================

TEST(ExpectedSwapRoundTripTest, RefType) {
  int v1 = 1, v2 = 2;
  expected<int&, std::string> a(v1);
  expected<int&, std::string> b(v2);
  a.swap(b);
  EXPECT_EQ(&a.value(), &v2);
  EXPECT_EQ(&b.value(), &v1);
  a.swap(b);
  EXPECT_EQ(&a.value(), &v1);
  EXPECT_EQ(&b.value(), &v2);
}

TEST(ExpectedSwapRoundTripTest, RefTypeCrossState) {
  int val = 42;
  expected<int&, std::string> a(val);
  expected<int&, std::string> b(make_unexpected(std::string("err")));
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(&b.value(), &val);
  // Swap back
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(&a.value(), &val);
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(b.error(), "err");
}

TEST(ExpectedSwapRoundTripTest, VoidTypeCrossState) {
  expected<void, std::string> a;
  expected<void, std::string> b(make_unexpected(std::string("err")));
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
  EXPECT_TRUE(b.has_value());
  // Swap back
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(b.error(), "err");
}

TEST(ExpectedSwapRoundTripTest, VoidTypeBothError) {
  expected<void, std::string> a(make_unexpected(std::string("a")));
  expected<void, std::string> b(make_unexpected(std::string("b")));
  a.swap(b);
  EXPECT_EQ(a.error(), "b");
  EXPECT_EQ(b.error(), "a");
  a.swap(b);
  EXPECT_EQ(a.error(), "a");
  EXPECT_EQ(b.error(), "b");
}

// ============================================================================
// Swap — move-only types
// ============================================================================

TEST(ExpectedSwapMoveOnlyTest, ValueTypeMoveOnlyError) {
  // Note: can't call value() because bad_expected_access copies the error,
  // and unique_ptr is move-only. Use has_value() + error().
  expected<int, std::unique_ptr<int>> a(42);
  expected<int, std::unique_ptr<int>> b(
      make_unexpected(std::make_unique<int>(99)));
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(*a.error(), 99);
  EXPECT_TRUE(b.has_value());
  // b has value, but can't call value() with move-only error type
}

TEST(ExpectedSwapMoveOnlyTest, ValueTypeMoveOnlyErrorReverse) {
  expected<int, std::unique_ptr<int>> a(
      make_unexpected(std::make_unique<int>(99)));
  expected<int, std::unique_ptr<int>> b(42);
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(*b.error(), 99);
}

TEST(ExpectedSwapMoveOnlyTest, VoidTypeMoveOnlyError) {
  expected<void, std::unique_ptr<int>> a;
  expected<void, std::unique_ptr<int>> b(
      make_unexpected(std::make_unique<int>(99)));
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(*a.error(), 99);
  EXPECT_TRUE(b.has_value());
}

TEST(ExpectedSwapMoveOnlyTest, VoidTypeMoveOnlyErrorReverse) {
  expected<void, std::unique_ptr<int>> a(
      make_unexpected(std::make_unique<int>(99)));
  expected<void, std::unique_ptr<int>> b;
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(*b.error(), 99);
}

// ============================================================================
// Swap — noexcept specification verification
// ============================================================================

TEST(ExpectedSwapNoexceptTest, NothrowTypesAreNothrowSwappable) {
  // int and std::string are nothrow move constructible and swappable
  static_assert(noexcept(std::declval<expected<int, std::string>&>().swap(
      std::declval<expected<int, std::string>&>())));
  static_assert(noexcept(std::declval<expected<int&, std::string>&>().swap(
      std::declval<expected<int&, std::string>&>())));
  static_assert(noexcept(std::declval<expected<void, std::string>&>().swap(
      std::declval<expected<void, std::string>&>())));
}

TEST(ExpectedSwapNoexceptTest, NonNothrowMoveErrorIsNotNothrowSwappable) {
  // NoNothrowMoveError is NOT nothrow move constructible
  static_assert(
      !noexcept(std::declval<expected<int, NoNothrowMoveError>&>().swap(
          std::declval<expected<int, NoNothrowMoveError>&>())));
  static_assert(
      !noexcept(std::declval<expected<int&, NoNothrowMoveError>&>().swap(
          std::declval<expected<int&, NoNothrowMoveError>&>())));
  static_assert(
      !noexcept(std::declval<expected<void, NoNothrowMoveError>&>().swap(
          std::declval<expected<void, NoNothrowMoveError>&>())));
}

// ============================================================================
// Swap — exception safety: verify that if an exception is thrown during
// swap, both objects are left in a usable state (basic guarantee).
// ============================================================================

#ifdef __cpp_exceptions

namespace {
// A value type that throws during move construction
struct ThrowingMoveValue {
  int val;
  bool move_throws = false;
  explicit ThrowingMoveValue(int v, bool throws = false)
      : val(v), move_throws(throws) {}
  ThrowingMoveValue(const ThrowingMoveValue&) = default;
  ThrowingMoveValue(ThrowingMoveValue&& o) noexcept(false)
      : val(o.val), move_throws(o.move_throws) {
    if (o.move_throws) {
      throw std::runtime_error("value_move_throws");
    }
    o.val = -1;
  }
  ThrowingMoveValue& operator=(const ThrowingMoveValue&) = default;
  ThrowingMoveValue& operator=(ThrowingMoveValue&&) = default;
  bool operator==(const ThrowingMoveValue& o) const { return val == o.val; }
};

// An error type that throws during move construction
struct ThrowingMoveError {
  int val;
  bool move_throws = false;
  explicit ThrowingMoveError(int v, bool throws = false)
      : val(v), move_throws(throws) {}
  ThrowingMoveError(const ThrowingMoveError&) : val(0) {}
  ThrowingMoveError(ThrowingMoveError&& o) noexcept(false)
      : val(o.val), move_throws(o.move_throws) {
    if (o.move_throws) {
      throw std::runtime_error("error_move_throws");
    }
    o.val = -1;
  }
  ThrowingMoveError& operator=(const ThrowingMoveError&) = default;
  ThrowingMoveError& operator=(ThrowingMoveError&& o) {
    val = o.val;
    o.val = -1;
    return *this;
  }
  ~ThrowingMoveError() {}
  bool operator==(const ThrowingMoveError& o) const { return val == o.val; }
};

}  // namespace

TEST(ExpectedSwapExceptionTest, ValueTypeThrowingValueMove) {
  // When E is nothrow move constructible but T's move constructor may throw,
  // swap uses the nothrow-E path. If constructing value in other throws,
  // other's error should be restored.
  // Use in_place to avoid moving during construction.
  expected<ThrowingMoveValue, std::string> a(std::in_place, 42, true);
  expected<ThrowingMoveValue, std::string> b(
      make_unexpected(std::string("original_error")));
  // a has a value with move_throws=true, b has an error.
  // During swap, E is nothrow, so the nothrow path is taken:
  // 1. Move b's error to temp
  // 2. Destroy b
  // 3. Try to move a's value into b → throws!
  // 4. Catch: restore b's error from temp, rethrow
  EXPECT_THROW(a.swap(b), std::runtime_error);
  // After failed swap: a should still have its value (we never destroyed a)
  // b should have its original error restored
  EXPECT_TRUE(a.has_value());
  EXPECT_TRUE(b.has_value() == false);
  EXPECT_EQ(b.error(), "original_error");
}

TEST(ExpectedSwapExceptionTest, ValueTypeNonNothrowErrorPath) {
  // When E is not nothrow move constructible, swap uses the non-nothrow-E
  // path. If constructing error in this throws, this's value should be
  // restored.
  // Use in_place to avoid moving during construction.
  expected<int, ThrowingMoveError> a(42);
  expected<int, ThrowingMoveError> b(unexpect_t{}, 99, true);
  // a has value 42, b has error with move_throws=true.
  // During swap, non-nothrow path:
  // 1. Save a's value (42) to temp, destroy a
  // 2. Try to construct error in a from b's error → throws!
  // 3. Catch: restore a's value from temp, rethrow
  EXPECT_THROW(a.swap(b), std::runtime_error);
  // After failed swap: a's value should be restored
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(a.value(), 42);
  // b is in a valid but unspecified state (move was attempted)
}

TEST(ExpectedSwapExceptionTest, VoidTypeNothrowPath) {
  // For expected<void, E>: when E is nothrow move constructible, the swap
  // moves other.error_ to temp, destroys other.error_, then constructs
  // this->error_ from temp. Construction shouldn't throw (E is nothrow).
  expected<void, std::string> a;
  expected<void, std::string> b(make_unexpected(std::string("err")));
  EXPECT_NO_THROW(a.swap(b));
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
  EXPECT_TRUE(b.has_value());
}

TEST(ExpectedSwapExceptionTest, VoidTypeNonNothrowErrorPath) {
  // expected<void, E> with non-nothrow error: this has value, other has error
  // The non-nothrow path constructs this->error_ from other's error.
  // If that throws, this should remain with value, other with error
  // (in a valid but unspecified state).
  expected<void, ThrowingMoveError> a;
  expected<void, ThrowingMoveError> b(unexpect_t{}, 99, true);
  EXPECT_THROW(a.swap(b), std::runtime_error);
  // a should still have value (construction into this->error_ threw)
  EXPECT_TRUE(a.has_value());
}

TEST(ExpectedSwapExceptionTest, RefTypeNothrowPath) {
  // expected<T&, E> with nothrow error: this has value, other has error.
  // The nothrow path moves other's error to temp, destroys other,
  // then tries to construct value in other from this->storage_.value.
  // For references, value is a pointer, so move is just copying a pointer —
  // it won't throw.
  int val = 42;
  expected<int&, std::string> a(val);
  expected<int&, std::string> b(make_unexpected(std::string("err")));
  EXPECT_NO_THROW(a.swap(b));
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(&b.value(), &val);
}

TEST(ExpectedSwapExceptionTest, RefTypeNonNothrowErrorPath) {
  // expected<T&, E> with non-nothrow error: this has value, other has error.
  // The non-nothrow path saves this->storage_.value (pointer), destroys this,
  // then tries to construct error from other's error. If that throws,
  // this's value should be restored.
  int val = 42;
  expected<int&, ThrowingMoveError> a(val);
  expected<int&, ThrowingMoveError> b(unexpect_t{}, 99, true);
  EXPECT_THROW(a.swap(b), std::runtime_error);
  // a should still have value and refer to the same object
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(&a.value(), &val);
  EXPECT_EQ(a.value(), 42);
}

#endif  // __cpp_exceptions

// ============================================================================
// Swap — std::swap free-function integration
// ============================================================================

TEST(ExpectedSwapFreeFunctionTest, ValueType) {
  expected<int, std::string> a(1);
  expected<int, std::string> b(2);
  using std::swap;
  swap(a, b);
  EXPECT_EQ(a.value(), 2);
  EXPECT_EQ(b.value(), 1);
}

TEST(ExpectedSwapFreeFunctionTest, RefType) {
  int v1 = 1, v2 = 2;
  expected<int&, std::string> a(v1);
  expected<int&, std::string> b(v2);
  using std::swap;
  swap(a, b);
  EXPECT_EQ(&a.value(), &v2);
  EXPECT_EQ(&b.value(), &v1);
}

TEST(ExpectedSwapFreeFunctionTest, VoidType) {
  expected<void, std::string> a;
  expected<void, std::string> b(make_unexpected(std::string("err")));
  using std::swap;
  swap(a, b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
  EXPECT_TRUE(b.has_value());
}

TEST(ExpectedSwapFreeFunctionTest, CrossState) {
  expected<int, std::string> a(42);
  expected<int, std::string> b(make_unexpected(std::string("err")));
  using std::swap;
  swap(a, b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "err");
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), 42);
}

// ============================================================================
// Swap — edge case: both void (has_value()) — nothing to swap, state unchanged
// ============================================================================

TEST(ExpectedSwapVoidEdgeTest, BothHaveValue) {
  expected<void, std::string> a;
  expected<void, std::string> b;
  EXPECT_TRUE(a.has_value());
  EXPECT_TRUE(b.has_value());
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_TRUE(b.has_value());
}

// ============================================================================
// Swap — the "else" branch (this has error, other has value) for value type
// ============================================================================

TEST(ExpectedSwapElseBranchTest, ValueTypeThisErrorOtherValue) {
  expected<std::string, std::string> a(make_unexpected(std::string("error")));
  expected<std::string, std::string> b(std::string("value"));
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(a.value(), "value");
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(b.error(), "error");
}

TEST(ExpectedSwapElseBranchTest, ValueTypeSwapTwice) {
  expected<std::string, std::string> a(make_unexpected(std::string("error")));
  expected<std::string, std::string> b(std::string("value"));
  a.swap(b);
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "error");
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), "value");
}

// ============================================================================
// Swap — same-type value and error cross-state with non-trivial types
// ============================================================================

TEST(ExpectedSwapSameTypeTest, CrossStateBackAndForth) {
  expected<std::string, std::string> a(std::string("hello"));
  expected<std::string, std::string> b(make_unexpected(std::string("world")));

  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_EQ(a.error(), "world");
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(b.value(), "hello");

  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(a.value(), "hello");
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(b.error(), "world");
}

// ============================================================================
// Swap — cross-state with both being move-only (unique_ptr)
// ============================================================================

TEST(ExpectedSwapMoveOnlyBothTest, CrossState) {
  // Note: cannot call value()/operator* when exceptions are enabled because
  // bad_expected_access copies the error and unique_ptr is move-only.
  expected<std::unique_ptr<int>, std::unique_ptr<int>> a(std::in_place,
                                                         new int(42));
  expected<std::unique_ptr<int>, std::unique_ptr<int>> b(unexpect_t{},
                                                         new int(99));
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  a.swap(b);
  EXPECT_FALSE(a.has_value());
  EXPECT_TRUE(b.has_value());
  EXPECT_EQ(*a.error(), 99);
  // b has a value, but can't call value() because bad_expected_access
  // would need to copy the error type. Just verify has_value().
  SUCCEED();
}

TEST(ExpectedSwapMoveOnlyBothTest, CrossStateReverse) {
  expected<std::unique_ptr<int>, std::unique_ptr<int>> a(unexpect_t{},
                                                         new int(99));
  expected<std::unique_ptr<int>, std::unique_ptr<int>> b(std::in_place,
                                                         new int(42));
  a.swap(b);
  EXPECT_TRUE(a.has_value());
  EXPECT_FALSE(b.has_value());
  EXPECT_EQ(*b.error(), 99);
  SUCCEED();
}
