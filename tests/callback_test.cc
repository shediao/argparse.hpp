#include <gtest/gtest.h>

#include <map>
#include <vector>

#include "argparse/argparse.hpp"

class CallbackArgsMaker : public ::testing::Test {
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

TEST_F(CallbackArgsMaker, Callback1) {
  auto args =
      make_args("prog", "-i", "1", "-f", "1.0", "-d", "1.0", "-s", "111");

  argparse::ArgParser parser("prog", "");
  int i{0};
  float f{0.0f};
  double d{0.0};
  std::string s;
  int count = 0;
  parser.add_option("i", "", i).callback([&count](const int& val) {
    ASSERT_EQ(val, 1);
    count++;
  });
  parser.add_option("f", "", f).callback([&count](const float& val) {
    ASSERT_FLOAT_EQ(val, 1.0f);
    count++;
  });

  parser.add_option("d", "", d).callback([&count](const double& val) {
    ASSERT_DOUBLE_EQ(val, 1.0);
    count++;
  });
  parser.add_option("s", "", s).callback([&count](const std::string& val) {
    ASSERT_EQ(val, "111");
    count++;
  });

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));

  ASSERT_EQ(i, 1);
  ASSERT_FLOAT_EQ(f, 1);
  ASSERT_DOUBLE_EQ(d, 1);
  ASSERT_EQ(s, "111");
  ASSERT_EQ(4, count);
}

TEST_F(CallbackArgsMaker, Callback2) {
  auto args =
      make_args("prog", "-i", "1", "-f", "1.0", "-d", "1.0", "-s", "111");

  argparse::ArgParser parser("prog", "");
  std::optional<int> i{0};
  std::optional<float> f;
  std::optional<double> d;
  std::optional<std::string> s;
  int count = 0;
  parser.add_option("i", "", i).callback([&count](const int& val) {
    ASSERT_EQ(val, 1);
    count++;
  });
  parser.add_option("f", "", f).callback([&count](const float& val) {
    ASSERT_FLOAT_EQ(val, 1.0f);
    count++;
  });

  parser.add_option("d", "", d).callback([&count](const double& val) {
    ASSERT_DOUBLE_EQ(val, 1.0);
    count++;
  });
  parser.add_option("s", "", s).callback([&count](const std::string& val) {
    ASSERT_EQ(val, "111");
    count++;
  });

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

TEST_F(CallbackArgsMaker, Callback3) {
  auto args = make_args("prog", "-c", "1", "-c", "2", "-c", "3", "-c", "4");

  argparse::ArgParser parser("prog", "");

  std::vector<int> c;
  int count = 0;
  parser.add_option("c", "", c).callback([&](std::vector<int> const& v) {
    count++;
    ASSERT_EQ(v.size(), count);
    ASSERT_EQ(v.back(), count);
  });

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
  ASSERT_EQ(4, c.size());
  ASSERT_EQ(1, c[0]);
  ASSERT_EQ(2, c[1]);
  ASSERT_EQ(3, c[2]);
  ASSERT_EQ(4, c[3]);
}

TEST_F(CallbackArgsMaker, Callback4) {
  auto args =
      make_args("prog", "-p", "1,1", "-p", "2,2", "-p", "3,3", "-p", "4,4");

  argparse::ArgParser parser("prog", "");

  std::map<int, int> p;
  int count = 0;
  parser.add_option("p", "", p, ',').callback([&](std::map<int, int> const& p) {
    count++;
    ASSERT_EQ(p.size(), count);
  });

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));
  ASSERT_EQ(4, p.size());
  ASSERT_EQ(1, p[1]);
  ASSERT_EQ(2, p[2]);
  ASSERT_EQ(3, p[3]);
  ASSERT_EQ(4, p[4]);
}

TEST_F(CallbackArgsMaker, Callback5) {
  auto args =
      make_args("prog", "-p", "s1,1", "-p", "s2,2", "-p", "s3,3", "-p", "s4,4");

  argparse::ArgParser parser("prog", "");

  std::vector<std::pair<std::string, int>> p;
  int count = 0;
  parser.add_option("p", "", p, ',')
      .callback([&](std::vector<std::pair<std::string, int>> const& p) {
        count++;
        ASSERT_EQ(p.size(), count);
      });

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
