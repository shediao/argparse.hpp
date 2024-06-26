#include "argparse.hpp"
#include <gtest/gtest.h>

TEST(Base, count0) {
  argparse::ArgParser parser;
  bool is_debug{false};
  parser.add_flag("d,debug,!r,!release", is_debug);

  ASSERT_EQ(0, parser["d"].count());

  std::vector<const char*> cmd{"argparser", "-d", "--debug"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_TRUE(is_debug);
  ASSERT_EQ(2, parser["d"].count());

  ASSERT_TRUE(parser["debug"].get<bool>());
  ASSERT_THROW(parser["debug"].get<int>(), argparse::bad_value_access);
}

TEST(Base, count1) {
  argparse::ArgParser parser;
  bool is_debug{false};
  parser.add_flag("d,debug,!r,!release", is_debug);

  ASSERT_EQ(0, parser["d"].count());

  std::vector<const char*> cmd{"argparser", "-d",        "--debug",
                               "-r",        "--release", "-d"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_TRUE(is_debug);
  ASSERT_EQ(5, parser["d"].count());

  ASSERT_TRUE(parser["debug"].get<bool>());
  ASSERT_THROW(parser["debug"].get<int>(), argparse::bad_value_access);
}

TEST(Base, is_flag) {
  argparse::ArgParser parser;
  parser.add_flag<bool>("d,debug,!r,!release");
  parser.add_option<std::string>("type");

  ASSERT_TRUE(parser["debug"].is_flag());
  ASSERT_FALSE(parser["type"].is_flag());

  ASSERT_FALSE(parser["debug"].is_option());
  ASSERT_TRUE(parser["type"].is_option());
}

TEST(Base, as) {
  argparse::ArgParser parser;
  bool verbose{false};
  parser.add_flag("v,verbose", verbose);
  parser.add_flag<bool>("d,debug,!r,!release");
  parser.add_option<std::string>("type");

  ASSERT_FALSE(parser["verbose"].get<bool>());
  ASSERT_FALSE(parser["debug"].get<bool>());
  ASSERT_EQ(parser["type"].get<std::string>(), "");

  ASSERT_THROW(parser["debug"].get<int>(), argparse::bad_value_access);
  ASSERT_THROW(parser["type"].get<int>(), argparse::bad_value_access);
  ASSERT_THROW(parser["verbose"].get<int>(), argparse::bad_value_access);
}

TEST(Base, as1) {
  argparse::ArgParser parser;
  bool verbose{false};
  parser.add_flag("v,verbose", verbose);
  parser.add_option<std::vector<std::string>>("i,input");

  ASSERT_THROW(parser["input"].get<std::string>(), argparse::bad_value_access);
  ASSERT_TRUE(parser["input"].get<std::vector<std::string>>().empty());

  std::vector<const char*> cmd{"argparser", "-i",    "file1",
                               "--input",   "file2", "-v"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_FALSE(parser["input"].get<std::vector<std::string>>().empty());
  auto& inputs = parser["input"].get<std::vector<std::string>>();

  ASSERT_EQ(inputs.size(), 2);
  ASSERT_EQ("file1", inputs[0]);
  ASSERT_EQ("file2", inputs[1]);
}

TEST(Base, set_default) {
  argparse::ArgParser parser;
  parser.add_flag<bool>("d,debug,!r,!release");
  parser.add_flag<int>("level");
  parser.add_option<std::string>("name");
  parser.add_option<std::vector<std::string>>("input");

  ASSERT_FALSE(parser["debug"].get<bool>());

  parser["debug"].set_default(true);
  ASSERT_TRUE(parser["debug"].get<bool>());

  ASSERT_EQ(0, parser["level"].get<int>());
  parser["level"].set_default(1000);
  ASSERT_EQ(1000, parser["level"].get<int>());

  ASSERT_TRUE(parser["name"].get<std::string>().empty());
  parser["name"].set_default("shediao.xsd");
  ASSERT_EQ(parser["name"].get<std::string>(), "shediao.xsd");

  ASSERT_TRUE(parser["input"].get<std::vector<std::string>>().empty());
  std::vector<std::string> inputs{"1", "2", "3", "4", "5"};
  parser["input"].set_default(inputs);

  ASSERT_EQ(parser["input"].get<std::vector<std::string>>().size(), 5);
  ASSERT_EQ(parser["input"].get<std::vector<std::string>>()[0], "1");
  ASSERT_EQ(parser["input"].get<std::vector<std::string>>()[1], "2");
  ASSERT_EQ(parser["input"].get<std::vector<std::string>>()[2], "3");
  ASSERT_EQ(parser["input"].get<std::vector<std::string>>()[3], "4");
  ASSERT_EQ(parser["input"].get<std::vector<std::string>>()[4], "5");
}

TEST(Base, set_default2) {
  argparse::ArgParser parser;
  bool is_debug{false};
  int level{0};
  std::string name{};
  std::vector<std::string> inputs{};

  parser.add_flag("d,debug,!r,!release", is_debug);
  parser.add_flag("level", level);
  parser.add_option("name", name);
  parser.add_option("input", inputs);

  ASSERT_FALSE(is_debug);

  parser["debug"].set_default(true);
  ASSERT_TRUE(is_debug);

  ASSERT_EQ(0, level);
  parser["level"].set_default(1000);
  ASSERT_EQ(1000, level);

  ASSERT_TRUE(name.empty());
  parser["name"].set_default("shediao.xsd");
  ASSERT_EQ(name, "shediao.xsd");

  ASSERT_TRUE(parser["input"].get<std::vector<std::string>>().empty());
  std::vector<std::string> inputs2{"1", "2", "3", "4", "5"};
  parser["input"].set_default(inputs2);

  ASSERT_EQ(inputs.size(), 5);
  ASSERT_EQ(inputs[0], "1");
  ASSERT_EQ(inputs[1], "2");
  ASSERT_EQ(inputs[2], "3");
  ASSERT_EQ(inputs[3], "4");
  ASSERT_EQ(inputs[4], "5");
}

TEST(Base, bind) {
  argparse::ArgParser parser;
  bool help{false};
  int level{9};
  std::string name{};
  std::string dir{};
  std::vector<std::string> files{};
  parser.add_flag("h,help").bind(help);
  parser.add_option("l,level").bind(level);
  parser.add_option("n,name").bind(name);
  parser.add_positional("dir").bind(dir);
  parser.add_positional("files").bind(files);

  std::vector<const char*> cmd{
      "test", "-l5", "--name=shediao.xsd", "dir1", "file1", "file2", "file3"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_EQ(help, parser["help"].get<bool>());
  ASSERT_EQ(level, parser["level"].get<int>());
  ASSERT_EQ(name, parser["name"].get<std::string>());
  ASSERT_EQ(dir, parser["dir"].get<std::string>());
  ASSERT_EQ(files, parser["files"].get<std::vector<std::string>>());

  ASSERT_FALSE(help);

  ASSERT_EQ(level, 5);
  ASSERT_EQ(name, "shediao.xsd");
  ASSERT_EQ(dir, "dir1");
  ASSERT_EQ(files.size(), 3);
  ASSERT_EQ("file1", files[0]);
  ASSERT_EQ("file2", files[1]);
  ASSERT_EQ("file3", files[2]);
}

TEST(ArgParser, parse0) {
  argparse::ArgParser parser1;
  std::vector<const char*> cmd{"test0"};
  ASSERT_NO_THROW(parser1.parse(cmd.size(), cmd.data()));
}
TEST(ArgParser, parse0_1) {
  argparse::ArgParser parser1;
  std::vector<const char*> cmd{"test0", "-f"};
  ASSERT_ANY_THROW(parser1.parse(cmd.size(), cmd.data()));
}
TEST(ArgParser, parse0_2) {
  argparse::ArgParser parser1;
  std::vector<const char*> cmd{"test0", "--"};
  ASSERT_NO_THROW(parser1.parse(cmd.size(), cmd.data()));
}

TEST(ArgParser, parser0_3) {
  bool flag_a{false};
  bool flag_b{false};
  bool flag_c{false};
  bool flag_d{false};
  bool flag_h{false};
  bool flag_1{false};
  bool flag_2{false};
  bool flag_3{false};
  int flag_v{0};
  std::map<std::string, std::string> option_e;
  std::vector<std::string> option_f;
  std::string option_o;

  argparse::ArgParser parser;
  parser.add_flag("a", flag_a);
  parser.add_flag("b", flag_b);
  parser.add_flag("c", flag_c);
  parser.add_flag("d", flag_d);
  parser.add_flag("h,help", flag_h);
  parser.add_flag("1", flag_1);
  parser.add_flag("2", flag_2);
  parser.add_flag("3", flag_3);
  parser.add_flag("v,verbose", flag_v);
  parser.add_option("e,env", option_e);
  parser.add_option("f,file", option_f);
  parser.add_option("o,output", option_o);

  std::vector<const char*> cmd2{"test1",
                                "-1",
                                "-b",
                                "-2",
                                "-3",
                                "-o",
                                "8",
                                "-f",
                                "myfile",
                                "--file",
                                "/path/to/myfile",
                                "-e",
                                "xxx=yyy",
                                "-e",
                                "e1=v1",
                                "-e",
                                "e2=v2",
                                "-vvvv"};
  ASSERT_NO_THROW(parser.parse(cmd2.size(), cmd2.data()));
  EXPECT_EQ(flag_a, false);
  EXPECT_EQ(flag_b, true);
  EXPECT_EQ(flag_c, false);
  EXPECT_EQ(flag_d, false);
  EXPECT_EQ(flag_h, false);
  EXPECT_EQ(flag_1, true);
  EXPECT_EQ(flag_2, true);
  EXPECT_EQ(flag_3, true);
  EXPECT_EQ(flag_v, 4);

  EXPECT_EQ(parser["a"].get<bool>(), false);
  EXPECT_EQ(parser["b"].get<bool>(), true);
  EXPECT_EQ(parser["c"].get<bool>(), false);
  EXPECT_EQ(parser["d"].get<bool>(), false);
  EXPECT_EQ(parser["h"].get<bool>(), false);
  EXPECT_EQ(parser["1"].get<bool>(), true);
  EXPECT_EQ(parser["2"].get<bool>(), true);
  EXPECT_EQ(parser["3"].get<bool>(), true);
  EXPECT_EQ(parser["v"].get<int>(), 4);

  EXPECT_EQ(parser["a"].get<bool>(), false);
  EXPECT_EQ(parser["b"].get<bool>(), true);
  EXPECT_EQ(parser["c"].get<bool>(), false);
  EXPECT_EQ(parser["d"].get<bool>(), false);
  EXPECT_EQ(parser["h"].get<bool>(), false);
  EXPECT_EQ(parser["1"].get<bool>(), true);
  EXPECT_EQ(parser["2"].get<bool>(), true);
  EXPECT_EQ(parser["3"].get<bool>(), true);
  EXPECT_EQ(parser["v"].get<int>(), 4);

  EXPECT_EQ(3, option_e.size());
  EXPECT_EQ(option_e.at("xxx"), "yyy");
  EXPECT_EQ(option_e.at("e1"), "v1");
  EXPECT_EQ(option_e.at("e2"), "v2");

  EXPECT_EQ("8", option_o);

  EXPECT_EQ(2, option_f.size());
  EXPECT_EQ("myfile", option_f[0]);
  EXPECT_EQ("/path/to/myfile", option_f[1]);
}

TEST(ArgParser, negate) {
  argparse::ArgParser parser;
  bool is_release{true};
  parser.add_flag("-r,--release,!-d,!--debug", is_release);
  std::vector<const char*> cmd{"test0", "--debug", "--release", "-d"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  EXPECT_FALSE(is_release);
}
