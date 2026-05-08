#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "argparse/argparse.hpp"

TEST(ConceptTest, IsIntegral) {
  ASSERT_FALSE(argparse::detail::is_integral_v<bool>);
  ASSERT_FALSE(argparse::detail::is_integral_v<std::string>);
  ASSERT_FALSE(argparse::detail::is_integral_v<char*>);
  ASSERT_TRUE(argparse::detail::is_integral_v<int>);
  ASSERT_TRUE(argparse::detail::is_integral_v<short>);
  ASSERT_TRUE(argparse::detail::is_integral_v<long>);
  ASSERT_TRUE(argparse::detail::is_integral_v<long long>);
  ASSERT_TRUE(argparse::detail::is_integral_v<unsigned int>);
  ASSERT_TRUE(argparse::detail::is_integral_v<unsigned short>);
  ASSERT_TRUE(argparse::detail::is_integral_v<unsigned long>);
  ASSERT_TRUE(argparse::detail::is_integral_v<unsigned long long>);
}
// 测试 ParseFromStringTupleLikeType concept
TEST(ConceptTest, IsTupleLike) {
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::tuple<int, std::string>>));
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringTupleLikeType<std::pair<int, double>>));
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringTupleLikeType<std::array<int, 3>>));
  ASSERT_TRUE(
      !argparse::detail::ParseFromStringTupleLikeType<std::vector<int>>);
  ASSERT_TRUE(!argparse::detail::ParseFromStringTupleLikeType<std::string>);
}

// 测试 ParseFromStringBasicType concept
TEST(ConceptTest, CanParseFromStringWithoutSplit) {
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<char>);
  ASSERT_TRUE(!argparse::detail::ParseFromStringBasicType<wchar_t>);
  ASSERT_TRUE(!argparse::detail::ParseFromStringBasicType<char16_t>);
  ASSERT_TRUE(!argparse::detail::ParseFromStringBasicType<char32_t>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<bool>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<int>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<double>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<std::string>);
  ASSERT_TRUE(!argparse::detail::ParseFromStringBasicType<std::vector<int>>);
}

// 测试 ParseFromStringTupleLikeType concept
TEST(ConceptTest, CanParseFromStringSplitOnce) {
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringTupleLikeType<std::pair<int, int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::tuple<int, std::string>>));
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringTupleLikeType<std::array<int, 8>>));
  ASSERT_TRUE(!argparse::detail::ParseFromStringTupleLikeType<int>);
  ASSERT_TRUE(
      !argparse::detail::ParseFromStringTupleLikeType<std::vector<int>>);
}

// 测试 ParseFromStringType concept
TEST(ConceptTest, CanParseFromString) {
  ASSERT_TRUE(argparse::detail::ParseFromStringType<int>);
  ASSERT_TRUE(argparse::detail::ParseFromStringType<std::string>);
  ASSERT_TRUE((argparse::detail::ParseFromStringType<std::pair<int, double>>));
  ASSERT_TRUE(argparse::detail::ParseFromStringType<std::optional<int>>);
  ASSERT_TRUE((argparse::detail::ParseFromStringType<
               std::optional<std::pair<int, double>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringType<
               std::optional<std::tuple<int, double>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringType<
               std::optional<std::array<int, 3>>>));

  ASSERT_FALSE(
      argparse::detail::ParseFromStringType<std::optional<std::vector<int>>>);
  ASSERT_FALSE(argparse::detail::ParseFromStringType<std::vector<int>>);
}

