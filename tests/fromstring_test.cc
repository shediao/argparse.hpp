//
// Created by shediao on 24-12-29.
//

#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <filesystem>
#include <limits>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

#include "argparse/argparse.hpp"

using argparse::detail::from_string;
using argparse::detail::split;
using argparse::detail::split_to_if;

// Test bool
TEST(ParseFromStringTest, BoolTest) {
  EXPECT_EQ(from_string<bool>("true"), true);
  EXPECT_EQ(from_string<bool>("false"), false);
  EXPECT_EQ(from_string<bool>("1"), true);
  EXPECT_EQ(from_string<bool>("0"), false);
  EXPECT_EQ(from_string<bool>("on"), true);
  EXPECT_EQ(from_string<bool>("off"), false);
}

// Test int
TEST(ParseFromStringTest, IntTest) {
  EXPECT_EQ(from_string<int>("123"), 123);
  EXPECT_EQ(from_string<int>("-456"), -456);
  EXPECT_FALSE(from_string<int>("12.34"));
  EXPECT_FALSE(from_string<int>("abc"));
}

// Test long
TEST(ParseFromStringTest, LongTest) {
  EXPECT_EQ(from_string<long>("1234567890"), 1234567890L);
  if constexpr (sizeof(long) > 4) {
    // 32 bit long: max 2,147,483,647, min -2,147,483,648
    EXPECT_EQ(from_string<long>("-9876543210"), -9876543210L);
  }
  EXPECT_EQ(from_string<long>("-2147483648"), -2147483648L);
  EXPECT_FALSE(from_string<long>("12.34"));
  EXPECT_FALSE(from_string<long>("abc"));
}

// Test double
TEST(ParseFromStringTest, DoubleTest) {
  EXPECT_DOUBLE_EQ(from_string<double>("3.14").value(), 3.14);
  EXPECT_DOUBLE_EQ(from_string<double>("-2.71").value(), -2.71);
  EXPECT_FALSE(from_string<double>("abc"));
}

// Test long long
TEST(ParseFromStringTest, LongLongTest) {
  EXPECT_EQ(from_string<long long>("123456789012345"), 123456789012345LL);
  EXPECT_EQ(from_string<long long>("-987654321098765"), -987654321098765LL);
  EXPECT_FALSE(from_string<long long>("12.34"));
  EXPECT_FALSE(from_string<long long>("abc"));
}

TEST(ParseFromStringTest, split) {
  EXPECT_EQ(split("k=v", '=', 1), (std::vector<std::string>{"k", "v"}));
  EXPECT_EQ(split("1,2,3", ',', 1), (std::vector<std::string>{"1", "2,3"}));
  EXPECT_EQ(split("1,2,3", ',', 2), (std::vector<std::string>{"1", "2", "3"}));
  EXPECT_EQ(split("1,2,3", ',', 3), (std::vector<std::string>{"1", "2", "3"}));
  EXPECT_EQ(split("1,", ',', -1), (std::vector<std::string>{"1", ""}));
  EXPECT_EQ(split(",1", ',', -1), (std::vector<std::string>{"", "1"}));
}

// ============================================================
// Tests for split_to_if (return-by-value, C++20 concepts)
// ============================================================

// --- Basic return-by-value semantics ---

TEST(SplitToIfTest, BasicSplitComma) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string("a,b,c"), is_comma);
  EXPECT_EQ(result, (std::vector<std::string>{"a", "b", "c"}));
}

TEST(SplitToIfTest, BasicSplitSpace) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello world foo"), is_space);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world", "foo"}));
}

TEST(SplitToIfTest, SingleElement) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string("hello"), is_comma);
  EXPECT_EQ(result, (std::vector<std::string>{"hello"}));
}

TEST(SplitToIfTest, EmptyString) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string(""), is_comma);
  EXPECT_EQ(result, (std::vector<std::string>{""}));
}

