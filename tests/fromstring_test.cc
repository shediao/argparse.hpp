//
// Created by shediao on 24-12-29.
//

#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

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

// --- from_string: container types (vector, list, set, deque, etc.) ---

TEST(FromStringTest, VectorInt) {
  auto v = from_string<std::vector<int>>("1,2,3,4,5", ',');
  EXPECT_EQ(v.size(), 5);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
  EXPECT_EQ(v[2], 3);
  EXPECT_EQ(v[3], 4);
  EXPECT_EQ(v[4], 5);
}

TEST(FromStringTest, VectorString) {
  auto v = from_string<std::vector<std::string>>("hello,world,foo,bar", ',');
  EXPECT_EQ(v.size(), 4);
  EXPECT_EQ(v[0], "hello");
  EXPECT_EQ(v[1], "world");
  EXPECT_EQ(v[2], "foo");
  EXPECT_EQ(v[3], "bar");
}

TEST(FromStringTest, VectorDouble) {
  auto v = from_string<std::vector<double>>("1.1,2.2,3.3", ',');
  EXPECT_EQ(v.size(), 3);
  EXPECT_DOUBLE_EQ(v[0], 1.1);
  EXPECT_DOUBLE_EQ(v[1], 2.2);
  EXPECT_DOUBLE_EQ(v[2], 3.3);
}

TEST(FromStringTest, VectorSingleElement) {
  auto v = from_string<std::vector<int>>("42", ',');
  EXPECT_EQ(v.size(), 1);
  EXPECT_EQ(v[0], 42);
}

TEST(FromStringTest, VectorEmptyString) {
  // Empty string: split gives [""], from_string<int>("") throws
  EXPECT_THROW((from_string<std::vector<int>>("", ',')), std::invalid_argument);
}

TEST(FromStringTest, VectorTrailingDelimiter) {
  // "1,2," -> split with -1 gives ["1", "2", ""]
  // from_string<int>("") throws
  EXPECT_THROW((from_string<std::vector<int>>("1,2,", ',')),
               std::invalid_argument);
}

TEST(FromStringTest, VectorLeadingDelimiter) {
  // ",1,2" -> split with -1 gives ["", "1", "2"]
  EXPECT_THROW((from_string<std::vector<int>>(",1,2", ',')),
               std::invalid_argument);
}

TEST(FromStringTest, VectorCustomDelimiter) {
  auto v = from_string<std::vector<int>>("10:20:30:40", ':');
  EXPECT_EQ(v.size(), 4);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 20);
  EXPECT_EQ(v[2], 30);
  EXPECT_EQ(v[3], 40);
}

TEST(FromStringTest, VectorInvalidElement) {
  EXPECT_THROW((from_string<std::vector<int>>("1,abc,3", ',')),
               std::invalid_argument);
  EXPECT_THROW((from_string<std::vector<double>>("1.0,xyz,3.0", ',')),
               std::invalid_argument);
}

TEST(FromStringTest, ListInt) {
  auto l = from_string<std::list<int>>("10,20,30,40", ',');
  EXPECT_EQ(l.size(), 4);
  auto it = l.begin();
  EXPECT_EQ(*it++, 10);
  EXPECT_EQ(*it++, 20);
  EXPECT_EQ(*it++, 30);
  EXPECT_EQ(*it++, 40);
  EXPECT_EQ(it, l.end());
}

TEST(FromStringTest, ListString) {
  auto l = from_string<std::list<std::string>>("a,b,c", ',');
  EXPECT_EQ(l.size(), 3);
  auto it = l.begin();
  EXPECT_EQ(*it++, "a");
  EXPECT_EQ(*it++, "b");
  EXPECT_EQ(*it++, "c");
}

TEST(FromStringTest, SetInt) {
  auto s = from_string<std::set<int>>("3,1,2,2,1", ',');
  // std::set deduplicates and sorts
  EXPECT_EQ(s.size(), 3);
  auto it = s.begin();
  EXPECT_EQ(*it++, 1);
  EXPECT_EQ(*it++, 2);
  EXPECT_EQ(*it++, 3);
}

TEST(FromStringTest, SetString) {
  auto s = from_string<std::set<std::string>>("c,a,b", ',');
  EXPECT_EQ(s.size(), 3);
  auto it = s.begin();
  EXPECT_EQ(*it++, "a");
  EXPECT_EQ(*it++, "b");
  EXPECT_EQ(*it++, "c");
}

TEST(FromStringTest, DequeInt) {
  auto d = from_string<std::deque<int>>("1,2,3", ',');
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 3);
}

// --- from_string: container of tuple-like types with two delimiters ---

TEST(FromStringTest, VectorOfPairs) {
  auto v =
      from_string<std::vector<std::pair<int, int>>>("1,2|3,4|5,6", '|', ',');
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], (std::pair<int, int>{1, 2}));
  EXPECT_EQ(v[1], (std::pair<int, int>{3, 4}));
  EXPECT_EQ(v[2], (std::pair<int, int>{5, 6}));
}

