#include <gtest/gtest.h>

#include <array>
#include <forward_list>
#include <initializer_list>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "argparse.hpp"

// 测试 is_tuple_like concept
TEST(ConceptTest, IsTupleLike) {
    ASSERT_TRUE((argparse::is_tuple_like<std::tuple<int, std::string>>));
    ASSERT_TRUE((argparse::is_tuple_like<std::pair<int, double>>));
    ASSERT_TRUE((argparse::is_tuple_like<std::array<int, 3>>));
    ASSERT_TRUE(!argparse::is_tuple_like<std::vector<int>>);
    ASSERT_TRUE(!argparse::is_tuple_like<std::string>);
}

// 测试 is_parse_from_string_basic_type concept
TEST(ConceptTest, CanParseFromStringWithoutSplit) {
    ASSERT_TRUE(!argparse::is_parse_from_string_basic_type<char>);
    ASSERT_TRUE(!argparse::is_parse_from_string_basic_type<wchar_t>);
    ASSERT_TRUE(!argparse::is_parse_from_string_basic_type<char16_t>);
    ASSERT_TRUE(!argparse::is_parse_from_string_basic_type<char32_t>);
    ASSERT_TRUE(argparse::is_parse_from_string_basic_type<bool>);
    ASSERT_TRUE(argparse::is_parse_from_string_basic_type<int>);
    ASSERT_TRUE(argparse::is_parse_from_string_basic_type<double>);
    ASSERT_TRUE(argparse::is_parse_from_string_basic_type<std::string>);
    ASSERT_TRUE(!argparse::is_parse_from_string_basic_type<std::vector<int>>);
}

// 测试 is_tuple_like_parse_from_split_string concept
TEST(ConceptTest, CanParseFromStringSplitOnce) {
    ASSERT_TRUE(
        (argparse::is_tuple_like_parse_from_split_string<std::pair<int, int>>));
    ASSERT_TRUE((argparse::is_tuple_like_parse_from_split_string<
                 std::tuple<int, std::string>>));
    ASSERT_TRUE(
        (argparse::is_tuple_like_parse_from_split_string<std::array<int, 8>>));
    ASSERT_TRUE(!argparse::is_tuple_like_parse_from_split_string<int>);
    ASSERT_TRUE(
        !argparse::is_tuple_like_parse_from_split_string<std::vector<int>>);
}

// 测试 can_parse_from_string concept
TEST(ConceptTest, CanParseFromString) {
    ASSERT_TRUE(argparse::can_parse_from_string<int>);
    ASSERT_TRUE(argparse::can_parse_from_string<std::string>);
    ASSERT_TRUE((argparse::can_parse_from_string<std::pair<int, double>>));
    ASSERT_TRUE(!argparse::can_parse_from_string<std::vector<int>>);
    ASSERT_TRUE(!argparse::can_parse_from_string<std::optional<int>>);
}

// 测试 is_container concept
TEST(ConceptTest, IsContainer) {
    ASSERT_TRUE(argparse::is_container<std::vector<int>>);
    ASSERT_TRUE(argparse::is_container<std::vector<std::string>>);
    ASSERT_TRUE((argparse::is_container<std::map<int, int>>));
    ASSERT_TRUE((argparse::is_container<std::unordered_map<int, int>>));
    ASSERT_TRUE(argparse::is_container<std::unordered_set<int>>);
    ASSERT_TRUE(argparse::is_container<std::set<int>>);
    ASSERT_TRUE(argparse::is_container<std::deque<int>>);
    ASSERT_TRUE(argparse::is_container<std::list<int>>);
    ASSERT_TRUE(!argparse::is_container<int>);
    ASSERT_TRUE(!argparse::is_container<std::string>);
    ASSERT_TRUE((!argparse::is_container<std::tuple<int, int>>));
}

// 测试 is_string concept
TEST(ConceptTest, IsString) {
    ASSERT_TRUE(argparse::is_string_v<std::string>);
    ASSERT_TRUE(!argparse::is_string_v<const char*>);
    ASSERT_TRUE(!argparse::is_string_v<int>);
    ASSERT_TRUE(!argparse::is_string_v<std::vector<char>>);
}

// 测试 is_optional concept
TEST(ConceptTest, IsOptional) {
    ASSERT_TRUE(argparse::is_optional_v<std::optional<int>>);
    ASSERT_TRUE(argparse::is_optional_v<std::optional<std::string>>);
    ASSERT_TRUE(!argparse::is_optional_v<int>);
    ASSERT_TRUE(!argparse::is_optional_v<std::string>);
}

// 测试组合场景
TEST(ConceptTest, CombinedScenarios) {
    ASSERT_TRUE((argparse::can_parse_from_string<
                 std::tuple<int, std::vector<std::string>>>));
    ASSERT_TRUE((argparse::can_parse_from_string<
                 std::pair<std::optional<int>, std::string>>));
    ASSERT_TRUE(
        !argparse::can_parse_from_string<std::vector<std::optional<int>>>);
    ASSERT_TRUE(
        !argparse::can_parse_from_string<std::optional<std::vector<int>>>);
}
