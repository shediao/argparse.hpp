#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "argparse/argparse.hpp"

class OptionalBindTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  template <typename... Args>
  std::vector<const char*> make_args(Args... args) {
    return {args...};
  }
};

// 测试 add_flag 绑定到 optional 类型
TEST_F(OptionalBindTest, FlagBinding) {
  argparse::ArgParser parser("test_prog", "测试 optional 类型的标志绑定");

  std::optional<bool> flag1;
  std::optional<bool> flag2;
  std::optional<int> count;

  parser.add_flag("f,flag1", "测试标志1", flag1);
  parser.add_flag("g,flag2", "测试标志2", flag2);
  parser.add_flag("c,count", "计数器", count);

  auto args = make_args("test_prog");
  // 测试不传入标志的情况
  parser.parse(args.size(), args.data());
  ASSERT_FALSE(flag1.has_value());
  ASSERT_FALSE(flag2.has_value());
  ASSERT_FALSE(count.has_value());

  // 测试传入标志的情况
  args = make_args("test_prog", "-f", "--flag2", "-c");
  parser.parse(args.size(), args.data());
  ASSERT_TRUE(flag1.has_value());
  ASSERT_TRUE(flag1.value());
  ASSERT_TRUE(flag2.has_value());
  ASSERT_TRUE(flag2.value());
  ASSERT_TRUE(count.has_value());
  ASSERT_EQ(count.value(), 1);
}

// 测试 add_option 绑定到 optional 类型
TEST_F(OptionalBindTest, OptionBinding) {
  argparse::ArgParser parser("test_prog", "测试 optional 类型的选项绑定");

  std::optional<int> opt_int;
  std::optional<std::string> opt_str;
  std::optional<double> opt_double;

  parser.add_option("i,int", "可选整数", opt_int);
  parser.add_option("s,str", "可选字符串", opt_str);
  parser.add_option("d,double", "可选浮点数", opt_double);

  // 测试不传入选项的情况
  auto args = make_args("test_prog");
  parser.parse(args.size(), args.data());
  ASSERT_FALSE(opt_int.has_value());
  ASSERT_FALSE(opt_str.has_value());
  ASSERT_FALSE(opt_double.has_value());

  // 测试传入选项的情况
  args = make_args("test_prog", "-i", "42", "-s", "hello", "-d", "3.14");
  parser.parse(args.size(), args.data());
  ASSERT_TRUE(opt_int.has_value());
  ASSERT_EQ(opt_int.value(), 42);
  ASSERT_TRUE(opt_str.has_value());
  ASSERT_EQ(opt_str.value(), "hello");
  ASSERT_TRUE(opt_double.has_value());
  ASSERT_DOUBLE_EQ(opt_double.value(), 3.14);
}

// 测试 add_positional 绑定到 optional 类型
TEST_F(OptionalBindTest, PositionalBinding) {
  argparse::ArgParser parser("test_prog", "测试 optional 类型的位置参数绑定");

  std::optional<int> pos1;
  std::optional<std::string> pos2;
  std::optional<double> pos3;

  parser.add_positional("pos1", "第一个位置参数", pos1);
  parser.add_positional("pos2", "第二个位置参数", pos2);
  parser.add_positional("pos3", "第三个位置参数", pos3);

  // 测试传入位置参数的情况
  auto args = make_args("test_prog", "42", "hello", "3.14");
  parser.parse(args.size(), args.data());
  ASSERT_TRUE(pos1.has_value());
  ASSERT_EQ(pos1.value(), 42);
  ASSERT_TRUE(pos2.has_value());
  ASSERT_EQ(pos2.value(), "hello");
  ASSERT_TRUE(pos3.has_value());
  ASSERT_DOUBLE_EQ(pos3.value(), 3.14);
}

// 测试默认值
TEST_F(OptionalBindTest, DefaultValues) {
  argparse::ArgParser parser("test_prog", "测试 optional 类型的默认值");

  std::optional<int> opt_int;
  std::optional<std::string> opt_str;
  std::optional<double> opt_double;

  auto& int_opt = parser.add_option("i,int", "可选整数", opt_int);
  int_opt.default_value("100");

  auto& str_opt = parser.add_option("s,str", "可选字符串", opt_str);
  str_opt.default_value("default");

  auto& double_opt = parser.add_option("d,double", "可选浮点数", opt_double);
  double_opt.default_value("3.14");

  // 测试使用默认值
  auto args = make_args("test_prog");
  parser.parse(args.size(), args.data());
  ASSERT_TRUE(opt_int.has_value());
  ASSERT_EQ(opt_int.value(), 100);
  ASSERT_TRUE(opt_str.has_value());
  ASSERT_EQ(opt_str.value(), "default");
  ASSERT_TRUE(opt_double.has_value());
  ASSERT_DOUBLE_EQ(opt_double.value(), 3.14);
}