TEST(FromStringTest, VectorOfTuples) {
  auto v = from_string<std::vector<std::tuple<int, double, std::string>>>(
      "1,3.14,hello|2,2.71,world|3,0.5,foo", '|', ',');
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(std::get<0>(v[0]), 1);
  EXPECT_DOUBLE_EQ(std::get<1>(v[0]), 3.14);
  EXPECT_EQ(std::get<2>(v[0]), "hello");
  EXPECT_EQ(std::get<0>(v[1]), 2);
  EXPECT_DOUBLE_EQ(std::get<1>(v[1]), 2.71);
  EXPECT_EQ(std::get<2>(v[1]), "world");
  EXPECT_EQ(std::get<0>(v[2]), 3);
  EXPECT_DOUBLE_EQ(std::get<1>(v[2]), 0.5);
  EXPECT_EQ(std::get<2>(v[2]), "foo");
}

TEST(FromStringTest, VectorOfArrays) {
  auto v = from_string<std::vector<std::array<int, 3>>>("1,2,3|4,5,6|7,8,9",
                                                        '|', ',');
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], (std::array<int, 3>{1, 2, 3}));
  EXPECT_EQ(v[1], (std::array<int, 3>{4, 5, 6}));
  EXPECT_EQ(v[2], (std::array<int, 3>{7, 8, 9}));
}

TEST(FromStringTest, ListOfPairs) {
  auto l = from_string<std::list<std::pair<int, int>>>("10,20|30,40", '|', ',');
  EXPECT_EQ(l.size(), 2);
  auto it = l.begin();
  EXPECT_EQ(*it++, (std::pair<int, int>{10, 20}));
  EXPECT_EQ(*it++, (std::pair<int, int>{30, 40}));
  EXPECT_EQ(it, l.end());
}

TEST(FromStringTest, DequeOfTuples) {
  auto d =
      from_string<std::deque<std::tuple<int, int>>>("1,2|3,4|5,6", '|', ',');
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d[0], (std::tuple<int, int>{1, 2}));
  EXPECT_EQ(d[1], (std::tuple<int, int>{3, 4}));
  EXPECT_EQ(d[2], (std::tuple<int, int>{5, 6}));
}

TEST(FromStringTest, TwoDelimiterSingleElement) {
  auto v = from_string<std::vector<std::pair<int, int>>>("1,2", '|', ',');
  EXPECT_EQ(v.size(), 1);
  EXPECT_EQ(v[0], (std::pair<int, int>{1, 2}));
}

TEST(FromStringTest, TwoDelimiterCustomDelimiters) {
  auto v =
      from_string<std::vector<std::tuple<int, int>>>("1:2;3:4;5:6", ';', ':');
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], (std::tuple<int, int>{1, 2}));
  EXPECT_EQ(v[1], (std::tuple<int, int>{3, 4}));
  EXPECT_EQ(v[2], (std::tuple<int, int>{5, 6}));
}

TEST(FromStringTest, TwoDelimiterEmptyString) {
  // Empty string: split gives [""], from_string<pair<int,int>>("") throws
  EXPECT_THROW((from_string<std::vector<std::pair<int, int>>>("", '|', ',')),
               std::invalid_argument);
}

