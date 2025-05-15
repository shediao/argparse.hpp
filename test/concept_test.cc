#include <gtest/gtest.h>

#include <array>
#include <deque>
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

#include "argparse/argparse.hpp"

// 测试 ParseFromStringTupleLikeType concept
TEST(ConceptTest, IsTupleLike) {
    ASSERT_TRUE(
        (argparse::ParseFromStringTupleLikeType<std::tuple<int, std::string>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringTupleLikeType<std::pair<int, double>>));
    ASSERT_TRUE((argparse::ParseFromStringTupleLikeType<std::array<int, 3>>));
    ASSERT_TRUE(!argparse::ParseFromStringTupleLikeType<std::vector<int>>);
    ASSERT_TRUE(!argparse::ParseFromStringTupleLikeType<std::string>);
}

// 测试 ParseFromStringBasicType concept
TEST(ConceptTest, CanParseFromStringWithoutSplit) {
    ASSERT_TRUE(argparse::ParseFromStringBasicType<char>);
    ASSERT_TRUE(!argparse::ParseFromStringBasicType<wchar_t>);
    ASSERT_TRUE(!argparse::ParseFromStringBasicType<char16_t>);
    ASSERT_TRUE(!argparse::ParseFromStringBasicType<char32_t>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<bool>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<int>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<double>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<std::string>);
    ASSERT_TRUE(!argparse::ParseFromStringBasicType<std::vector<int>>);
}

// 测试 ParseFromStringTupleLikeType concept
TEST(ConceptTest, CanParseFromStringSplitOnce) {
    ASSERT_TRUE((argparse::ParseFromStringTupleLikeType<std::pair<int, int>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringTupleLikeType<std::tuple<int, std::string>>));
    ASSERT_TRUE((argparse::ParseFromStringTupleLikeType<std::array<int, 8>>));
    ASSERT_TRUE(!argparse::ParseFromStringTupleLikeType<int>);
    ASSERT_TRUE(!argparse::ParseFromStringTupleLikeType<std::vector<int>>);
}

// 测试 ParseFromStringType concept
TEST(ConceptTest, CanParseFromString) {
    ASSERT_TRUE(argparse::ParseFromStringType<int>);
    ASSERT_TRUE(argparse::ParseFromStringType<std::string>);
    ASSERT_TRUE((argparse::ParseFromStringType<std::pair<int, double>>));
    ASSERT_TRUE(argparse::ParseFromStringType<std::optional<int>>);
    ASSERT_TRUE(
        (argparse::ParseFromStringType<std::optional<std::pair<int, double>>>));
    ASSERT_TRUE((
        argparse::ParseFromStringType<std::optional<std::tuple<int, double>>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringType<std::optional<std::array<int, 3>>>));

    ASSERT_FALSE(
        argparse::ParseFromStringType<std::optional<std::vector<int>>>);
    ASSERT_FALSE(argparse::ParseFromStringType<std::vector<int>>);
}

// 测试 ParseFromStringContainerType concept
TEST(ConceptTest, IsContainer) {
    ASSERT_TRUE(argparse::ParseFromStringContainerType<std::vector<int>>);
    ASSERT_TRUE(
        argparse::ParseFromStringContainerType<std::vector<std::string>>);
    ASSERT_TRUE((argparse::ParseFromStringContainerType<std::map<int, int>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringContainerType<std::unordered_map<int, int>>));
    ASSERT_TRUE(
        argparse::ParseFromStringContainerType<std::unordered_set<int>>);
    ASSERT_TRUE(argparse::ParseFromStringContainerType<std::set<int>>);
    ASSERT_TRUE(argparse::ParseFromStringContainerType<std::deque<int>>);
    ASSERT_TRUE(argparse::ParseFromStringContainerType<std::list<int>>);
    ASSERT_TRUE(!argparse::ParseFromStringContainerType<int>);
    ASSERT_TRUE(!argparse::ParseFromStringContainerType<std::string>);
    ASSERT_TRUE(
        (!argparse::ParseFromStringContainerType<std::tuple<int, int>>));
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
    ASSERT_FALSE((argparse::ParseFromStringType<
                  std::tuple<int, std::vector<std::string>>>));
    ASSERT_FALSE((argparse::ParseFromStringType<
                  std::pair<std::optional<int>, std::string>>));
    ASSERT_TRUE(
        !argparse::ParseFromStringType<std::vector<std::optional<int>>>);
    ASSERT_TRUE(
        !argparse::ParseFromStringType<std::optional<std::vector<int>>>);
}

