#include <gtest/gtest.h>

#include <argparse/argparse.hpp>
#include <cstdlib>
#if defined(_WIN32)
#include "Windows.h"
#endif

bool setEnvVariable(const std::string& name, const std::string& value,
                    bool overwrite = true) {
#if defined(_WIN32)
  if (!overwrite) {
    std::vector<char> buf(128);
    GetEnvironmentVariableA(name.c_str(), buf.data(),
                            static_cast<DWORD>(buf.size()));
    if (GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
      return SetEnvironmentVariableA(name.c_str(), value.c_str());
    }
    return true;
  } else {
    return SetEnvironmentVariableA(name.c_str(), value.c_str());
  }
#else
  if (setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0) != 0) {
    return false;
  }
  return true;
#endif
}

TEST(ArgParserTest, EnvTest) {
  std::vector<const char*> args{"prog"};
  argparse::ArgParser parser("prog", "the prog description");

  std::string key;
  int level{0};

  parser.add_option("key", "private key", key).env("TEST_STRING_VALUE");
  parser.add_option("level", "", level).env("TEST_INT_VALUE");

  setEnvVariable("TEST_STRING_VALUE", "******");
  setEnvVariable("TEST_INT_VALUE", "999");

  parser.parse(args.size(), args.data());

  ASSERT_EQ(key, "******");
  ASSERT_EQ(level, 999);
}

TEST(ArgParserTest, EnvTest2) {
  argparse::ArgParser parser("prog", "the prog description");

  std::string key;
  int level{0};

  parser.add_option("key", "private key", key).env("TEST_STRING_VALUE");
  parser.add_option("level", "", level).env("TEST_INT_VALUE");

  setEnvVariable("TEST_STRING_VALUE", "******");
  setEnvVariable("TEST_INT_VALUE", "999");

  std::vector<const char*> args{"prog", "--level", "123"};
  parser.parse(args.size(), args.data());

  ASSERT_EQ(key, "******");
  ASSERT_EQ(level, 123);
}

TEST(ArgParserTest, EnvTest3) {
  argparse::ArgParser parser("prog", "the prog description");

  std::string key;
  int level{0};

  parser.add_option("key", "private key", key).env("TEST_STRING_VALUE");
  parser.add_option("level", "", level).env("TEST_INT_VALUE");

  setEnvVariable("TEST_STRING_VALUE", "******");
  setEnvVariable("TEST_INT_VALUE", "999");

  std::vector<const char*> args{"prog", "--level", "123", "--key", "public"};
  parser.parse(args.size(), args.data());

  ASSERT_EQ(key, "public");
  ASSERT_EQ(level, 123);
}