TEST(SplitToIfTest, OnlyDelimiters) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string(",,,"), is_comma);
  EXPECT_EQ(result, (std::vector<std::string>{"", "", "", ""}));
}

TEST(SplitToIfTest, TrailingDelimiter) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string("a,b,"), is_comma);
  EXPECT_EQ(result, (std::vector<std::string>{"a", "b", ""}));
}

TEST(SplitToIfTest, LeadingDelimiter) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string(",a,b"), is_comma);
  EXPECT_EQ(result, (std::vector<std::string>{"", "a", "b"}));
}

TEST(SplitToIfTest, AdjacentDelimiters) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string("a,,b"), is_comma);
  EXPECT_EQ(result, (std::vector<std::string>{"a", "", "b"}));
}

// --- split_count parameter ---

TEST(SplitToIfTest, SplitCountZero) {
  auto is_comma = [](char c) { return c == ','; };
  // split_count=0 means the loop condition (split_count < 0 || count++ < 0)
  // is false from the start, so no splits occur and the entire string is
  // returned as a single element.
  auto result =
      split_to_if<std::vector<std::string>>(std::string("1,2,3"), is_comma, 0);
  EXPECT_EQ(result, (std::vector<std::string>{"1,2,3"}));
}

TEST(SplitToIfTest, SplitCountOne) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string("1,2,3"), is_comma, 1);
  EXPECT_EQ(result, (std::vector<std::string>{"1", "2,3"}));
}

TEST(SplitToIfTest, SplitCountTwo) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string("1,2,3"), is_comma, 2);
  EXPECT_EQ(result, (std::vector<std::string>{"1", "2", "3"}));
}

TEST(SplitToIfTest, SplitCountExceedsDelimiters) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string("a,b"), is_comma, 10);
  EXPECT_EQ(result, (std::vector<std::string>{"a", "b"}));
}

TEST(SplitToIfTest, SplitCountNegative) {
  auto is_comma = [](char c) { return c == ','; };
  auto result = split_to_if<std::vector<std::string>>(std::string("a,b,c,d"),
                                                      is_comma, -1);
  EXPECT_EQ(result, (std::vector<std::string>{"a", "b", "c", "d"}));
}

// --- compress_tokens mode ---

TEST(SplitToIfTest, CompressTokensBasic) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello   world  foo"), is_space, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world", "foo"}));
}

TEST(SplitToIfTest, CompressTokensSingleSpace) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello world foo"), is_space, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world", "foo"}));
}

TEST(SplitToIfTest, CompressTokensLeadingSpaces) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("   hello world"), is_space, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"", "hello", "world"}));
}

TEST(SplitToIfTest, CompressTokensTrailingSpaces) {
  // Key behavioral change (bug fix): trailing delimiters with
  // compress_tokens should NOT produce an empty trailing element.
  // The new code adds an early return when find_if_not reaches str.end().
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello world   "), is_space, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world"}));
}

TEST(SplitToIfTest, CompressTokensOnlySpaces) {
  // All-delimiter string: the first delimiter is at position 0, producing an
  // empty token. Then find_if_not reaches end, triggering early return.
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(std::string("     "),
                                                      is_space, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{""}));
}

TEST(SplitToIfTest, CompressTokensEmptyString) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(std::string(""), is_space,
                                                      -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{""}));
}

TEST(SplitToIfTest, CompressTokensWithSplitCount) {
  auto is_space = [](char c) { return c == ' '; };
  // Split at most once: "hello" and the remainder "  world   foo"
  // (compress_tokens only applies between splits, not to the final chunk)
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello   world   foo"), is_space, 1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world   foo"}));
}

TEST(SplitToIfTest, CompressTokensTrailingAfterSplitCount) {
  auto is_space = [](char c) { return c == ' '; };
  // Split at most once: trailing delimiters after the split point are
  // preserved in the remainder (no compress_tokens on the final chunk).
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello   world   "), is_space, 1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world   "}));
}

