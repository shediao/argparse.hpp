#include <gtest/gtest.h>

#include <argparse/argparse.hpp>
#if defined(_WIN32)
#include "Windows.h"
#endif

bool setEnvVariable(const std::string& name, const std::string& value,
                    bool overwrite = true) {
#ifdef _WIN32
  // Windows 平台
  // SetEnvironmentVariable 会自动覆盖已存在的值，所以 overwrite
  // 参数在这里不起作用 如果不希望覆盖，需要手动检查
  if (!overwrite) {
    char* pValue = nullptr;
    size_t len;
    errno_t err = _dupenv_s(&pValue, &len, name.c_str());
    if (err == 0 && pValue != nullptr) {
      // 变量已存在且我们不希望覆盖它
      free(pValue);  // 释放 _dupenv_s 分配的内存
      return true;   // 认为操作“成功”，因为它满足了不覆盖的条件
    }
    if (pValue) {
      free(pValue);
    }
  }
  // SetEnvironmentVariableA 用于 ANSI 字符集
  // 如果你的程序使用宽字符 (Unicode)，应该用 SetEnvironmentVariableW
  // 并且将 std::string 转换为 std::wstring
  if (SetEnvironmentVariableA(name.c_str(), value.c_str()) == 0) {
    // 返回值为 0 表示失败
    return false;
  }
  return true;

  /*
  // 另一种 Windows 方式: _putenv_s
  // 需要拼接 "NAME=VALUE" 格式的字符串
  std::string envString = name + "=" + value;
  if (_putenv_s(name.c_str(), value.c_str()) != 0) {
      return false;
  }
  return true;
  */
#else
  // Linux, macOS, 及其他 POSIX 兼容系统
  // setenv 的第三个参数控制是否覆盖
  // 1 表示覆盖，0 表示不覆盖
  if (setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0) != 0) {
    // 返回值非 0 表示失败
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