// 测试 ParseFromStringBasicType concept
TEST(ConceptTest, ParseFromStringBasicType) {
    // 基本类型测试
    ASSERT_TRUE(argparse::ParseFromStringBasicType<bool>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<char>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<int>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<long>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<unsigned long>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<long long>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<unsigned long long>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<float>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<double>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<long double>);

    // 字符串类型测试
    ASSERT_TRUE(argparse::ParseFromStringBasicType<std::string>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<std::wstring>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<std::u8string>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<std::u16string>);
    ASSERT_TRUE(argparse::ParseFromStringBasicType<std::u32string>);

    // 不支持的类型测试
    ASSERT_FALSE(argparse::ParseFromStringBasicType<unsigned char>);
    ASSERT_FALSE(argparse::ParseFromStringBasicType<short>);
    ASSERT_FALSE(argparse::ParseFromStringBasicType<unsigned short>);
    ASSERT_FALSE((argparse::ParseFromStringBasicType<std::vector<int>>));
    ASSERT_FALSE((argparse::ParseFromStringBasicType<std::pair<int, int>>));
    ASSERT_FALSE((argparse::ParseFromStringBasicType<std::tuple<int, int>>));
    ASSERT_FALSE((argparse::ParseFromStringBasicType<std::optional<int>>));
}

// 用于测试的自定义类型
class StringConstructible {
   public:
    explicit StringConstructible(const std::string&) {}
};

class StringConvertible {
   public:
    StringConvertible(const std::string&) {}
};

class NonStringConstructible {
   public:
    explicit NonStringConstructible(int) {}
};

// 测试 ParseFromStringCustomType concept
TEST(ConceptTest, ParseFromStringCustomType) {
    // 可以从字符串显式构造的自定义类型
    ASSERT_TRUE(argparse::ParseFromStringCustomType<StringConstructible>);

    // 可以从字符串隐式转换的自定义类型
    ASSERT_TRUE(argparse::ParseFromStringCustomType<StringConvertible>);

    // 基本类型应该返回 false（因为它们被 ParseFromStringBasicType 处理）
    ASSERT_FALSE(argparse::ParseFromStringCustomType<bool>);
    ASSERT_FALSE(argparse::ParseFromStringCustomType<int>);
    ASSERT_FALSE(argparse::ParseFromStringCustomType<double>);
    ASSERT_FALSE(argparse::ParseFromStringCustomType<std::string>);
    ASSERT_FALSE(argparse::ParseFromStringCustomType<std::wstring>);

    // 不支持的类型
    ASSERT_FALSE(argparse::ParseFromStringCustomType<NonStringConstructible>);
    ASSERT_FALSE((argparse::ParseFromStringCustomType<std::vector<int>>));
    ASSERT_FALSE((argparse::ParseFromStringCustomType<std::pair<int, int>>));
    ASSERT_FALSE((argparse::ParseFromStringCustomType<std::optional<int>>));
}

// 测试 ParseFromStringTupleLikeType concept
TEST(ConceptTest, ParseFromStringTupleLikeType) {
    // 基本元组类型测试
    ASSERT_TRUE((argparse::ParseFromStringTupleLikeType<std::tuple<int>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringTupleLikeType<std::tuple<int, std::string>>));
    ASSERT_TRUE((
        argparse::ParseFromStringTupleLikeType<std::tuple<bool, int, double>>));

    // pair 类型测试
    ASSERT_TRUE(
        (argparse::ParseFromStringTupleLikeType<std::pair<int, std::string>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringTupleLikeType<std::pair<double, bool>>));

    // array 类型测试
    ASSERT_TRUE((argparse::ParseFromStringTupleLikeType<std::array<int, 1>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringTupleLikeType<std::array<double, 3>>));

    // 包含自定义类型的元组测试
    ASSERT_TRUE((argparse::ParseFromStringTupleLikeType<
                 std::tuple<StringConstructible>>));
    ASSERT_TRUE((
        argparse::ParseFromStringTupleLikeType<std::tuple<StringConvertible>>));
    ASSERT_TRUE((argparse::ParseFromStringTupleLikeType<
                 std::pair<StringConstructible, StringConvertible>>));

    // 不支持的类型测试
    ASSERT_FALSE(argparse::ParseFromStringTupleLikeType<int>);
    ASSERT_FALSE(argparse::ParseFromStringTupleLikeType<std::string>);
    ASSERT_FALSE((argparse::ParseFromStringTupleLikeType<std::vector<int>>));
    ASSERT_FALSE((argparse::ParseFromStringTupleLikeType<
                  std::tuple<NonStringConstructible>>));
    ASSERT_FALSE(
        (argparse::ParseFromStringTupleLikeType<std::tuple<std::vector<int>>>));
    ASSERT_FALSE((argparse::ParseFromStringTupleLikeType<
                  std::pair<int, std::vector<int>>>));

    // 空元组类型（应该返回 false，因为要求至少有一个元素）
    // ASSERT_FALSE((argparse::ParseFromStringTupleLikeType<std::tuple<>>));

    // 嵌套元组类型测试
    ASSERT_FALSE(
        (argparse::ParseFromStringTupleLikeType<std::tuple<std::tuple<int>>>));
    ASSERT_FALSE((argparse::ParseFromStringTupleLikeType<
                  std::pair<std::pair<int, int>, int>>));
}