TEST(SplitToIfTest, CompressTokensWithTabs) {
  auto is_blank = [](char c) { return c == ' ' || c == '\t'; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello\t\tworld  foo"), is_blank, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world", "foo"}));
}

TEST(SplitToIfTest, CompressTokensTrailingMixedWhitespace) {
  // Trailing mixed whitespace with compress_tokens should produce no empty
  // trailing element (same bug fix as CompressTokensTrailingSpaces).
  auto is_blank = [](char c) { return c == ' ' || c == '\t'; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello\t world \t "), is_blank, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world"}));
}

// --- split_count=0 with compress_tokens ---

TEST(SplitToIfTest, SplitCountZeroWithCompress) {
  // split_count=0 means the loop body never executes, so compress_tokens
  // has no effect and the entire string is returned as a single element.
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("   hello   "), is_space, 0, true);
  EXPECT_EQ(result, (std::vector<std::string>{"   hello   "}));
}

TEST(SplitToIfTest, SplitCountZeroWithCompressEmpty) {
  auto is_space = [](char c) { return c == ' '; };
  auto result =
      split_to_if<std::vector<std::string>>(std::string(""), is_space, 0, true);
  EXPECT_EQ(result, (std::vector<std::string>{""}));
}

TEST(SplitToIfTest, SplitCountZeroWithCompressOnlyDelims) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(std::string("     "),
                                                      is_space, 0, true);
  EXPECT_EQ(result, (std::vector<std::string>{"     "}));
}

// --- compress_tokens with a single / only delimiter characters ---

TEST(SplitToIfTest, CompressTokensSingleDelimiter) {
  // A single delimiter: find_if finds it at position 0, pushes "".
  // find_if_not then reaches str.end() (no non-delimiter char),
  // triggering early return.  Result is a single empty token.
  auto is_comma = [](char c) { return c == ','; };
  auto result = split_to_if<std::vector<std::string>>(std::string(","),
                                                      is_comma, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{""}));
}

TEST(SplitToIfTest, CompressTokensTwoDelimiters) {
  // Two consecutive delimiters: same early-return path as the single-
  // delimiter case — find_if_not from position 0 reaches end immediately.
  auto is_comma = [](char c) { return c == ','; };
  auto result = split_to_if<std::vector<std::string>>(std::string(",,"),
                                                      is_comma, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{""}));
}

// --- compress_tokens with adjacent delimiters in the middle ---

TEST(SplitToIfTest, CompressTokensAdjacentDelimiters) {
  // Adjacent delimiters between tokens are skipped by compress_tokens.
  auto is_comma = [](char c) { return c == ','; };
  auto result = split_to_if<std::vector<std::string>>(std::string("a,,b"),
                                                      is_comma, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"a", "b"}));
}

// --- negative split_count values other than -1 ---

TEST(SplitToIfTest, SplitCountNegativeVarious) {
  auto is_comma = [](char c) { return c == ','; };
  // Any negative value means "unlimited splits".
  EXPECT_EQ(
      split_to_if<std::vector<std::string>>(std::string("a,b,c"), is_comma, -2),
      (std::vector<std::string>{"a", "b", "c"}));
  EXPECT_EQ(split_to_if<std::vector<std::string>>(std::string("a,b,c"),
                                                  is_comma, -100),
            (std::vector<std::string>{"a", "b", "c"}));
}

// --- compress_tokens with split_count > 1 ---

TEST(SplitToIfTest, CompressTokensWithSplitCountTwo) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("hello   world   foo"), is_space, 2, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello", "world", "foo"}));
}

TEST(SplitToIfTest, CompressTokensWithSplitCountThree) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::vector<std::string>>(
      std::string("a  b  c  d  e"), is_space, 3, true);
  EXPECT_EQ(result, (std::vector<std::string>{"a", "b", "c", "d  e"}));
}

