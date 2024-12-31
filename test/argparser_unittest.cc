//
// Created by shediao on 24-12-30.
//

#include "argparser.hpp"

#include <gtest/gtest.h>

using namespace arg::parser;
TEST(ArgParserTest, PrintUsaeg) {
    std::vector<const char*> args{"test", nullptr};
    ArgParser parser{1, args.data()};
    parser.add_flag("h,help", "Show this help message");
    parser.add_flag("v,version", "Show version info");
    parser.add_flag("d,debug", "Set debug mode");
    parser.add_flag("release", "Set release mode");
    std::string input, output;
    parser.add_option("i,input", "Path to input file", input);
    parser.add_option("output", "Path output file", output);
    parser.print_usage();
}