// 测试 ParseFromStringContainerType concept
TEST(ConceptTest, ParseFromStringContainerType) {
    // 基本类型容器测试
    ASSERT_TRUE((argparse::ParseFromStringContainerType<std::vector<int>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringContainerType<std::vector<std::string>>));
    ASSERT_TRUE((argparse::ParseFromStringContainerType<std::vector<double>>));
    ASSERT_TRUE((argparse::ParseFromStringContainerType<std::list<int>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringContainerType<std::deque<std::string>>));
    ASSERT_TRUE((argparse::ParseFromStringContainerType<std::set<int>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringContainerType<std::unordered_set<double>>));

    // 自定义类型容器测试
    ASSERT_TRUE((argparse::ParseFromStringContainerType<
                 std::vector<StringConstructible>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringContainerType<std::list<StringConvertible>>));
    ASSERT_TRUE((
        argparse::ParseFromStringContainerType<std::set<StringConstructible>>));

    // 元组类型容器测试
    ASSERT_TRUE((argparse::ParseFromStringContainerType<
                 std::vector<std::pair<int, int>>>));
    ASSERT_TRUE((argparse::ParseFromStringContainerType<
                 std::vector<std::tuple<int, std::string>>>));
    ASSERT_TRUE((
        argparse::ParseFromStringContainerType<std::list<std::array<int, 2>>>));

    // map类型测试（应该返回true，因为map的value_type是pair，且pair是支持的类型）
    ASSERT_TRUE((argparse::ParseFromStringContainerType<std::map<int, int>>));
    ASSERT_TRUE((argparse::ParseFromStringContainerType<
                 std::unordered_map<std::string, int>>));

    // 不支持的类型测试
    ASSERT_FALSE(argparse::ParseFromStringContainerType<int>);
    ASSERT_FALSE(argparse::ParseFromStringContainerType<std::string>);
    ASSERT_FALSE((argparse::ParseFromStringContainerType<std::pair<int, int>>));
    ASSERT_FALSE(
        (argparse::ParseFromStringContainerType<std::tuple<int, int>>));
    ASSERT_FALSE((argparse::ParseFromStringContainerType<std::array<int, 3>>));

    // 不支持的元素类型容器测试
    ASSERT_FALSE((argparse::ParseFromStringContainerType<
                  std::vector<NonStringConstructible>>));
    ASSERT_FALSE((
        argparse::ParseFromStringContainerType<std::vector<std::vector<int>>>));
    ASSERT_FALSE((argparse::ParseFromStringContainerType<
                  std::vector<std::tuple<std::tuple<int>>>>));
}