// --- compress_tokens with split_count and leading delimiters ---

TEST(SplitToIfTest, CompressTokensSplitCountLeadingDelims) {
  auto is_comma = [](char c) { return c == ','; };
  // Leading delimiters produce an empty first token; compress skips
  // consecutive commas, then split_count=1 stops further splitting.
  auto result = split_to_if<std::vector<std::string>>(std::string(",,a,b,c"),
                                                      is_comma, 1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"", "a,b,c"}));
}

// --- compress_tokens with a predicate that never matches ---

TEST(SplitToIfTest, CompressTokensNeverMatch) {
  auto never = [](char /*c*/) { return false; };
  // No delimiter found → loop never enters → whole string returned as-is.
  auto result = split_to_if<std::vector<std::string>>(std::string("hello"),
                                                      never, -1, true);
  EXPECT_EQ(result, (std::vector<std::string>{"hello"}));
}

// --- Alternate container types (verifies split_to_if is not hardcoded to
//     std::vector<std::string>) ---

TEST(SplitToIfTest, DequeString) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::deque<std::string>>(std::string("a,b,c"), is_comma);
  EXPECT_EQ(result, (std::deque<std::string>{"a", "b", "c"}));
}

TEST(SplitToIfTest, ListString) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::list<std::string>>(std::string("a,b,c"), is_comma);
  EXPECT_EQ(result, (std::list<std::string>{"a", "b", "c"}));
}

TEST(SplitToIfTest, DequeWithCompressTokens) {
  auto is_space = [](char c) { return c == ' '; };
  auto result = split_to_if<std::deque<std::string>>(
      std::string("hello   world"), is_space, -1, true);
  EXPECT_EQ(result, (std::deque<std::string>{"hello", "world"}));
}

TEST(SplitToIfTest, DequeWithSplitCount) {
  auto is_comma = [](char c) { return c == ','; };
  auto result =
      split_to_if<std::deque<std::string>>(std::string("1,2,3,4"), is_comma, 2);
  EXPECT_EQ(result, (std::deque<std::string>{"1", "2", "3,4"}));
}

// --- Wide string (std::wstring) tests ---

TEST(SplitToIfTest, WstringBasic) {
  auto is_comma = [](wchar_t c) { return c == L','; };
  auto result =
      split_to_if<std::vector<std::wstring>>(std::wstring(L"a,b,c"), is_comma);
  EXPECT_EQ(result, (std::vector<std::wstring>{L"a", L"b", L"c"}));
}

TEST(SplitToIfTest, WstringSingleElement) {
  auto is_comma = [](wchar_t c) { return c == L','; };
  auto result =
      split_to_if<std::vector<std::wstring>>(std::wstring(L"hello"), is_comma);
  EXPECT_EQ(result, (std::vector<std::wstring>{L"hello"}));
}

TEST(SplitToIfTest, WstringEmpty) {
  auto is_comma = [](wchar_t c) { return c == L','; };
  auto result =
      split_to_if<std::vector<std::wstring>>(std::wstring(L""), is_comma);
  EXPECT_EQ(result, (std::vector<std::wstring>{L""}));
}

TEST(SplitToIfTest, WstringTrailingDelimiter) {
  auto is_comma = [](wchar_t c) { return c == L','; };
  auto result =
      split_to_if<std::vector<std::wstring>>(std::wstring(L"a,b,"), is_comma);
  EXPECT_EQ(result, (std::vector<std::wstring>{L"a", L"b", L""}));
}

TEST(SplitToIfTest, WstringWithSplitCount) {
  auto is_comma = [](wchar_t c) { return c == L','; };
  auto result = split_to_if<std::vector<std::wstring>>(std::wstring(L"1,2,3,4"),
                                                       is_comma, 2);
  EXPECT_EQ(result, (std::vector<std::wstring>{L"1", L"2", L"3,4"}));
}

