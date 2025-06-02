#include <gtest/gtest.h>

#include <optional>
#include <stdexcept>

#include "argparse/argparse.hpp"

class CheckerArgsMaker : public ::testing::Test {
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
TEST_F(CheckerArgsMaker, Allowed) {
  auto args = make_args("prog", "-t", "xx");

  argparse::ArgParser parser("prog", "");
  std::string type;
  parser.add_option("t", "type", type).choices({"debug", "release"});

  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "debug");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}
TEST_F(CheckerArgsMaker, AllowedOptional) {
  auto args = make_args("prog", "-t", "xx");

  argparse::ArgParser parser("prog", "");
  std::optional<std::string> type;
  parser.add_option("t", "type", type).choices({"debug", "release"});

  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "debug");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}

TEST_F(CheckerArgsMaker, Range) {
  auto args = make_args("prog", "-t", "2.0");

  argparse::ArgParser parser("prog", "");
  float t;
  parser.add_option("t", "", t).range(0.0f, 1.0f);
  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "0.5");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}

TEST_F(CheckerArgsMaker, RangeOptional) {
  auto args = make_args("prog", "-t", "2.0");

  argparse::ArgParser parser("prog", "");
  std::optional<float> t;
  parser.add_option("t", "", t).range(0.0f, 1.0f);
  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "0.5");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}

TEST_F(CheckerArgsMaker, Checker) {
  auto args = make_args("prog", "-t", "2.0");

  argparse::ArgParser parser("prog", "");
  float t;
  parser.add_option("t", "", t)
      .checker([](float const& v) { return v > 0.0f && v < 1.0f; }, "");
  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "0.5");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}

TEST_F(CheckerArgsMaker, CheckerOptional) {
  auto args = make_args("prog", "-t", "2.0");

  argparse::ArgParser parser("prog", "");
  std::optional<float> t;
  parser.add_option("t", "", t)
      .checker([](float const& v) { return v > 0.0f && v < 1.0f; }, "");
  ASSERT_THROW(parser.parse(args.size(), args.data()), std::invalid_argument);
  args = make_args("prog", "-t", "0.5");
  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
}
TEST_F(CheckerArgsMaker, Checker1) {
  auto args =
      make_args("prog", "-i", "1", "-f", "1.0", "-d", "1.0", "-s", "111");

  argparse::ArgParser parser("prog", "");
  int i{0};
  float f{0.0f};
  double d{0.0};
  std::string s;
  int count = 0;
  parser.add_option("i", "", i)
      .checker(
          [&count](const int& val) {
            count++;
            return val == 1;
          },
          "i value must is 1");
  parser.add_option("f", "", f)
      .checker(
          [&count](const float& val) {
            count++;
            return val == 1.0f;
          },
          "f value must is 1.0f");

  parser.add_option("d", "", d)
      .checker(
          [&count](const double& val) {
            count++;
            return val == 1.0;
          },
          "d value must is 1.0");
  parser.add_option("s", "", s)
      .checker(
          [&count](const std::string& val) {
            count++;
            return val == "111";
          },
          "s value must is \"111\"");

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));

  ASSERT_EQ(i, 1);
  ASSERT_FLOAT_EQ(f, 1);
  ASSERT_DOUBLE_EQ(d, 1);
  ASSERT_EQ(s, "111");
  ASSERT_EQ(4, count);
}

TEST_F(CheckerArgsMaker, Checker2) {
  auto args =
      make_args("prog", "-i", "1", "-f", "1.0", "-d", "1.0", "-s", "111");

  argparse::ArgParser parser("prog", "");
  std::optional<int> i{0};
  std::optional<float> f;
  std::optional<double> d;
  std::optional<std::string> s;
  int count = 0;
  parser.add_option("i", "", i)
      .range(1, 4)
      .checker(
          [&count](const int& val) {
            count++;
            return val == 1;
          },
          "");
  parser.add_option("f", "", f)
      .range(1.0f, 4.0f)
      .checker(
          [&count](const float& val) {
            count++;
            return val == 1.0f;
          },
          "");

  parser.add_option("d", "", d)
      .range(1.0, 4.0)
      .checker(
          [&count](const double& val) {
            count++;
            return val == 1;
          },
          "");
  parser.add_option("s", "", s)
      .checker(
          [&count](const std::string& val) {
            count++;
            return val == "111";
          },
          "");

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
  ASSERT_TRUE(i.has_value());
  ASSERT_TRUE(f.has_value());
  ASSERT_TRUE(d.has_value());
  ASSERT_TRUE(s.has_value());

  ASSERT_EQ(i, 1);
  ASSERT_FLOAT_EQ(f.value(), 1);
  ASSERT_DOUBLE_EQ(d.value(), 1);
  ASSERT_EQ(s, "111");
  ASSERT_EQ(4, count);
}

TEST_F(CheckerArgsMaker, Checker3) {
  auto args = make_args("prog", "-c", "1", "-c", "2", "-c", "3", "-c", "4");

  argparse::ArgParser parser("prog", "");

  std::vector<int> c;
  int count = 0;
  parser.add_option("c", "", c)
      .range(1, 4)
      .checker(
          [&count](int const& v) {
            count++;
            return 1 <= v && 4 >= v;
          },
          "");

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
  ASSERT_EQ(4, c.size());
  ASSERT_EQ(1, c[0]);
  ASSERT_EQ(2, c[1]);
  ASSERT_EQ(3, c[2]);
  ASSERT_EQ(4, c[3]);
}

TEST_F(CheckerArgsMaker, Checker4) {
  auto args =
      make_args("prog", "-p", "1,1", "-p", "2,2", "-p", "3,3", "-p", "4,4");

  argparse::ArgParser parser("prog", "");

  std::map<int, int> p;
  int count = 0;
  parser.add_option("p", "", p, ',')
      .checker(
          [&count](std::map<int, int>::value_type const& p) {
            count++;
            return p.first >= 1 && p.first <= 4 && p.first == p.second;
          },
          "");

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
  ASSERT_EQ(4, p.size());
  ASSERT_EQ(1, p[1]);
  ASSERT_EQ(2, p[2]);
  ASSERT_EQ(3, p[3]);
  ASSERT_EQ(4, p[4]);
}

TEST_F(CheckerArgsMaker, Checker5) {
  auto args =
      make_args("prog", "-p", "s1,1", "-p", "s2,2", "-p", "s3,3", "-p", "s4,4");

  argparse::ArgParser parser("prog", "");

  std::vector<std::pair<std::string, int>> p;
  int count = 0;
  parser.add_option("p", "", p, ',')
      .choices({{"s1", 1}, {"s2", 2}, {"s3", 3}, {"s4", 4}})
      .checker(
          [&](std::vector<std::pair<std::string, int>>::value_type const& p) {
            count++;
            return p.second == count;
          },
          "");

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
  ASSERT_EQ(4, p.size());
  ASSERT_EQ("s1", p[0].first);
  ASSERT_EQ("s2", p[1].first);
  ASSERT_EQ("s3", p[2].first);
  ASSERT_EQ("s4", p[3].first);
  ASSERT_EQ(1, p[0].second);
  ASSERT_EQ(2, p[1].second);
  ASSERT_EQ(3, p[2].second);
  ASSERT_EQ(4, p[3].second);
}
