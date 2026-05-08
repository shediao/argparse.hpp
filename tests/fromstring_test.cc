//
// Created by shediao on 24-12-29.
//

#include <gtest/gtest.h>

#include <array>
#include <limits>

#include "argparse/argparse.hpp"

using argparse::detail::from_string;
using argparse::detail::from_wstring;
using argparse::detail::parse_from_string;
using argparse::detail::split;

// Test bool
TEST(ParseFromStringTest, BoolTest) {
  EXPECT_EQ(parse_from_string<bool>("true"), true);
  EXPECT_EQ(parse_from_string<bool>("false"), false);
  EXPECT_EQ(parse_from_string<bool>("1"), true);
  EXPECT_EQ(parse_from_string<bool>("0"), false);
  EXPECT_EQ(parse_from_string<bool>("on"), true);
  EXPECT_EQ(parse_from_string<bool>("off"), false);
}

// Test int
TEST(ParseFromStringTest, IntTest) {
  EXPECT_EQ(parse_from_string<int>("123"), 123);
  EXPECT_EQ(parse_from_string<int>("-456"), -456);
  EXPECT_THROW(parse_from_string<int>("12.34"), std::invalid_argument);
  EXPECT_THROW(parse_from_string<int>("abc"), std::invalid_argument);
}

// Test long
TEST(ParseFromStringTest, LongTest) {
  EXPECT_EQ(parse_from_string<long>("1234567890"), 1234567890L);
  if constexpr (sizeof(long) > 4) {
    // 32 bit long: max 2,147,483,647, min -2,147,483,648
    EXPECT_EQ(parse_from_string<long>("-9876543210"), -9876543210L);
  }
  EXPECT_EQ(parse_from_string<long>("-2147483648"), -2147483648L);
  EXPECT_THROW(parse_from_string<long>("12.34"), std::invalid_argument);
  EXPECT_THROW(parse_from_string<long>("abc"), std::invalid_argument);
}

// Test double
TEST(ParseFromStringTest, DoubleTest) {
  EXPECT_DOUBLE_EQ(parse_from_string<double>("3.14"), 3.14);
  EXPECT_DOUBLE_EQ(parse_from_string<double>("-2.71"), -2.71);
  EXPECT_THROW(parse_from_string<double>("abc"), std::invalid_argument);
}

// Test long long
TEST(ParseFromStringTest, LongLongTest) {
  EXPECT_EQ(parse_from_string<long long>("123456789012345"), 123456789012345LL);
  EXPECT_EQ(parse_from_string<long long>("-987654321098765"),
            -987654321098765LL);
  EXPECT_THROW(parse_from_string<long long>("12.34"), std::invalid_argument);
  EXPECT_THROW(parse_from_string<long long>("abc"), std::invalid_argument);
}

TEST(ParseFromStringTest, split) {
  EXPECT_EQ(split("k=v", '=', 1), (std::vector<std::string>{"k", "v"}));
  EXPECT_EQ(split("1,2,3", ',', 1), (std::vector<std::string>{"1", "2,3"}));
  EXPECT_EQ(split("1,2,3", ',', 2), (std::vector<std::string>{"1", "2", "3"}));
  EXPECT_EQ(split("1,2,3", ',', 3), (std::vector<std::string>{"1", "2", "3"}));
  EXPECT_EQ(split("1,", ',', -1), (std::vector<std::string>{"1", ""}));
  EXPECT_EQ(split(",1", ',', -1), (std::vector<std::string>{"", "1"}));
}

TEST(ParseFromStringTest, Sizeable) {
  EXPECT_EQ((parse_from_string<std::pair<int, int>>("1,2", ',')),
            (std::pair<int, int>{1, 2}));
  EXPECT_EQ((parse_from_string<std::tuple<int, int, int>>("1,2,3", ',')),
            (std::tuple{1, 2, 3}));

  EXPECT_EQ(
      (parse_from_string<std::pair<std::string, std::string>>("k=v", '=')),
      (std::pair<std::string, std::string>{"k", "v"}));
  EXPECT_EQ((parse_from_string<std::pair<std::string, std::string>>("k=", '=')),
            (std::pair<std::string, std::string>{"k", ""}));
  EXPECT_EQ(
      (parse_from_string<std::pair<std::string, std::string>>("k=v=vv", '=')),
      (std::pair<std::string, std::string>{"k", "v=vv"}));

  EXPECT_THROW(
      (parse_from_string<std::pair<std::string, std::string>>("k", '=')),
      std::invalid_argument);
}