TEST(SplitToIfTest, WstringCompressTokens) {
  auto is_space = [](wchar_t c) { return c == L' '; };
  auto result = split_to_if<std::vector<std::wstring>>(
      std::wstring(L"hello   world  foo"), is_space, -1, true);
  EXPECT_EQ(result, (std::vector<std::wstring>{L"hello", L"world", L"foo"}));
}

TEST(SplitToIfTest, WstringCompressTokensTrailing) {
  auto is_space = [](wchar_t c) { return c == L' '; };
  auto result = split_to_if<std::vector<std::wstring>>(
      std::wstring(L"hello world   "), is_space, -1, true);
  EXPECT_EQ(result, (std::vector<std::wstring>{L"hello", L"world"}));
}

TEST(SplitToIfTest, WstringDeque) {
  auto is_comma = [](wchar_t c) { return c == L','; };
  auto result =
      split_to_if<std::deque<std::wstring>>(std::wstring(L"x,y,z"), is_comma);
  EXPECT_EQ(result, (std::deque<std::wstring>{L"x", L"y", L"z"}));
}

TEST(SplitToIfTest, WstringCompressTokensWithSplitCount) {
  auto is_space = [](wchar_t c) { return c == L' '; };
  auto result = split_to_if<std::vector<std::wstring>>(
      std::wstring(L"hello   world   foo"), is_space, 1, true);
  EXPECT_EQ(result, (std::vector<std::wstring>{L"hello", L"world   foo"}));
}

TEST(ParseFromStringTest, Sizeable) {
  EXPECT_EQ((from_string<std::pair<int, int>>("1,2", ',')),
            (std::pair<int, int>{1, 2}));
  EXPECT_EQ((from_string<std::tuple<int, int, int>>("1,2,3", ',')),
            (std::tuple{1, 2, 3}));

  EXPECT_EQ((from_string<std::pair<std::string, std::string>>("k=v", '=')),
            (std::pair<std::string, std::string>{"k", "v"}));
  EXPECT_EQ((from_string<std::pair<std::string, std::string>>("k=", '=')),
            (std::pair<std::string, std::string>{"k", ""}));
  EXPECT_EQ((from_string<std::pair<std::string, std::string>>("k=v=vv", '=')),
            (std::pair<std::string, std::string>{"k", "v=vv"}));

  EXPECT_FALSE((from_string<std::pair<std::string, std::string>>("k", '=')));
}

TEST(ParseFromStringTest, Container) {
  EXPECT_EQ((from_string<std::tuple<int, int, int>>("1,2,3", ',')),
            (std::tuple<int, int, int>{1, 2, 3}));
  EXPECT_EQ((from_string<std::array<int, 3>>("1,2,3", ',')),
            (std::array<int, 3>{1, 2, 3}));
}

// ============================================================
// Tests for the new from_string (C++20 concepts)
// ============================================================

// --- from_string: integral types ---

TEST(FromStringTest, Int) {
  EXPECT_EQ(from_string<int>("0"), 0);
  EXPECT_EQ(from_string<int>("123"), 123);
  EXPECT_EQ(from_string<int>("-456"), -456);
  EXPECT_EQ(from_string<int>("2147483647"), 2147483647);
  EXPECT_EQ(from_string<int>("-2147483648"), -2147483648);
  EXPECT_FALSE(from_string<int>("12.34"));
  EXPECT_FALSE(from_string<int>("abc"));
  EXPECT_FALSE(from_string<int>(""));
}

TEST(FromStringTest, Long) {
  EXPECT_EQ(from_string<long>("0"), 0L);
  EXPECT_EQ(from_string<long>("1234567890"), 1234567890L);
  EXPECT_EQ(from_string<long>("-987654321"), -987654321L);
  EXPECT_FALSE(from_string<long>("12.34"));
  EXPECT_FALSE(from_string<long>("abc"));
  EXPECT_FALSE(from_string<long>(""));
}