// 测试 ParseFromStringContainerType concept
TEST(ConceptTest, IsContainer) {
  ASSERT_TRUE(argparse::detail::ParseFromStringContainerType<std::vector<int>>);
  ASSERT_TRUE(
      argparse::detail::ParseFromStringContainerType<std::vector<std::string>>);
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringContainerType<std::map<int, int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::unordered_map<int, int>>));
  ASSERT_TRUE(
      argparse::detail::ParseFromStringContainerType<std::unordered_set<int>>);
  ASSERT_TRUE(argparse::detail::ParseFromStringContainerType<std::set<int>>);
  ASSERT_TRUE(argparse::detail::ParseFromStringContainerType<std::deque<int>>);
  ASSERT_TRUE(argparse::detail::ParseFromStringContainerType<std::list<int>>);
  ASSERT_TRUE(!argparse::detail::ParseFromStringContainerType<int>);
  ASSERT_TRUE(!argparse::detail::ParseFromStringContainerType<std::string>);
  ASSERT_TRUE(
      (!argparse::detail::ParseFromStringContainerType<std::tuple<int, int>>));
}

// 测试 is_string concept
TEST(ConceptTest, IsString) {
  ASSERT_TRUE(argparse::detail::is_string_v<std::string>);
  ASSERT_TRUE(!argparse::detail::is_string_v<const char*>);
  ASSERT_TRUE(!argparse::detail::is_string_v<int>);
  ASSERT_TRUE(!argparse::detail::is_string_v<std::vector<char>>);
}

// 测试 is_optional concept
TEST(ConceptTest, IsOptional) {
  ASSERT_TRUE(argparse::detail::is_optional_v<std::optional<int>>);
  ASSERT_TRUE(argparse::detail::is_optional_v<std::optional<std::string>>);
  ASSERT_TRUE(!argparse::detail::is_optional_v<int>);
  ASSERT_TRUE(!argparse::detail::is_optional_v<std::string>);
}

// 测试组合场景
TEST(ConceptTest, CombinedScenarios) {
  ASSERT_FALSE((argparse::detail::ParseFromStringType<
                std::tuple<int, std::vector<std::string>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringType<
                std::pair<std::optional<int>, std::string>>));
  ASSERT_TRUE(
      !argparse::detail::ParseFromStringType<std::vector<std::optional<int>>>);
  ASSERT_TRUE(
      !argparse::detail::ParseFromStringType<std::optional<std::vector<int>>>);
}

// 测试 ParseFromStringBasicType concept
TEST(ConceptTest, ParseFromStringBasicType) {
  // 基本类型测试
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<bool>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<char>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<int>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<long>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<unsigned long>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<long long>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<unsigned long long>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<float>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<double>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<long double>);

  // 字符串类型测试
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<std::string>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<std::wstring>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<std::u8string>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<std::u16string>);
  ASSERT_TRUE(argparse::detail::ParseFromStringBasicType<std::u32string>);

  // 不支持的类型测试
  ASSERT_FALSE(argparse::detail::ParseFromStringBasicType<unsigned char>);
  ASSERT_FALSE(argparse::detail::ParseFromStringBasicType<short>);
  ASSERT_FALSE(argparse::detail::ParseFromStringBasicType<unsigned short>);
  ASSERT_FALSE((argparse::detail::ParseFromStringBasicType<std::vector<int>>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringBasicType<std::pair<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringBasicType<std::tuple<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringBasicType<std::optional<int>>));
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
  ASSERT_TRUE(argparse::detail::ParseFromStringCustomType<StringConstructible>);

  // 可以从字符串隐式转换的自定义类型
  ASSERT_TRUE(argparse::detail::ParseFromStringCustomType<StringConvertible>);

  // 基本类型应该返回 false（因为它们被 ParseFromStringBasicType 处理）
  ASSERT_FALSE(argparse::detail::ParseFromStringCustomType<bool>);
  ASSERT_FALSE(argparse::detail::ParseFromStringCustomType<int>);
  ASSERT_FALSE(argparse::detail::ParseFromStringCustomType<double>);
  ASSERT_FALSE(argparse::detail::ParseFromStringCustomType<std::string>);
  ASSERT_FALSE(argparse::detail::ParseFromStringCustomType<std::wstring>);

  // 不支持的类型
  ASSERT_FALSE(
      argparse::detail::ParseFromStringCustomType<NonStringConstructible>);
  ASSERT_FALSE((argparse::detail::ParseFromStringCustomType<std::vector<int>>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringCustomType<std::pair<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringCustomType<std::optional<int>>));
}