TEST(ParseFromStringTest, Container) {
  EXPECT_EQ((parse_from_string<std::tuple<int, int, int>>("1,2,3", ',')),
            (std::tuple<int, int, int>{1, 2, 3}));
  EXPECT_EQ((parse_from_string<std::array<int, 3>>("1,2,3", ',')),
            (std::array<int, 3>{1, 2, 3}));
}

// ============================================================
// Tests for the new from_string / from_wstring (C++20 concepts)
// ============================================================

// --- from_string: types constructible from std::string ---

TEST(FromStringTest, ConstructibleFromString) {
  // std::string_view is constructible from std::string
  std::string hello("hello world");
  auto sv = from_string<std::string_view>(hello);
  EXPECT_EQ(sv, "hello world");

  // std::string itself
  auto s = from_string<std::string>("test string");
  EXPECT_EQ(s, "test string");
}

// --- from_string: integral types ---

TEST(FromStringTest, Int) {
  EXPECT_EQ(from_string<int>("0"), 0);
  EXPECT_EQ(from_string<int>("123"), 123);
  EXPECT_EQ(from_string<int>("-456"), -456);
  EXPECT_EQ(from_string<int>("2147483647"), 2147483647);
  EXPECT_EQ(from_string<int>("-2147483648"), -2147483648);
  EXPECT_THROW(from_string<int>("12.34"), std::invalid_argument);
  EXPECT_THROW(from_string<int>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<int>(""), std::invalid_argument);
}

TEST(FromStringTest, Long) {
  EXPECT_EQ(from_string<long>("0"), 0L);
  EXPECT_EQ(from_string<long>("1234567890"), 1234567890L);
  EXPECT_EQ(from_string<long>("-987654321"), -987654321L);
  EXPECT_THROW(from_string<long>("12.34"), std::invalid_argument);
  EXPECT_THROW(from_string<long>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<long>(""), std::invalid_argument);
}

TEST(FromStringTest, UnsignedLong) {
  EXPECT_EQ(from_string<unsigned long>("0"), 0UL);
  EXPECT_EQ(from_string<unsigned long>("4294967295"), 4294967295UL);
  // std::stoul wraps negative values (standard behavior)
  EXPECT_EQ(from_string<unsigned long>("-1"),
            (std::numeric_limits<unsigned long>::max)());
  EXPECT_THROW(from_string<unsigned long>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<unsigned long>(""), std::invalid_argument);
}

TEST(FromStringTest, LongLong) {
  EXPECT_EQ(from_string<long long>("0"), 0LL);
  EXPECT_EQ(from_string<long long>("123456789012345"), 123456789012345LL);
  EXPECT_EQ(from_string<long long>("-987654321098765"), -987654321098765LL);
  EXPECT_EQ(from_string<long long>("9223372036854775807"),
            9223372036854775807LL);
  EXPECT_THROW(from_string<long long>("12.34"), std::invalid_argument);
  EXPECT_THROW(from_string<long long>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<long long>(""), std::invalid_argument);
}

TEST(FromStringTest, UnsignedLongLong) {
  EXPECT_EQ(from_string<unsigned long long>("0"), 0ULL);
  EXPECT_EQ(from_string<unsigned long long>("18446744073709551615"),
            18446744073709551615ULL);
  // std::stoull wraps negative values (standard behavior)
  EXPECT_EQ(from_string<unsigned long long>("-1"),
            (std::numeric_limits<unsigned long long>::max)());
  EXPECT_THROW(from_string<unsigned long long>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<unsigned long long>(""), std::invalid_argument);
}

// --- from_string: floating-point types ---

