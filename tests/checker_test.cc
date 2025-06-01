#include <gtest/gtest.h>

#include <stdexcept>

#include "argparse/argparse.hpp"

class ArgsMaker : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  template <typename... Args>
  std::vector<const char*> make_args(Args... args) {
    return {args...};
  }
  std::vector<const char*> make_args(const char* prog,
                                     std::initializer_list<const char*> args) {
    std::vector<const char*> ret{args};
    ret.insert(ret.begin(), prog);
    return ret;
  }
};
TEST_F(ArgsMaker, Allowed) {
  auto args = make_args("prog", "-t", "xx");

  argparse::ArgParser parser("prog", "");
  std::string type;
  parser.add_option("t", "type", type).allowed({"debug", "release"});

  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "debug");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}

TEST_F(ArgsMaker, Range) {
  auto args = make_args("prog", "-t", "2.0");

  argparse::ArgParser parser("prog", "");
  float t;
  parser.add_option("t", "", t).range(0.0f, 1.0f);
  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "0.5");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}

TEST_F(ArgsMaker, Checker) {
  auto args = make_args("prog", "-t", "2.0");

  argparse::ArgParser parser("prog", "");
  float t;
  parser.add_option("t", "", t)
      .checker([](float const& v) { return v > 0.0f && v < 1.0f; }, "");
  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "0.5");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}