// 测试 ParseFromStringTupleLikeType concept
TEST(ConceptTest, ParseFromStringTupleLikeType) {
  // 基本元组类型测试
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringTupleLikeType<std::tuple<int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::tuple<int, std::string>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::tuple<bool, int, double>>));

  // pair 类型测试
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::pair<int, std::string>>));
  ASSERT_TRUE((
      argparse::detail::ParseFromStringTupleLikeType<std::pair<double, bool>>));

  // array 类型测试
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringTupleLikeType<std::array<int, 1>>));
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringTupleLikeType<std::array<double, 3>>));

  // 包含自定义类型的元组测试
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::tuple<StringConstructible>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::tuple<StringConvertible>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringTupleLikeType<
               std::pair<StringConstructible, StringConvertible>>));

  // 不支持的类型测试
  ASSERT_FALSE(argparse::detail::ParseFromStringTupleLikeType<int>);
  ASSERT_FALSE(argparse::detail::ParseFromStringTupleLikeType<std::string>);
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringTupleLikeType<std::vector<int>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringTupleLikeType<
                std::tuple<NonStringConstructible>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringTupleLikeType<
                std::tuple<std::vector<int>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringTupleLikeType<
                std::pair<int, std::vector<int>>>));

  // 空元组类型（应该返回 false，因为要求至少有一个元素）
  // ASSERT_FALSE((argparse::detail::ParseFromStringTupleLikeType<std::tuple<>>));

  // 嵌套元组类型测试
  ASSERT_FALSE((argparse::detail::ParseFromStringTupleLikeType<
                std::tuple<std::tuple<int>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringTupleLikeType<
                std::pair<std::pair<int, int>, int>>));
}

// 测试 ParseFromStringContainerType concept
TEST(ConceptTest, ParseFromStringContainerType) {
  // 基本类型容器测试
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringContainerType<std::vector<int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::vector<std::string>>));
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringContainerType<std::vector<double>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<std::list<int>>));
  ASSERT_TRUE((
      argparse::detail::ParseFromStringContainerType<std::deque<std::string>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<std::set<int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::unordered_set<double>>));

  // 自定义类型容器测试
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::vector<StringConstructible>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::list<StringConvertible>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::set<StringConstructible>>));

  // 元组类型容器测试
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::vector<std::pair<int, int>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::vector<std::tuple<int, std::string>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::list<std::array<int, 2>>>));

  // map类型测试（应该返回true，因为map的value_type是pair，且pair是支持的类型）
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringContainerType<std::map<int, int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringContainerType<
               std::unordered_map<std::string, int>>));

  // 不支持的类型测试
  ASSERT_FALSE(argparse::detail::ParseFromStringContainerType<int>);
  ASSERT_FALSE(argparse::detail::ParseFromStringContainerType<std::string>);
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringContainerType<std::pair<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringContainerType<std::tuple<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringContainerType<std::array<int, 3>>));

  // 不支持的元素类型容器测试
  ASSERT_FALSE((argparse::detail::ParseFromStringContainerType<
                std::vector<NonStringConstructible>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringContainerType<
                std::vector<std::vector<int>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringContainerType<
                std::vector<std::tuple<std::tuple<int>>>>));
}

// 测试 ParseFromStringOptionalSingleType concept
TEST(ConceptTest, ParseFromStringOptionalSingleType) {
  // 基本类型的 optional 测试
  ASSERT_TRUE((
      argparse::detail::ParseFromStringOptionalSingleType<std::optional<int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<double>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<bool>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<std::string>>));

  // 自定义类型的 optional 测试
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<StringConstructible>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<StringConvertible>>));

  // 不支持的类型测试
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalSingleType<int>));
  ASSERT_FALSE(
      (argparse::detail::ParseFromStringOptionalSingleType<std::string>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalSingleType<
                std::optional<NonStringConstructible>>));

  // 不支持的复合类型的 optional 测试
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalSingleType<
                std::optional<std::vector<int>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalSingleType<
                std::optional<std::pair<int, int>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalSingleType<
                std::optional<std::tuple<int>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalSingleType<
                std::optional<std::array<int, 1>>>));

  // 嵌套 optional 测试
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalSingleType<
                std::optional<std::optional<int>>>));

  // const 类型测试
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<const int>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<const std::string>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalSingleType<
               std::optional<const StringConstructible>>));
}