TEST(FromStringTest, Float) {
  EXPECT_FLOAT_EQ(from_string<float>("0"), 0.0f);
  EXPECT_FLOAT_EQ(from_string<float>("3.14"), 3.14f);
  EXPECT_FLOAT_EQ(from_string<float>("-2.71"), -2.71f);
  EXPECT_FLOAT_EQ(from_string<float>("1.5e2"), 150.0f);
  EXPECT_THROW(from_string<float>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<float>(""), std::invalid_argument);
}

TEST(FromStringTest, Double) {
  EXPECT_DOUBLE_EQ(from_string<double>("0"), 0.0);
  EXPECT_DOUBLE_EQ(from_string<double>("3.14159265358979"), 3.14159265358979);
  EXPECT_DOUBLE_EQ(from_string<double>("-2.71828182845905"), -2.71828182845905);
  EXPECT_DOUBLE_EQ(from_string<double>("1e-10"), 1e-10);
  EXPECT_THROW(from_string<double>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<double>(""), std::invalid_argument);
}

TEST(FromStringTest, LongDouble) {
  EXPECT_DOUBLE_EQ(static_cast<double>(from_string<long double>("0")), 0.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(from_string<long double>("3.14")), 3.14);
  EXPECT_DOUBLE_EQ(static_cast<double>(from_string<long double>("-2.71")),
                   -2.71);
  EXPECT_THROW(from_string<long double>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<long double>(""), std::invalid_argument);
}

// --- from_string: additional integral types ---

TEST(FromStringTest, UnsignedInt) {
  EXPECT_EQ(from_string<unsigned int>("0"), 0U);
  EXPECT_EQ(from_string<unsigned int>("123"), 123U);
  EXPECT_EQ(from_string<unsigned int>("4294967295"), 4294967295U);
  EXPECT_THROW(from_string<unsigned int>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<unsigned int>(""), std::invalid_argument);
  EXPECT_THROW(from_string<unsigned int>("4294967296"), std::invalid_argument);
}

TEST(FromStringTest, Short) {
  EXPECT_EQ(from_string<short>("0"), 0);
  EXPECT_EQ(from_string<short>("123"), 123);
  EXPECT_EQ(from_string<short>("-456"), -456);
  EXPECT_EQ(from_string<short>("32767"), 32767);
  EXPECT_EQ(from_string<short>("-32768"), -32768);
  EXPECT_THROW(from_string<short>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<short>(""), std::invalid_argument);
  EXPECT_THROW(from_string<short>("32768"), std::invalid_argument);
  EXPECT_THROW(from_string<short>("-32769"), std::invalid_argument);
}

TEST(FromStringTest, UnsignedShort) {
  EXPECT_EQ(from_string<unsigned short>("0"), static_cast<unsigned short>(0));
  EXPECT_EQ(from_string<unsigned short>("123"),
            static_cast<unsigned short>(123));
  EXPECT_EQ(from_string<unsigned short>("65535"),
            static_cast<unsigned short>(65535));
  EXPECT_THROW(from_string<unsigned short>("abc"), std::invalid_argument);
  EXPECT_THROW(from_string<unsigned short>(""), std::invalid_argument);
  EXPECT_THROW(from_string<unsigned short>("65536"), std::invalid_argument);
}

// --- from_string: tuple-like types (pair, tuple, array) ---

TEST(FromStringTest, Pair) {
  auto p = from_string<std::pair<int, int>>("1,2", ',');
  EXPECT_EQ(p.first, 1);
  EXPECT_EQ(p.second, 2);

  auto ps =
      from_string<std::pair<std::string, std::string>>("hello,world", ',');
  EXPECT_EQ(ps.first, "hello");
  EXPECT_EQ(ps.second, "world");

  auto pm = from_string<std::pair<int, double>>("42,3.14", ',');
  EXPECT_EQ(pm.first, 42);
  EXPECT_DOUBLE_EQ(pm.second, 3.14);
}