// 测试 ParseFromStringOptionalSingleType concept
TEST(ConceptTest, ParseFromStringOptionalSingleType) {
    // 基本类型的 optional 测试
    ASSERT_TRUE(
        (argparse::ParseFromStringOptionalSingleType<std::optional<int>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringOptionalSingleType<std::optional<double>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringOptionalSingleType<std::optional<bool>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalSingleType<
                 std::optional<std::string>>));

    // 自定义类型的 optional 测试
    ASSERT_TRUE((argparse::ParseFromStringOptionalSingleType<
                 std::optional<StringConstructible>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalSingleType<
                 std::optional<StringConvertible>>));

    // 不支持的类型测试
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<int>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<std::string>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<
                  std::optional<NonStringConstructible>>));

    // 不支持的复合类型的 optional 测试
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<
                  std::optional<std::vector<int>>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<
                  std::optional<std::pair<int, int>>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<
                  std::optional<std::tuple<int>>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<
                  std::optional<std::array<int, 1>>>));

    // 嵌套 optional 测试
    ASSERT_FALSE((argparse::ParseFromStringOptionalSingleType<
                  std::optional<std::optional<int>>>));

    // const 类型测试
    ASSERT_TRUE((
        argparse::ParseFromStringOptionalSingleType<std::optional<const int>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalSingleType<
                 std::optional<const std::string>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalSingleType<
                 std::optional<const StringConstructible>>));
}

// 测试 ParseFromStringOptionalTupleLikeType concept
TEST(ConceptTest, ParseFromStringOptionalTupleLikeType) {
    // 基本元组类型的 optional 测试
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::tuple<int>>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::tuple<int, std::string>>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::tuple<bool, int, double>>>));

    // pair 类型的 optional 测试
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::pair<int, std::string>>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::pair<double, bool>>>));

    // array 类型的 optional 测试
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::array<int, 1>>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::array<double, 3>>>));

    // 包含自定义类型的元组的 optional 测试
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::tuple<StringConstructible>>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::tuple<StringConvertible>>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringOptionalTupleLikeType<
            std::optional<std::pair<StringConstructible, StringConvertible>>>));

    // 不支持的类型测试
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<int>));
    ASSERT_FALSE(
        (argparse::ParseFromStringOptionalTupleLikeType<std::optional<int>>));
    ASSERT_FALSE(
        (argparse::ParseFromStringOptionalTupleLikeType<std::tuple<int>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<
                  std::optional<std::string>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<
                  std::optional<std::vector<int>>>));

    // 不支持的元素类型测试
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<
                  std::optional<std::tuple<NonStringConstructible>>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<
                  std::optional<std::tuple<std::vector<int>>>>));

    // 嵌套类型测试
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<
                  std::optional<std::tuple<std::tuple<int>>>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<
                  std::optional<std::pair<std::pair<int, int>, int>>>));
    ASSERT_FALSE((argparse::ParseFromStringOptionalTupleLikeType<
                  std::optional<std::optional<std::tuple<int>>>>));

    // const 类型测试
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<const std::tuple<int>>>));
    ASSERT_TRUE((argparse::ParseFromStringOptionalTupleLikeType<
                 std::optional<std::tuple<const int>>>));
    ASSERT_TRUE(
        (argparse::ParseFromStringOptionalTupleLikeType<
            std::optional<const std::pair<const int, const std::string>>>));
}

// 测试 BindableType concept
TEST(ConceptTest, BindableType) {
    // 基本类型测试
    ASSERT_TRUE(argparse::BindableType<int>);
    ASSERT_TRUE(argparse::BindableType<std::string>);
    ASSERT_TRUE(argparse::BindableType<bool>);
    ASSERT_TRUE(argparse::BindableType<double>);

    // 自定义类型测试
    ASSERT_TRUE(argparse::BindableType<StringConstructible>);
    ASSERT_TRUE(argparse::BindableType<StringConvertible>);

    // optional类型测试
    ASSERT_TRUE((argparse::BindableType<std::optional<int>>));
    ASSERT_TRUE((argparse::BindableType<std::optional<std::string>>));
    ASSERT_TRUE((argparse::BindableType<std::optional<StringConstructible>>));

    // 元组类型测试
    ASSERT_TRUE((argparse::BindableType<std::pair<int, int>>));
    ASSERT_TRUE((argparse::BindableType<std::tuple<int, std::string>>));
    ASSERT_TRUE((argparse::BindableType<std::array<int, 3>>));

    // 容器类型测试
    ASSERT_TRUE((argparse::BindableType<std::vector<int>>));
    ASSERT_TRUE((argparse::BindableType<std::list<std::string>>));
    ASSERT_TRUE((argparse::BindableType<std::set<double>>));
    ASSERT_TRUE((argparse::BindableType<std::vector<std::pair<int, int>>>));

    // 不支持的类型测试
    ASSERT_FALSE(argparse::BindableType<NonStringConstructible>);
    ASSERT_FALSE((argparse::BindableType<std::vector<NonStringConstructible>>));
    ASSERT_FALSE((argparse::BindableType<std::vector<std::vector<int>>>));
}