// 测试 ParseFromStringOptionalTupleLikeType concept
TEST(ConceptTest, ParseFromStringOptionalTupleLikeType) {
  // 基本元组类型的 optional 测试
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::tuple<int>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::tuple<int, std::string>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::tuple<bool, int, double>>>));

  // pair 类型的 optional 测试
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::pair<int, std::string>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::pair<double, bool>>>));

  // array 类型的 optional 测试
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::array<int, 1>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::array<double, 3>>>));

  // 包含自定义类型的元组的 optional 测试
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::tuple<StringConstructible>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::tuple<StringConvertible>>>));
  ASSERT_TRUE(
      (argparse::detail::ParseFromStringOptionalTupleLikeType<
          std::optional<std::pair<StringConstructible, StringConvertible>>>));

  // 不支持的类型测试
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<int>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<int>>));
  ASSERT_FALSE((
      argparse::detail::ParseFromStringOptionalTupleLikeType<std::tuple<int>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<std::string>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<std::vector<int>>>));

  // 不支持的元素类型测试
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<std::tuple<NonStringConstructible>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<std::tuple<std::vector<int>>>>));

  // 嵌套类型测试
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<std::tuple<std::tuple<int>>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<std::pair<std::pair<int, int>, int>>>));
  ASSERT_FALSE((argparse::detail::ParseFromStringOptionalTupleLikeType<
                std::optional<std::optional<std::tuple<int>>>>));

  // const 类型测试
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<const std::tuple<int>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<std::tuple<const int>>>));
  ASSERT_TRUE((argparse::detail::ParseFromStringOptionalTupleLikeType<
               std::optional<const std::pair<const int, const std::string>>>));
}

// 测试 BindableType concept
TEST(ConceptTest, BindableType) {
  // 基本类型测试
  ASSERT_TRUE(argparse::detail::BindableType<int>);
  ASSERT_TRUE(argparse::detail::BindableType<std::string>);
  ASSERT_TRUE(argparse::detail::BindableType<bool>);
  ASSERT_TRUE(argparse::detail::BindableType<double>);

  // 自定义类型测试
  ASSERT_TRUE(argparse::detail::BindableType<StringConstructible>);
  ASSERT_TRUE(argparse::detail::BindableType<StringConvertible>);

  // optional类型测试
  ASSERT_TRUE((argparse::detail::BindableType<std::optional<int>>));
  ASSERT_TRUE((argparse::detail::BindableType<std::optional<std::string>>));
  ASSERT_TRUE(
      (argparse::detail::BindableType<std::optional<StringConstructible>>));

  // 元组类型测试
  ASSERT_TRUE((argparse::detail::BindableType<std::pair<int, int>>));
  ASSERT_TRUE((argparse::detail::BindableType<std::tuple<int, std::string>>));
  ASSERT_TRUE((argparse::detail::BindableType<std::array<int, 3>>));

  // 容器类型测试
  ASSERT_TRUE((argparse::detail::BindableType<std::vector<int>>));
  ASSERT_TRUE((argparse::detail::BindableType<std::list<std::string>>));
  ASSERT_TRUE((argparse::detail::BindableType<std::set<double>>));
  ASSERT_TRUE(
      (argparse::detail::BindableType<std::vector<std::pair<int, int>>>));

  // 不支持的类型测试
  ASSERT_FALSE(argparse::detail::BindableType<NonStringConstructible>);
  ASSERT_FALSE(
      (argparse::detail::BindableType<std::vector<NonStringConstructible>>));
  ASSERT_FALSE((argparse::detail::BindableType<std::vector<std::vector<int>>>));
}