TEST(FromStringTest, TupleBasic) {
  auto t2 = from_string<std::tuple<int, int>>("10,20", ',');
  EXPECT_EQ(std::get<0>(t2), 10);
  EXPECT_EQ(std::get<1>(t2), 20);

  auto t3 = from_string<std::tuple<int, int, int>>("1,2,3", ',');
  EXPECT_EQ(std::get<0>(t3), 1);
  EXPECT_EQ(std::get<1>(t3), 2);
  EXPECT_EQ(std::get<2>(t3), 3);
}

TEST(FromStringTest, TupleSingleElement) {
  auto t1 = from_string<std::tuple<int>>("42", ',');
  EXPECT_EQ(std::get<0>(t1), 42);

  auto ts1 = from_string<std::tuple<std::string>>("hello", ',');
  EXPECT_EQ(std::get<0>(ts1), "hello");
}

TEST(FromStringTest, TupleMixedTypes) {
  auto t =
      from_string<std::tuple<int, double, std::string>>("1,3.14,hello", ',');
  EXPECT_EQ(std::get<0>(t), 1);
  EXPECT_DOUBLE_EQ(std::get<1>(t), 3.14);
  EXPECT_EQ(std::get<2>(t), "hello");
}

TEST(FromStringTest, TupleCustomDelimiter) {
  auto t = from_string<std::tuple<int, int, int>>("1:2:3", ':');
  EXPECT_EQ(std::get<0>(t), 1);
  EXPECT_EQ(std::get<1>(t), 2);
  EXPECT_EQ(std::get<2>(t), 3);

  auto p = from_string<std::pair<std::string, std::string>>("key=value", '=');
  EXPECT_EQ(p.first, "key");
  EXPECT_EQ(p.second, "value");
}

TEST(FromStringTest, TupleTooManyElements) {
  EXPECT_THROW((from_string<std::tuple<int, int>>("1,2,3", ',')),
               std::invalid_argument);
  EXPECT_THROW((from_string<std::pair<int, int>>("1,2,3", ',')),
               std::invalid_argument);
  EXPECT_THROW((from_string<std::tuple<int>>("1,2", ',')),
               std::invalid_argument);
}

TEST(FromStringTest, TupleTooFewElements) {
  EXPECT_THROW((from_string<std::tuple<int, int, int>>("1,2", ',')),
               std::invalid_argument);
  EXPECT_THROW((from_string<std::pair<int, int>>("1", ',')),
               std::invalid_argument);
}

TEST(FromStringTest, TupleInvalidElement) {
  EXPECT_THROW((from_string<std::tuple<int, int>>("1,abc", ',')),
               std::invalid_argument);
  EXPECT_THROW((from_string<std::tuple<int, double>>("abc,3.14", ',')),
               std::invalid_argument);
  EXPECT_THROW((from_string<std::tuple<int, int, int>>("1,,3", ',')),
               std::invalid_argument);
}

TEST(FromStringTest, Array) {
  auto a = from_string<std::array<int, 3>>("1,2,3", ',');
  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(a[1], 2);
  EXPECT_EQ(a[2], 3);

  auto a2 = from_string<std::array<int, 4>>("10,20,30,40", ',');
  EXPECT_EQ(a2[0], 10);
  EXPECT_EQ(a2[1], 20);
  EXPECT_EQ(a2[2], 30);
  EXPECT_EQ(a2[3], 40);

  // Too many elements for array
  EXPECT_THROW((from_string<std::array<int, 2>>("1,2,3", ',')),
               std::invalid_argument);
  // Too few elements for array
  EXPECT_THROW((from_string<std::array<int, 3>>("1,2", ',')),
               std::invalid_argument);
}

TEST(FromStringTest, TupleEmptyString) {
  // Empty string with a 1-element tuple: split gives [""], which is 1 element
  // from_string<int>("") throws, so this should throw
  EXPECT_THROW((from_string<std::tuple<int>>("", ',')), std::invalid_argument);
}

// --- from_wstring: types constructible from std::wstring ---

TEST(FromWStringTest, ConstructibleFromWString) {
  auto ws = from_wstring<std::wstring>(std::wstring(L"hello"));
  EXPECT_EQ(ws, L"hello");
}
