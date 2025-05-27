//
// Created by shediao on 24-12-30.
//

#include "argparse/argparse.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace argparse;
TEST(ArgParserTest, PrintUsaeg) {
    std::vector<const char*> args{"prog", nullptr};
    ArgParser parser{"prog", "the prog description"};
    struct Configs {
        bool show_help = false;
        bool show_version = false;
        bool is_debug = false;
        int level = 0;
        std::string input, output;
        std::vector<std::string> targets;
    };
    Configs configs;
    parser.add_flag("h,help", "Show this help message", configs.show_help);
    parser.add_flag("v,version", "Show version info", configs.show_version);
    parser.add_flag("d,debug", "Set debug mode", configs.is_debug);
    parser.add_negative_flag("release", "Set release mode", configs.is_debug);
    parser.add_flag("l,level", "Set level", configs.level);
    parser.add_option("i,input", "Path to input file", configs.input);
    parser.add_option("output", "Path output file", configs.output);
    parser.add_positional("target", "build targets", configs.targets);
    parser.print_usage();
}

TEST(ArgParserTest, DefaultHelpFlag) {
    std::vector<const char*> args{"prog", "-h"};
    ArgParser parser{"prog", "the prog description"};
    ASSERT_EXIT(parser.parse(args.size(), args.data()),
                ::testing::ExitedWithCode(0),
                ".*-h, --help.*Display this help information.*");
}

TEST(ArgParserTest, FlagAlreadyExists) {
    ArgParser parser{"prog", "the prog description"};
    bool flag;
    parser.add_flag("f,flag", "", flag);
    ASSERT_NO_THROW(parser.add_positional("flag", "", flag));

    ASSERT_THROW(parser.add_option("f", "", flag), std::runtime_error);
}
TEST(ArgParserTest, AliasAlreadyExists) {
    ArgParser parser{"prog", "the prog description"};
    bool flag;
    parser.add_option("o,option", "", flag);
    parser.add_option("o2,option2", "", flag);
    ASSERT_THROW(parser.add_alias("o2", "", "option"), std::runtime_error);
}
TEST(ArgParserTest, OptionAlreadyExists) {
    ArgParser parser{"prog", "the prog description"};
    bool flag;
    parser.add_option("f,flag", "", flag);
    ASSERT_NO_THROW(parser.add_positional("flag", "", flag));

    ASSERT_THROW(parser.add_flag("f", "", flag), std::runtime_error);
}
TEST(ArgParserTest, PositionalAlreadyExists) {
    ArgParser parser{"prog", "the prog description"};
    std::string val;
    parser.add_positional("positional", "", val);
    ASSERT_THROW(parser.add_positional("positional", "", val),
                 std::runtime_error);
}