// 测试 BindableWithoutDelimiterType concept
TEST(ConceptTest, BindableWithoutDelimiterType) {
  // 基本类型测试
  ASSERT_TRUE(argparse::detail::BindableWithoutDelimiterType<int>);
  ASSERT_TRUE(argparse::detail::BindableWithoutDelimiterType<std::string>);
  ASSERT_TRUE(argparse::detail::BindableWithoutDelimiterType<bool>);
  ASSERT_TRUE(argparse::detail::BindableWithoutDelimiterType<double>);

  // 自定义类型测试
  ASSERT_TRUE(
      argparse::detail::BindableWithoutDelimiterType<StringConstructible>);
  ASSERT_TRUE(
      argparse::detail::BindableWithoutDelimiterType<StringConvertible>);

  // optional基本类型测试
  ASSERT_TRUE(
      (argparse::detail::BindableWithoutDelimiterType<std::optional<int>>));
  ASSERT_TRUE((argparse::detail::BindableWithoutDelimiterType<
               std::optional<std::string>>));
  ASSERT_TRUE((argparse::detail::BindableWithoutDelimiterType<
               std::optional<StringConstructible>>));

  // 基本类型的容器测试
  ASSERT_TRUE(
      (argparse::detail::BindableWithoutDelimiterType<std::vector<int>>));
  ASSERT_TRUE(
      (argparse::detail::BindableWithoutDelimiterType<std::list<std::string>>));
  ASSERT_TRUE(
      (argparse::detail::BindableWithoutDelimiterType<std::set<double>>));

  // 不支持的类型测试
  ASSERT_FALSE(
      (argparse::detail::BindableWithoutDelimiterType<std::pair<int, int>>));
  ASSERT_FALSE((argparse::detail::BindableWithoutDelimiterType<
                std::tuple<int, std::string>>));
  ASSERT_FALSE(
      (argparse::detail::BindableWithoutDelimiterType<std::array<int, 3>>));
  ASSERT_FALSE((argparse::detail::BindableWithoutDelimiterType<
                std::vector<std::pair<int, int>>>));
  ASSERT_FALSE(
      argparse::detail::BindableWithoutDelimiterType<NonStringConstructible>);
  ASSERT_FALSE((argparse::detail::BindableWithoutDelimiterType<
                std::optional<std::pair<int, int>>>));
}

// 测试 BindableWithDelimiterType concept
TEST(ConceptTest, BindableWithDelimiterType) {
  // 元组类型测试
  ASSERT_TRUE(
      (argparse::detail::BindableWithDelimiterType<std::pair<int, int>>));
  ASSERT_TRUE((argparse::detail::BindableWithDelimiterType<
               std::tuple<int, std::string>>));
  ASSERT_TRUE(
      (argparse::detail::BindableWithDelimiterType<std::array<int, 3>>));

  // optional元组类型测试
  ASSERT_TRUE((argparse::detail::BindableWithDelimiterType<
               std::optional<std::pair<int, int>>>));
  ASSERT_TRUE((argparse::detail::BindableWithDelimiterType<
               std::optional<std::tuple<int, std::string>>>));
  ASSERT_TRUE((argparse::detail::BindableWithDelimiterType<
               std::optional<std::array<int, 3>>>));

  // 元组容器类型测试
  ASSERT_TRUE((argparse::detail::BindableWithDelimiterType<
               std::vector<std::pair<int, int>>>));
  ASSERT_TRUE((argparse::detail::BindableWithDelimiterType<
               std::list<std::tuple<int, std::string>>>));
  ASSERT_TRUE((argparse::detail::BindableWithDelimiterType<
               std::vector<std::array<int, 2>>>));

  // 不支持的类型测试
  ASSERT_FALSE(argparse::detail::BindableWithDelimiterType<int>);
  ASSERT_FALSE(argparse::detail::BindableWithDelimiterType<std::string>);
  ASSERT_FALSE((argparse::detail::BindableWithDelimiterType<std::vector<int>>));
  ASSERT_FALSE(
      (argparse::detail::BindableWithDelimiterType<std::optional<int>>));
  ASSERT_FALSE(
      argparse::detail::BindableWithDelimiterType<StringConstructible>);
  ASSERT_FALSE((argparse::detail::BindableWithDelimiterType<
                std::vector<std::vector<int>>>));
  ASSERT_FALSE((argparse::detail::BindableWithDelimiterType<
                std::tuple<std::tuple<int>>>));
}