TEST(FromStringTest, UnsignedLong) {
  EXPECT_EQ(from_string<unsigned long>("0"), 0UL);
  EXPECT_EQ(from_string<unsigned long>("4294967295"), 4294967295UL);
  // std::stoul wraps negative values (standard behavior)
  EXPECT_EQ(from_string<unsigned long>("-1"),
            (std::numeric_limits<unsigned long>::max)());
  EXPECT_FALSE(from_string<unsigned long>("abc"));
  EXPECT_FALSE(from_string<unsigned long>(""));
}

TEST(FromStringTest, LongLong) {
  EXPECT_EQ(from_string<long long>("0"), 0LL);
  EXPECT_EQ(from_string<long long>("123456789012345"), 123456789012345LL);
  EXPECT_EQ(from_string<long long>("-987654321098765"), -987654321098765LL);
  EXPECT_EQ(from_string<long long>("9223372036854775807"),
            9223372036854775807LL);
  EXPECT_FALSE(from_string<long long>("12.34"));
  EXPECT_FALSE(from_string<long long>("abc"));
  EXPECT_FALSE(from_string<long long>(""));
}

TEST(FromStringTest, UnsignedLongLong) {
  EXPECT_EQ(from_string<unsigned long long>("0"), 0ULL);
  EXPECT_EQ(from_string<unsigned long long>("18446744073709551615"),
            18446744073709551615ULL);
  // std::stoull wraps negative values (standard behavior)
  EXPECT_EQ(from_string<unsigned long long>("-1"),
            (std::numeric_limits<unsigned long long>::max)());
  EXPECT_FALSE(from_string<unsigned long long>("abc"));
  EXPECT_FALSE(from_string<unsigned long long>(""));
}

// --- from_string: floating-point types ---

TEST(FromStringTest, Float) {
  EXPECT_FLOAT_EQ(from_string<float>("0").value(), 0.0f);
  EXPECT_FLOAT_EQ(from_string<float>("3.14").value(), 3.14f);
  EXPECT_FLOAT_EQ(from_string<float>("-2.71").value(), -2.71f);
  EXPECT_FLOAT_EQ(from_string<float>("1.5e2").value(), 150.0f);
  EXPECT_FALSE(from_string<float>("abc"));
  EXPECT_FALSE(from_string<float>(""));
}

TEST(FromStringTest, Double) {
  EXPECT_DOUBLE_EQ(from_string<double>("0").value(), 0.0);
  EXPECT_DOUBLE_EQ(from_string<double>("3.14159265358979").value(),
                   3.14159265358979);
  EXPECT_DOUBLE_EQ(from_string<double>("-2.71828182845905").value(),
                   -2.71828182845905);
  EXPECT_DOUBLE_EQ(from_string<double>("1e-10").value(), 1e-10);
  EXPECT_FALSE(from_string<double>("abc"));
  EXPECT_FALSE(from_string<double>(""));
}

TEST(FromStringTest, LongDouble) {
  EXPECT_DOUBLE_EQ(static_cast<double>(from_string<long double>("0").value()),
                   0.0);
  EXPECT_DOUBLE_EQ(
      static_cast<double>(from_string<long double>("3.14").value()), 3.14);
  EXPECT_DOUBLE_EQ(
      static_cast<double>(from_string<long double>("-2.71").value()), -2.71);
  EXPECT_FALSE(from_string<long double>("abc"));
  EXPECT_FALSE(from_string<long double>(""));
}

// --- from_string: additional integral types ---

TEST(FromStringTest, UnsignedInt) {
  EXPECT_EQ(from_string<unsigned int>("0"), 0U);
  EXPECT_EQ(from_string<unsigned int>("123"), 123U);
  EXPECT_EQ(from_string<unsigned int>("4294967295"), 4294967295U);
  EXPECT_FALSE(from_string<unsigned int>("abc"));
  EXPECT_FALSE(from_string<unsigned int>(""));
  EXPECT_FALSE(from_string<unsigned int>("4294967296"));
}

