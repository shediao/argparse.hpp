//
// Created by shediao on 24-12-29.
//

#include <gtest/gtest.h>

#include "argparser.hpp"

using namespace arg::parser;
using namespace arg::parser::detail;
// 测试 bool
TEST(ParseFromStringTest, BoolTest) {
    EXPECT_EQ(parse_from_string<bool>("true"), true);
    EXPECT_EQ(parse_from_string<bool>("false"), false);
    EXPECT_EQ(parse_from_string<bool>("1"), true);
    EXPECT_EQ(parse_from_string<bool>("0"), false);

    EXPECT_THROW(parse_from_string<bool>("invalid"), std::invalid_argument);
}

// 测试 int
TEST(ParseFromStringTest, IntTest) {
    EXPECT_EQ(parse_from_string<int>("123"), 123);
    EXPECT_EQ(parse_from_string<int>("-456"), -456);
    EXPECT_EQ(parse_from_string<int>("0"), 0);

    EXPECT_THROW(parse_from_string<int>("123a"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<int>("invalid"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<int>(
                     std::to_string(std::numeric_limits<long long>::max())),
                 std::out_of_range);
}

// 测试 long
TEST(ParseFromStringTest, LongTest) {
    EXPECT_EQ(parse_from_string<long>("1234567890"), 1234567890L);
    EXPECT_EQ(parse_from_string<long>("-9876543210"), -9876543210L);
    EXPECT_EQ(parse_from_string<long>("0"), 0L);

    EXPECT_THROW(parse_from_string<long>("123a"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<long>("invalid"), std::invalid_argument);
    if constexpr (sizeof(long) != sizeof(long long)) {
        EXPECT_THROW(parse_from_string<long>(
                         std::to_string(std::numeric_limits<long long>::max())),
                     std::out_of_range);
    }
}

// 测试 double
TEST(ParseFromStringTest, DoubleTest) {
    EXPECT_DOUBLE_EQ(parse_from_string<double>("3.14"), 3.14);
    EXPECT_DOUBLE_EQ(parse_from_string<double>("-2.71"), -2.71);
    EXPECT_DOUBLE_EQ(parse_from_string<double>("0.0"), 0.0);
    EXPECT_DOUBLE_EQ(parse_from_string<double>("1e5"), 1e5);
    EXPECT_DOUBLE_EQ(parse_from_string<double>("-1e-5"), -1e-5);

    EXPECT_THROW(parse_from_string<double>("123a"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<double>("invalid"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<double>(
                     "1" + std::to_string(std::numeric_limits<double>::max())),
                 std::out_of_range);
}

// 测试 long long
TEST(ParseFromStringTest, LongLongTest) {
    EXPECT_EQ(parse_from_string<long long>("123456789012345"), 123456789012345LL);
    EXPECT_EQ(parse_from_string<long long>("-987654321098765"),
              -987654321098765LL);
    EXPECT_EQ(parse_from_string<long long>("0"), 0LL);

    EXPECT_THROW(parse_from_string<long long>("123a"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<long long>("invalid"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<long long>(
                     std::to_string(std::numeric_limits<long double>::max())),
                 std::out_of_range);
}

TEST(ParseFromStringTest, Split) {
    EXPECT_EQ(Split("k=v", '=', 2), (std::vector<std::string>{"k", "v"}));
    EXPECT_EQ(Split("1,2,3", ',', 3),
              (std::vector<std::string>{"1", "2", "3"}));
    EXPECT_EQ(Split("1,2,3", ',', 2), (std::vector<std::string>{"1", "2,3"}));
    EXPECT_EQ(Split("1,2,3", ',', 4),
              (std::vector<std::string>{"1", "2", "3"}));
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