// ============================================================
// Tests for has_to_string / has_to_wstring traits
// ============================================================

// Custom type with a free to_string in its own namespace (ADL)
namespace custom_ns {
struct WithToString {};
inline std::string to_string(WithToString const&) { return "custom"; }

struct WithToWstring {};
inline std::wstring to_wstring(WithToWstring const&) { return L"custom"; }

struct WithBoth {};
inline std::string to_string(WithBoth const&) { return "both"; }
inline std::wstring to_wstring(WithBoth const&) { return L"both"; }

// A type with to_string that returns something not convertible to std::string
struct BadToString {};
inline int to_string(BadToString const&) { return 42; }

struct BadToWstring {};
inline int to_wstring(BadToWstring const&) { return 42; }
}  // namespace custom_ns

// 测试 has_to_string trait
TEST(ConceptTest, HasToString) {
  // std::to_string 支持的标准类型
  ASSERT_TRUE(argparse::detail::has_to_string_v<int>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<long>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<long long>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<unsigned>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<unsigned long>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<unsigned long long>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<float>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<double>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<long double>);

  // std::string（通过 argparse 中的 identity 重载）
  ASSERT_TRUE(argparse::detail::has_to_string_v<std::string>);

  // 通过 ADL 找到的自定义类型
  ASSERT_TRUE(argparse::detail::has_to_string_v<custom_ns::WithToString>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<custom_ns::WithBoth>);

  // bool / char / short 等可通过整型提升隐式转换为 int
  ASSERT_TRUE(argparse::detail::has_to_string_v<bool>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<char>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<short>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<unsigned short>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<unsigned char>);

  // wchar_t / char16_t / char32_t 的整型提升结果因平台而异，
  // 但通常也能落到 std::to_string 的某个重载上
  // （此处仅验证 trait 行为一致，结果可为 true 或 false）

  // std::to_string 不支持的类型
  ASSERT_FALSE(argparse::detail::has_to_string_v<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_to_string_v<std::vector<int>>);
  ASSERT_FALSE((argparse::detail::has_to_string_v<std::pair<int, int>>));
  ASSERT_FALSE(argparse::detail::has_to_string_v<custom_ns::WithToWstring>);

  // 返回类型不匹配（返回 int 而非 std::string）
  ASSERT_FALSE(argparse::detail::has_to_string_v<custom_ns::BadToString>);

  // const char* 可隐式构造 std::string，匹配 identity 重载
  ASSERT_TRUE(argparse::detail::has_to_string_v<const char*>);

  // 不相关指针类型
  ASSERT_FALSE(argparse::detail::has_to_string_v<int*>);
}

// 测试 has_to_wstring trait
TEST(ConceptTest, HasToWstring) {
  // std::to_wstring 支持的标准类型
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<int>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<long long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<unsigned>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<unsigned long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<unsigned long long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<float>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<double>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<long double>);

  // std::wstring（通过 argparse 中的 identity 重载）
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<std::wstring>);

  // 通过 ADL 找到的自定义类型
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<custom_ns::WithToWstring>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<custom_ns::WithBoth>);

  // bool / char / short 等可通过整型提升隐式转换为 int
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<bool>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<char>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<short>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<unsigned short>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<unsigned char>);

  // std::to_wstring 不支持的类型
  ASSERT_FALSE(argparse::detail::has_to_wstring_v<std::string>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_v<std::vector<int>>);
  ASSERT_FALSE((argparse::detail::has_to_wstring_v<std::pair<int, int>>));
  ASSERT_FALSE(argparse::detail::has_to_wstring_v<custom_ns::WithToString>);

  // 返回类型不匹配（返回 int 而非 std::wstring）
  ASSERT_FALSE(argparse::detail::has_to_wstring_v<custom_ns::BadToWstring>);

  // const wchar_t* 可隐式构造 std::wstring，匹配 identity 重载
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<const wchar_t*>);

  // 不相关指针类型
  ASSERT_FALSE(argparse::detail::has_to_wstring_v<int*>);
}

