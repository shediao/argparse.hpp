#include <gtest/gtest.h>

#include "argparse/argparse.hpp"

using namespace argparse;

class Utf8ArgParserTest : public ::testing::Test {
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

class Utf16ArgParserTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  template <typename... Args>
  std::vector<const wchar_t*> make_args(Args... args) {
    return {args...};
  }
  std::vector<const wchar_t*> make_args(
      const wchar_t* prog, std::initializer_list<const wchar_t*> args) {
    std::vector<const wchar_t*> ret{args};
    ret.insert(ret.begin(), prog);
    return ret;
  }
};

TEST_F(Utf8ArgParserTest, UnicodeTest) {
  auto args = make_args("prog", "--you", "你", "-i", "我", "--他", "he", "--她",
                        "she", "--它", "it");

  ArgParser parser("prog", "the prog description");
  std::string i, you, he, she, it;
  parser.add_option("i", "", i);
  parser.add_option("you", "", you);
  parser.add_option("他", "", he);
  parser.add_option("她", "", she);
  parser.add_option("它", "", it);

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));

  ASSERT_EQ(i, "我");
  ASSERT_EQ(you, "你");
  ASSERT_EQ(he, "he");
  ASSERT_EQ(she, "she");
  ASSERT_EQ(it, "it");
}

#if defined(_WIN32)
TEST_F(Utf16ArgParserTest, UnicodeTest) {
  auto args = make_args(L"prog", L"--you", L"你", L"-i", L"我", L"--他", L"he",
                        L"--她", L"she", L"--它", L"it");

  ArgParser parser("prog", "the prog description");
  std::string i, you, he, she, it;
  parser.add_option("i", "", i);
  parser.add_option("you", "", you);
  parser.add_option("他", "", he);
  parser.add_option("她", "", she);
  parser.add_option("它", "", it);

  ASSERT_NO_THROW(parser.parse(args.size(), args.data()));

  ASSERT_EQ(i, "我");
  ASSERT_EQ(you, "你");
  ASSERT_EQ(he, "he");
  ASSERT_EQ(she, "she");
  ASSERT_EQ(it, "it");
}
#endif