TEST(FromStringTest, Short) {
  EXPECT_EQ(from_string<short>("0"), 0);
  EXPECT_EQ(from_string<short>("123"), 123);
  EXPECT_EQ(from_string<short>("-456"), -456);
  EXPECT_EQ(from_string<short>("32767"), 32767);
  EXPECT_EQ(from_string<short>("-32768"), -32768);
  EXPECT_FALSE(from_string<short>("abc"));
  EXPECT_FALSE(from_string<short>(""));
  EXPECT_FALSE(from_string<short>("32768"));
  EXPECT_FALSE(from_string<short>("-32769"));
}

TEST(FromStringTest, UnsignedShort) {
  EXPECT_EQ(from_string<unsigned short>("0"), static_cast<unsigned short>(0));
  EXPECT_EQ(from_string<unsigned short>("123"),
            static_cast<unsigned short>(123));
  EXPECT_EQ(from_string<unsigned short>("65535"),
            static_cast<unsigned short>(65535));
  EXPECT_FALSE(from_string<unsigned short>("abc"));
  EXPECT_FALSE(from_string<unsigned short>(""));
  EXPECT_FALSE(from_string<unsigned short>("65536"));
}

// --- from_string: tuple-like types (pair, tuple, array) ---

TEST(FromStringTest, Pair) {
  auto p = from_string<std::pair<int, int>>("1,2", ',');
  EXPECT_EQ(p.value().first, 1);
  EXPECT_EQ(p.value().second, 2);

  auto ps =
      from_string<std::pair<std::string, std::string>>("hello,world", ',');
  EXPECT_EQ(ps.value().first, "hello");
  EXPECT_EQ(ps.value().second, "world");

  auto pm = from_string<std::pair<int, double>>("42,3.14", ',');
  EXPECT_EQ(pm.value().first, 42);
  EXPECT_DOUBLE_EQ(pm.value().second, 3.14);
}

TEST(FromStringTest, TupleBasic) {
  auto t2 = from_string<std::tuple<int, int>>("10,20", ',');
  EXPECT_EQ(std::get<0>(t2.value()), 10);
  EXPECT_EQ(std::get<1>(t2.value()), 20);

  auto t3 = from_string<std::tuple<int, int, int>>("1,2,3", ',');
  EXPECT_EQ(std::get<0>(t3.value()), 1);
  EXPECT_EQ(std::get<1>(t3.value()), 2);
  EXPECT_EQ(std::get<2>(t3.value()), 3);
}

TEST(FromStringTest, TupleSingleElement) {
  auto t1 = from_string<std::tuple<int>>("42", ',');
  EXPECT_EQ(std::get<0>(t1.value()), 42);

  auto ts1 = from_string<std::tuple<std::string>>("hello", ',');
  EXPECT_EQ(std::get<0>(ts1.value()), "hello");
}

TEST(FromStringTest, TupleMixedTypes) {
  auto t =
      from_string<std::tuple<int, double, std::string>>("1,3.14,hello", ',');
  EXPECT_EQ(std::get<0>(t.value()), 1);
  EXPECT_DOUBLE_EQ(std::get<1>(t.value()), 3.14);
  EXPECT_EQ(std::get<2>(t.value()), "hello");
}

TEST(FromStringTest, TupleCustomDelimiter) {
  auto t = from_string<std::tuple<int, int, int>>("1:2:3", ':');
  EXPECT_EQ(std::get<0>(t.value()), 1);
  EXPECT_EQ(std::get<1>(t.value()), 2);
  EXPECT_EQ(std::get<2>(t.value()), 3);

  auto p = from_string<std::pair<std::string, std::string>>("key=value", '=');
  EXPECT_EQ(p.value().first, "key");
  EXPECT_EQ(p.value().second, "value");
}

