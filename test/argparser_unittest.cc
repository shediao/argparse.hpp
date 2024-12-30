//
// Created by shediao on 24-12-30.
//

#include "argparser.hpp"

#include <gtest/gtest.h>

using namespace arg::parser;
TEST(ArgParserTest, PrintUsaeg) {
  std::vector<const char*> args{"test", nullptr};
  ArgParser parser{1, args.data()};
  parser.add_flag("h,help", "show help");
  parser.add_flag("v,version", "show version info");
  parser.add_flag("d,debug", "set debuggable");
  std::string input, output;
  parser.add_option("i,input", "path to input file", input);
  parser.add_option("o,output", "path output file", output);
  parser.print_usage();
}