// 测试 BindableWithoutDelimiterType concept
TEST(ConceptTest, BindableWithoutDelimiterType) {
    // 基本类型测试
    ASSERT_TRUE(argparse::BindableWithoutDelimiterType<int>);
    ASSERT_TRUE(argparse::BindableWithoutDelimiterType<std::string>);
    ASSERT_TRUE(argparse::BindableWithoutDelimiterType<bool>);
    ASSERT_TRUE(argparse::BindableWithoutDelimiterType<double>);

    // 自定义类型测试
    ASSERT_TRUE(argparse::BindableWithoutDelimiterType<StringConstructible>);
    ASSERT_TRUE(argparse::BindableWithoutDelimiterType<StringConvertible>);

    // optional基本类型测试
    ASSERT_TRUE((argparse::BindableWithoutDelimiterType<std::optional<int>>));
    ASSERT_TRUE(
        (argparse::BindableWithoutDelimiterType<std::optional<std::string>>));
    ASSERT_TRUE((argparse::BindableWithoutDelimiterType<
                 std::optional<StringConstructible>>));

    // 基本类型的容器测试
    ASSERT_TRUE((argparse::BindableWithoutDelimiterType<std::vector<int>>));
    ASSERT_TRUE(
        (argparse::BindableWithoutDelimiterType<std::list<std::string>>));
    ASSERT_TRUE((argparse::BindableWithoutDelimiterType<std::set<double>>));

    // 不支持的类型测试
    ASSERT_FALSE((argparse::BindableWithoutDelimiterType<std::pair<int, int>>));
    ASSERT_FALSE(
        (argparse::BindableWithoutDelimiterType<std::tuple<int, std::string>>));
    ASSERT_FALSE((argparse::BindableWithoutDelimiterType<std::array<int, 3>>));
    ASSERT_FALSE((argparse::BindableWithoutDelimiterType<
                  std::vector<std::pair<int, int>>>));
    ASSERT_FALSE(
        argparse::BindableWithoutDelimiterType<NonStringConstructible>);
    ASSERT_FALSE((argparse::BindableWithoutDelimiterType<
                  std::optional<std::pair<int, int>>>));
}

// 测试 BindableWithDelimiterType concept
TEST(ConceptTest, BindableWithDelimiterType) {
    // 元组类型测试
    ASSERT_TRUE((argparse::BindableWithDelimiterType<std::pair<int, int>>));
    ASSERT_TRUE(
        (argparse::BindableWithDelimiterType<std::tuple<int, std::string>>));
    ASSERT_TRUE((argparse::BindableWithDelimiterType<std::array<int, 3>>));

    // optional元组类型测试
    ASSERT_TRUE((argparse::BindableWithDelimiterType<
                 std::optional<std::pair<int, int>>>));
    ASSERT_TRUE((argparse::BindableWithDelimiterType<
                 std::optional<std::tuple<int, std::string>>>));
    ASSERT_TRUE((argparse::BindableWithDelimiterType<
                 std::optional<std::array<int, 3>>>));

    // 元组容器类型测试
    ASSERT_TRUE((
        argparse::BindableWithDelimiterType<std::vector<std::pair<int, int>>>));
    ASSERT_TRUE((argparse::BindableWithDelimiterType<
                 std::list<std::tuple<int, std::string>>>));
    ASSERT_TRUE(
        (argparse::BindableWithDelimiterType<std::vector<std::array<int, 2>>>));

    // 不支持的类型测试
    ASSERT_FALSE(argparse::BindableWithDelimiterType<int>);
    ASSERT_FALSE(argparse::BindableWithDelimiterType<std::string>);
    ASSERT_FALSE((argparse::BindableWithDelimiterType<std::vector<int>>));
    ASSERT_FALSE((argparse::BindableWithDelimiterType<std::optional<int>>));
    ASSERT_FALSE(argparse::BindableWithDelimiterType<StringConstructible>);
    ASSERT_FALSE(
        (argparse::BindableWithDelimiterType<std::vector<std::vector<int>>>));
    ASSERT_FALSE(
        (argparse::BindableWithDelimiterType<std::tuple<std::tuple<int>>>));
}