TEST(FromStringTest, TwoDelimiterTrailingDelimiter) {
  // "1,2|3,4|" -> split with -1 gives ["1,2", "3,4", ""]
  // from_string<pair<int,int>>("") throws
  EXPECT_THROW(
      (from_string<std::vector<std::pair<int, int>>>("1,2|3,4|", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, TwoDelimiterLeadingDelimiter) {
  // "|1,2|3,4" -> split with -1 gives ["", "1,2", "3,4"]
  // from_string<pair<int,int>>("") throws
  EXPECT_THROW(
      (from_string<std::vector<std::pair<int, int>>>("|1,2|3,4", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, TwoDelimiterInvalidInnerElement) {
  // Inner tuple has invalid element "abc"
  EXPECT_THROW(
      (from_string<std::vector<std::pair<int, int>>>("1,abc|3,4", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, TwoDelimiterTooFewInnerElements) {
  // Inner pair expects 2 elements but gets "1"
  EXPECT_THROW(
      (from_string<std::vector<std::pair<int, int>>>("1|3,4", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, TwoDelimiterTooManyInnerElements) {
  // Inner pair expects 2 elements but gets "1,2,3"
  EXPECT_THROW(
      (from_string<std::vector<std::pair<int, int>>>("1,2,3|4,5", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, TwoDelimiterInnerPairStringString) {
  auto v = from_string<std::vector<std::pair<std::string, std::string>>>(
      "k1=v1|k2=v2|k3=v3", '|', '=');
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], (std::pair<std::string, std::string>{"k1", "v1"}));
  EXPECT_EQ(v[1], (std::pair<std::string, std::string>{"k2", "v2"}));
  EXPECT_EQ(v[2], (std::pair<std::string, std::string>{"k3", "v3"}));
}

// --- from_string: map-like types with two delimiters ---

TEST(FromStringTest, MapIntString) {
  auto m =
      from_string<std::map<int, std::string>>("1,one|2,two|3,three", '|', ',');
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m.at(1), "one");
  EXPECT_EQ(m.at(2), "two");
  EXPECT_EQ(m.at(3), "three");
}

TEST(FromStringTest, MapStringInt) {
  auto m = from_string<std::map<std::string, int>>("apple,5|banana,3|cherry,8",
                                                   '|', ',');
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m.at("apple"), 5);
  EXPECT_EQ(m.at("banana"), 3);
  EXPECT_EQ(m.at("cherry"), 8);
}

TEST(FromStringTest, MapStringString) {
  auto m = from_string<std::map<std::string, std::string>>("k1=v1|k2=v2|k3=v3",
                                                           '|', '=');
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m.at("k1"), "v1");
  EXPECT_EQ(m.at("k2"), "v2");
  EXPECT_EQ(m.at("k3"), "v3");
}

TEST(FromStringTest, MapIntDouble) {
  auto m = from_string<std::map<int, double>>("1,1.5|2,2.5|3,3.75", '|', ',');
  EXPECT_EQ(m.size(), 3);
  EXPECT_DOUBLE_EQ(m.at(1), 1.5);
  EXPECT_DOUBLE_EQ(m.at(2), 2.5);
  EXPECT_DOUBLE_EQ(m.at(3), 3.75);
}

TEST(FromStringTest, UnorderedMapIntString) {
  auto m = from_string<std::unordered_map<int, std::string>>(
      "1,one|2,two|3,three", '|', ',');
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m.at(1), "one");
  EXPECT_EQ(m.at(2), "two");
  EXPECT_EQ(m.at(3), "three");
}

TEST(FromStringTest, UnorderedMapStringInt) {
  auto m = from_string<std::unordered_map<std::string, int>>("x,10|y,20|z,30",
                                                             '|', ',');
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m.at("x"), 10);
  EXPECT_EQ(m.at("y"), 20);
  EXPECT_EQ(m.at("z"), 30);
}

TEST(FromStringTest, MultimapIntString) {
  auto m = from_string<std::multimap<int, std::string>>(
      "1,one|1,uno|2,two|2,dos", '|', ',');
  EXPECT_EQ(m.size(), 4);
  auto range = m.equal_range(1);
  auto it = range.first;
  EXPECT_EQ(it->second, "one");
  ++it;
  EXPECT_EQ(it->second, "uno");
  range = m.equal_range(2);
  it = range.first;
  EXPECT_EQ(it->second, "two");
  ++it;
  EXPECT_EQ(it->second, "dos");
}

TEST(FromStringTest, MapSingleElement) {
  auto m = from_string<std::map<int, std::string>>("42,answer", '|', ',');
  EXPECT_EQ(m.size(), 1);
  EXPECT_EQ(m.at(42), "answer");
}

TEST(FromStringTest, MapEmptyString) {
  // Empty string: split gives [""], from_string<pair> on "" throws
  EXPECT_THROW((from_string<std::map<int, std::string>>("", '|', ',')),
               std::invalid_argument);
}

TEST(FromStringTest, MapTrailingDelimiter) {
  // "1,one|2,two|" -> last split element is ""
  EXPECT_THROW(
      (from_string<std::map<int, std::string>>("1,one|2,two|", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, MapLeadingDelimiter) {
  // "|1,one|2,two" -> first split element is ""
  EXPECT_THROW(
      (from_string<std::map<int, std::string>>("|1,one|2,two", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, MapInvalidKey) {
  // "abc" cannot be parsed as int key
  EXPECT_THROW(
      (from_string<std::map<int, std::string>>("abc,val|2,two", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, MapInvalidValue) {
  // "xyz" cannot be parsed as int value
  EXPECT_THROW(
      (from_string<std::map<std::string, int>>("k1,xyz|k2,2", '|', ',')),
      std::invalid_argument);
}

TEST(FromStringTest, MapTooFewInnerElements) {
  // "1" is not a valid key-value pair (missing value)
  EXPECT_THROW((from_string<std::map<int, std::string>>("1|2,two", '|', ',')),
               std::invalid_argument);
}

TEST(FromStringTest, MapTooManyInnerElements) {
  // "1,one,extra" has too many elements for a pair
  EXPECT_THROW(
      (from_string<std::map<int, std::string>>("1,one,extra|2,two", '|', ',')),
      std::invalid_argument);
}

// --- from_wstring: types constructible from std::wstring ---

TEST(FromWStringTest, ConstructibleFromWString) {
  auto ws = from_wstring<std::wstring>(std::wstring(L"hello"));
  EXPECT_EQ(ws, L"hello");
}
