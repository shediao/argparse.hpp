#include <gtest/gtest.h>

#include <argparse/argparse.hpp>
#include <cstdlib>
#if defined(_WIN32)
#include "Windows.h"
#endif

bool setEnvVariable(const std::string& name, const std::string& value,
                    bool overwrite = true) {
#if defined(_WIN32) && !defined(__GNUC__)
  if (!overwrite) {
    char* pValue = nullptr;
    size_t len;
    errno_t err = _dupenv_s(&pValue, &len, name.c_str());
    if (err == 0 && pValue != nullptr) {
      free(pValue);  // 释放 _dupenv_s 分配的内存
      return true;   // 认为操作“成功”，因为它满足了不覆盖的条件
    }
    if (pValue) {
      free(pValue);
    }
  }
  if (_putenv_s(name.c_str(), value.c_str()) != 0) {
    return false;
  }
  return true;
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