TEST(FromStringTest, TupleTooManyElements) {
  EXPECT_FALSE((from_string<std::tuple<int, int>>("1,2,3", ',')));
  EXPECT_FALSE((from_string<std::pair<int, int>>("1,2,3", ',')));
  EXPECT_FALSE((from_string<std::tuple<int>>("1,2", ',')));
}

TEST(FromStringTest, TupleTooFewElements) {
  EXPECT_FALSE((from_string<std::tuple<int, int, int>>("1,2", ',')));
  EXPECT_FALSE((from_string<std::pair<int, int>>("1", ',')));
}

TEST(FromStringTest, TupleInvalidElement) {
  EXPECT_FALSE((from_string<std::tuple<int, int>>("1,abc", ',')));
  EXPECT_FALSE((from_string<std::tuple<int, double>>("abc,3.14", ',')));
  EXPECT_FALSE((from_string<std::tuple<int, int, int>>("1,,3", ',')));
}

TEST(FromStringTest, Array) {
  auto a = from_string<std::array<int, 3>>("1,2,3", ',');
  EXPECT_EQ(a.value()[0], 1);
  EXPECT_EQ(a.value()[1], 2);
  EXPECT_EQ(a.value()[2], 3);

  auto a2 = from_string<std::array<int, 4>>("10,20,30,40", ',');
  EXPECT_EQ(a2.value()[0], 10);
  EXPECT_EQ(a2.value()[1], 20);
  EXPECT_EQ(a2.value()[2], 30);
  EXPECT_EQ(a2.value()[3], 40);

  // Too many elements for array
  EXPECT_FALSE((from_string<std::array<int, 2>>("1,2,3", ',')));
  // Too few elements for array
  EXPECT_FALSE((from_string<std::array<int, 3>>("1,2", ',')));
}

TEST(FromStringTest, TupleEmptyString) {
  // Empty string with a 1-element tuple: split gives [""], which is 1 element
  // from_string<int>("") throws, so this should throw
  EXPECT_FALSE((from_string<std::tuple<int>>("", ',')));
}

// --- from_string: char ---

TEST(FromStringTest, Char) {
  EXPECT_EQ(from_string<char>("a"), 'a');
  EXPECT_EQ(from_string<char>("z"), 'z');
  EXPECT_EQ(from_string<char>("0"), '0');
  EXPECT_EQ(from_string<char>(" "), ' ');
  // Multi-character string should throw
  EXPECT_FALSE(from_string<char>("ab"));
  EXPECT_FALSE(from_string<char>(""));
}

// --- from_string: types constructible from std::string ---

TEST(FromStringTest, FilesystemPath) {
  // std::filesystem::path is constructible from std::string
  auto p = from_string<std::filesystem::path>("/usr/local/bin");
  EXPECT_EQ(p, std::filesystem::path("/usr/local/bin"));

  auto p2 = from_string<std::filesystem::path>("relative/path");
  EXPECT_EQ(p2, std::filesystem::path("relative/path"));

  // Empty path
  auto p3 = from_string<std::filesystem::path>("");
  EXPECT_EQ(p3, std::filesystem::path(""));
}

// --- from_string: tuple-like with std::filesystem::path elements ---

TEST(FromStringTest, TupleWithFilesystemPath) {
  auto t =
      from_string<std::tuple<std::filesystem::path, std::filesystem::path>>(
          "/usr/bin,/usr/local/bin", ',');
  EXPECT_EQ(std::get<0>(t.value()), std::filesystem::path("/usr/bin"));
  EXPECT_EQ(std::get<1>(t.value()), std::filesystem::path("/usr/local/bin"));
}

TEST(FromStringTest, PairWithFilesystemPath) {
  auto p =
      from_string<std::pair<std::filesystem::path, int>>("/tmp/test,42", ',');
  EXPECT_EQ(p.value().first, std::filesystem::path("/tmp/test"));
  EXPECT_EQ(p.value().second, 42);
}
