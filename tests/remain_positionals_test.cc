#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "argparse/argparse.hpp"

class RemainOptionalsTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  template <typename... Args>
  std::vector<const char*> make_args(Args... args) {
    return {args...};
  }
};

// 测试 add_flag 绑定到 optional 类型
TEST_F(RemainOptionalsTest, Test1) {
  argparse::ArgParser parser("test_prog", "测试 optional 类型的标志绑定");

  std::optional<bool> flag1;
  std::optional<bool> flag2;
  std::optional<int> count;
  std::vector<std::string> cmds;

  parser.add_flag("f,flag1", "测试标志1", flag1);
  parser.add_flag("g,flag2", "测试标志2", flag2);
  parser.add_flag("c,count", "计数器", count);
  parser.add_positional("cmds", "cmds", cmds);
  parser.set_remaining_are_positional();

  auto args = make_args("test_prog");
  // 测试不传入标志的情况
  parser.parse(args.size(), args.data());
  ASSERT_FALSE(flag1.has_value());
  ASSERT_FALSE(flag2.has_value());
  ASSERT_FALSE(count.has_value());
  ASSERT_TRUE(cmds.empty());

  // 测试传入标志的情况
  args = make_args("test_prog", "-f", "--flag2", "-c");
  parser.parse(args.size(), args.data());
  ASSERT_TRUE(flag1.has_value());
  ASSERT_TRUE(flag1.value());
  ASSERT_TRUE(flag2.has_value());
  ASSERT_TRUE(flag2.value());
  ASSERT_TRUE(count.has_value());
  ASSERT_EQ(count.value(), 1);
  ASSERT_TRUE(cmds.empty());

  args = make_args("test_prog", "-f", "--flag2", "-c", "cmd", "-a", "-b", "val",
                   "-f", "--flag2", "-c");
  parser.parse(args.size(), args.data());
  ASSERT_EQ(cmds.size(), 7);
  ASSERT_EQ("cmd", cmds[0]);
  ASSERT_EQ("-a", cmds[1]);
  ASSERT_EQ("-b", cmds[2]);
  ASSERT_EQ("val", cmds[3]);
  ASSERT_EQ("-f", cmds[4]);
  ASSERT_EQ("--flag2", cmds[5]);
  ASSERT_EQ("-c", cmds[6]);
}