// 测试 has_to_string 与 const/volatile 修饰符
TEST(ConceptTest, HasToStringConstVolatile) {
  ASSERT_TRUE(argparse::detail::has_to_string_v<const int>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<const double>);
  ASSERT_TRUE(argparse::detail::has_to_string_v<const std::string>);

  ASSERT_TRUE(argparse::detail::has_to_wstring_v<const int>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<const double>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<const std::wstring>);
}

// 测试 has_to_string 与枚举类型（std::to_string 不直接支持枚举）
TEST(ConceptTest, HasToStringEnum) {
  enum Color { Red, Green, Blue };
  enum class Fruit { Apple, Banana };

  // 无作用域枚举可隐式转换为 int，因此 std::to_string 可用
  ASSERT_TRUE(argparse::detail::has_to_string_v<Color>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_v<Color>);

  // 有作用域枚举不可隐式转换，std::to_string 不可用
  ASSERT_FALSE(argparse::detail::has_to_string_v<Fruit>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_v<Fruit>);
}

// ============================================================
// Tests for has_to_string_memfunc / has_string_memfunc /
// has_to_wstring_memfunc / has_wstring_memfunc / has_c_str_memfunc traits
// ============================================================

// Custom types with member functions for testing the new memfunc traits
namespace memfunc_ns {

struct WithToStringMemfunc {
  std::string to_string() const { return "via to_string()"; }
};

struct WithStringMemfunc {
  std::string string() const { return "via string()"; }
};

struct WithToWstringMemfunc {
  std::wstring to_wstring() const { return L"via to_wstring()"; }
};

struct WithWstringMemfunc {
  std::wstring wstring() const { return L"via wstring()"; }
};

struct WithCStrMemfunc {
  std::string c_str() const { return "via c_str()"; }
};

// A type with all five member functions
struct WithAll {
  std::string to_string() const { return "all"; }
  std::string string() const { return "all"; }
  std::wstring to_wstring() const { return L"all"; }
  std::wstring wstring() const { return L"all"; }
  std::string c_str() const { return "all"; }
};

// Types with wrong return types
struct BadToStringMemfunc {
  int to_string() const { return 42; }
};

struct BadStringMemfunc {
  int string() const { return 42; }
};

struct BadToWstringMemfunc {
  int to_wstring() const { return 42; }
};

struct BadWstringMemfunc {
  int wstring() const { return 42; }
};

struct BadCStrMemfunc {
  int c_str() const { return 42; }
};

// Types with wrong return types (returning different string types)
struct WrongStringReturn {
  std::wstring to_string() const { return L"wrong"; }
};

struct WrongWstringReturn {
  std::string to_wstring() const { return "wrong"; }
};

// Non-const member functions
struct NonConstToString {
  std::string to_string() { return "non-const"; }
};

struct NonConstString {
  std::string string() { return "non-const"; }
};

struct NonConstToWstring {
  std::wstring to_wstring() { return L"non-const"; }
};

struct NonConstWstring {
  std::wstring wstring() { return L"non-const"; }
};

struct NonConstCStr {
  std::string c_str() { return "non-const"; }
};

// Empty type with no member functions
struct Empty {};

}  // namespace memfunc_ns

