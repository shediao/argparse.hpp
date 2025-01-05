//
// Created by shediao on 24-12-29.
//

#include <gtest/gtest.h>

#include "argparse.hpp"

using namespace argparse;

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
    EXPECT_EQ(parse_from_string<long long>("123456789012345"),
              123456789012345LL);
    EXPECT_EQ(parse_from_string<long long>("-987654321098765"),
              -987654321098765LL);
    EXPECT_THROW(parse_from_string<long long>("12.34"), std::invalid_argument);
    EXPECT_THROW(parse_from_string<long long>("abc"), std::invalid_argument);
}

TEST(ParseFromStringTest, split) {
    EXPECT_EQ(split("k=v", '=', 2), (std::vector<std::string>{"k", "v"}));
    EXPECT_EQ(split("1,2,3", ',', 3),
              (std::vector<std::string>{"1", "2", "3"}));
    EXPECT_EQ(split("1,2,3", ',', 2), (std::vector<std::string>{"1", "2,3"}));
    EXPECT_EQ(split("1,2,3", ',', 4),
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
    EXPECT_EQ(
        (parse_from_string<std::pair<std::string, std::string>>("k=", '=')),
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
