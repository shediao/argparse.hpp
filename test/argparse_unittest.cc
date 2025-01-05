//
// Created by shediao on 24-12-30.
//

#include "argparse.hpp"

#include <gtest/gtest.h>

using namespace argparse;
TEST(ArgParserTest, PrintUsaeg) {
    std::vector<const char*> args{"test", nullptr};
    ArgParser parser{1, args.data()};
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
    parser.add_flag("release", "Set release mode", configs.is_debug,
                    store_false);
    parser.add_flag("l,level", "Set level", configs.level);
    parser.add_option("i,input", "Path to input file", configs.input);
    parser.add_option("output", "Path output file", configs.output);
    parser.add_positional("target", "build targets", configs.targets);
    parser.print_usage();
}