// ============================================================
// has_to_string_memfunc
// ============================================================
TEST(ConceptTest, HasToStringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct to_string() -> std::string
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc_v<WithToStringMemfunc>);
  // Non-const member function also accepted (T&)
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc_v<NonConstToString>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<std::string>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<WithStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<BadToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<WrongStringReturn>);

  // const qualification works (matches requires(const T&))
  ASSERT_TRUE(
      argparse::detail::has_to_string_memfunc_v<const WithToStringMemfunc>);

  // volatile qualification does not (traits check const T&, not volatile T&)
  ASSERT_FALSE((
      argparse::detail::has_to_string_memfunc_v<volatile WithToStringMemfunc>));
}

// ============================================================
// has_string_memfunc
// ============================================================
TEST(ConceptTest, HasStringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct string() -> std::string
  ASSERT_TRUE(argparse::detail::has_string_memfunc_v<WithStringMemfunc>);
  ASSERT_TRUE(argparse::detail::has_string_memfunc_v<WithAll>);

  // Non-const member function also accepted (T&)
  ASSERT_TRUE(argparse::detail::has_string_memfunc_v<NonConstString>);

  // Types without string() member function
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<int>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<std::string>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<BadStringMemfunc>);

  // const/volatile qualifications
  ASSERT_TRUE(argparse::detail::has_string_memfunc_v<const WithStringMemfunc>);
}

// ============================================================
// has_to_wstring_memfunc
// ============================================================
TEST(ConceptTest, HasToWstringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct to_wstring() -> std::wstring
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc_v<WithToWstringMemfunc>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc_v<WithAll>);

  // Non-const member function also accepted (T&)
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc_v<NonConstToWstring>);

  // Types without to_wstring() member function
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<int>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<WithWstringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<BadToWstringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<WrongWstringReturn>);

  // const/volatile qualifications
  ASSERT_TRUE(
      argparse::detail::has_to_wstring_memfunc_v<const WithToWstringMemfunc>);
}

// ============================================================
// has_wstring_memfunc
// ============================================================
TEST(ConceptTest, HasWstringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct wstring() -> std::wstring
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc_v<WithWstringMemfunc>);
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc_v<WithAll>);

  // Non-const member function also accepted (T&)
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc_v<NonConstWstring>);

  // Types without wstring() member function
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<int>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<BadWstringMemfunc>);

  // const/volatile qualifications
  ASSERT_TRUE(
      argparse::detail::has_wstring_memfunc_v<const WithWstringMemfunc>);
}

// ============================================================
// has_c_str_memfunc
// ============================================================
TEST(ConceptTest, HasCStrMemfunc) {
  using namespace memfunc_ns;

  // Types with correct c_str() -> std::string
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<WithCStrMemfunc>);
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<WithAll>);

  // Non-const member function also accepted (T&)
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<NonConstCStr>);

  // Types without c_str() member function
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc_v<int>);
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<std::string>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc_v<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc_v<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc_v<BadCStrMemfunc>);

  // const/volatile qualifications
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<const WithCStrMemfunc>);
}

// ============================================================
// Combined / cross-type tests
// ============================================================
TEST(ConceptTest, MemfuncTraitsCombined) {
  using namespace memfunc_ns;

  // WithAll should satisfy all five traits
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc_v<WithAll>);
  ASSERT_TRUE(argparse::detail::has_string_memfunc_v<WithAll>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc_v<WithAll>);
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc_v<WithAll>);
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<WithAll>);

  // Each specialized type should only satisfy its own trait
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc_v<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc_v<WithToStringMemfunc>);

  ASSERT_TRUE(argparse::detail::has_string_memfunc_v<WithStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<WithStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<WithStringMemfunc>);

  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<WithCStrMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<WithCStrMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<WithCStrMemfunc>);

  // Standard library types: most should not have these member functions
  // Note: std::string has c_str() returning const char*, which is convertible
  // to std::string, so has_c_str_memfunc detects it.
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<std::string>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<std::string>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<std::string>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<std::string>);
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc_v<std::string>);

  // Same for std::wstring
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc_v<std::wstring>);

  // Empty type satisfies none
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc_v<Empty>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc_v<Empty>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc_v<Empty>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc_v<Empty>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc_v<Empty>);
}
