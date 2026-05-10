#include <gtest/gtest.h>

#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "argparse/argparse.hpp"

using namespace argparse;

// ============================================================
// 1. Move construction
// ============================================================

TEST(MoveTest, MoveConstructEmptyParser) {
  ArgParser parser1("prog", "description");

  ArgParser parser2(std::move(parser1));

  // parser2 should work
  std::vector<const char*> args{"prog"};
  EXPECT_NO_THROW(parser2.parse(args.size(), args.data()));
}

TEST(MoveTest, MoveConstructWithFlags) {
  ArgParser parser1("prog", "description");
  bool flag_a = false;
  bool flag_b = false;
  parser1.add_flag("a", "Flag A", flag_a);
  parser1.add_flag("b", "Flag B", flag_b);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "-a", "-b"};
  parser2.parse(args.size(), args.data());

  EXPECT_TRUE(flag_a);
  EXPECT_TRUE(flag_b);
}

TEST(MoveTest, MoveConstructWithOptions) {
  ArgParser parser1("prog", "description");
  std::string name;
  int count = 0;
  parser1.add_option("n,name", "Name", name);
  parser1.add_option("c,count", "Count", count);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "--name=test", "-c", "42"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(name, "test");
  EXPECT_EQ(count, 42);
}

TEST(MoveTest, MoveConstructWithPositionals) {
  ArgParser parser1("prog", "description");
  std::string input;
  std::string output;
  parser1.add_positional("input", "Input file", input);
  parser1.add_positional("output", "Output file", output);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "in.txt", "out.txt"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(input, "in.txt");
  EXPECT_EQ(output, "out.txt");
}

TEST(MoveTest, MoveConstructWithVectorOption) {
  ArgParser parser1("prog", "description");
  std::vector<int> numbers;
  parser1.add_option("n", "Numbers", numbers);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "-n", "1", "-n", "2", "-n", "3"};
  parser2.parse(args.size(), args.data());

  ASSERT_EQ(numbers.size(), 3);
  EXPECT_EQ(numbers[0], 1);
  EXPECT_EQ(numbers[1], 2);
  EXPECT_EQ(numbers[2], 3);
}

TEST(MoveTest, MoveConstructWithTupleOption) {
  ArgParser parser1("prog", "description");
  std::tuple<int, int> point;
  parser1.add_option("p,point", "A point", point, ',');

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "--point", "3,4"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(std::get<0>(point), 3);
  EXPECT_EQ(std::get<1>(point), 4);
}

TEST(MoveTest, MoveConstructWithSubcommand) {
  ArgParser parser1("prog", "description");
  std::string commit_msg;
  parser1.add_command("commit", "Commit command")
      .add_option("m,message", "Commit message", commit_msg);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "commit", "-m", "hello world"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(commit_msg, "hello world");
}

TEST(MoveTest, MoveConstructWithCallback) {
  bool callback_called = false;

  ArgParser parser1("prog", "description");
  parser1.callback([&callback_called]() { callback_called = true; });

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog"};
  parser2.parse(args.size(), args.data());

  EXPECT_TRUE(callback_called);
}

TEST(MoveTest, MoveConstructWithFlagCallback) {
  ArgParser parser1("prog", "description");
  bool flag = false;
  bool flag_callback_called = false;
  parser1.add_flag("f,flag", "A flag", flag)
      .callback([&flag_callback_called](bool val) {
        if (val) {
          flag_callback_called = true;
        }
      });

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "-f"};
  parser2.parse(args.size(), args.data());

  EXPECT_TRUE(flag);
  EXPECT_TRUE(flag_callback_called);
}

TEST(MoveTest, MoveConstructWithDefaultValues) {
  ArgParser parser1("prog", "description");
  std::string name;
  parser1.add_option("n,name", "Name", name).default_value("default_name");

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(name, "default_name");
}

TEST(MoveTest, MoveConstructMultipleLevels) {
  // Move construct, then move construct again
  ArgParser parser1("prog", "description");
  bool flag = false;
  parser1.add_flag("f", "Flag", flag);

  ArgParser parser2(std::move(parser1));
  ArgParser parser3(std::move(parser2));

  std::vector<const char*> args{"prog", "-f"};
  parser3.parse(args.size(), args.data());

  EXPECT_TRUE(flag);
}

// ============================================================
// 2. Move assignment
// ============================================================

TEST(MoveTest, MoveAssignEmptyParser) {
  ArgParser parser1("prog1", "desc1");
  ArgParser parser2("prog2", "desc2");

  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog"};
  EXPECT_NO_THROW(parser2.parse(args.size(), args.data()));
}

TEST(MoveTest, MoveAssignWithFlags) {
  ArgParser parser1("prog1", "desc1");
  bool flag_a = false;
  bool flag_b = false;
  parser1.add_flag("a", "Flag A", flag_a);
  parser1.add_flag("b", "Flag B", flag_b);

  ArgParser parser2("prog2", "desc2");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog", "-a", "-b"};
  parser2.parse(args.size(), args.data());

  EXPECT_TRUE(flag_a);
  EXPECT_TRUE(flag_b);
}

TEST(MoveTest, MoveAssignWithOptions) {
  ArgParser parser1("prog1", "desc1");
  std::string name;
  int count = 0;
  parser1.add_option("n,name", "Name", name);
  parser1.add_option("c,count", "Count", count);

  ArgParser parser2("prog2", "desc2");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog", "-n", "hello", "--count=99"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(name, "hello");
  EXPECT_EQ(count, 99);
}

TEST(MoveTest, MoveAssignWithPositionals) {
  ArgParser parser1("prog1", "desc1");
  std::string input;
  std::string output;
  parser1.add_positional("input", "Input file", input);
  parser1.add_positional("output", "Output file", output);

  ArgParser parser2("prog2", "desc2");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog", "a.txt", "b.txt"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(input, "a.txt");
  EXPECT_EQ(output, "b.txt");
}

TEST(MoveTest, MoveAssignWithSubcommands) {
  ArgParser parser1("prog1", "desc1");
  std::string msg;
  parser1.add_command("send", "Send command")
      .add_option("m,msg", "Message", msg);

  ArgParser parser2("prog2", "desc2");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog", "send", "-m", "hi"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(msg, "hi");
}

TEST(MoveTest, MoveAssignOverwritesExistingContent) {
  ArgParser parser1("prog1", "desc1");
  bool flag1 = false;
  parser1.add_flag("f", "Flag 1", flag1);

  ArgParser parser2("prog2", "desc2");
  bool flag2 = false;
  parser2.add_flag("f", "Flag 2", flag2);

  // Move parser1 into parser2, overwriting parser2's content
  parser2 = std::move(parser1);

  // flag1 should now be the bound variable, not flag2
  std::vector<const char*> args{"prog", "-f"};
  parser2.parse(args.size(), args.data());

  EXPECT_TRUE(flag1);
  EXPECT_FALSE(flag2);  // flag2 should NOT be affected
}

TEST(MoveTest, MoveAssignMultipleTimes) {
  ArgParser parser1("prog1", "desc1");
  bool flag1 = false;
  parser1.add_flag("a", "Flag A", flag1);

  ArgParser parser2("prog2", "desc2");
  bool flag2 = false;
  parser2.add_flag("b", "Flag B", flag2);

  ArgParser parser3("prog3", "desc3");
  bool flag3 = false;
  parser3.add_flag("c", "Flag C", flag3);

  // Chain: parser3 <- parser2 <- parser1
  parser2 = std::move(parser1);
  parser3 = std::move(parser2);

  std::vector<const char*> args{"prog", "-a"};
  parser3.parse(args.size(), args.data());

  EXPECT_TRUE(flag1);
  EXPECT_FALSE(flag2);
  EXPECT_FALSE(flag3);
}

// ============================================================
// 3. Move with mixed argument types
// ============================================================

TEST(MoveTest, MoveConstructMixedArgTypes) {
  ArgParser parser1("prog", "description");
  bool verbose = false;
  int count = 0;
  std::string name;
  std::vector<std::string> files;
  std::pair<int, int> point;

  parser1.add_flag("v,verbose", "Verbose", verbose);
  parser1.add_flag("c,count", "Count", count);
  parser1.add_option("n,name", "Name", name);
  parser1.add_option("p,point", "Point", point, ',');
  parser1.add_positional("files", "Files", files);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog",    "-v",    "-c",        "--name=test",
                                "--point", "10,20", "file1.txt", "file2.txt"};
  parser2.parse(args.size(), args.data());

  EXPECT_TRUE(verbose);
  EXPECT_EQ(count, 1);
  EXPECT_EQ(name, "test");
  EXPECT_EQ(point.first, 10);
  EXPECT_EQ(point.second, 20);
  ASSERT_EQ(files.size(), 2);
  EXPECT_EQ(files[0], "file1.txt");
  EXPECT_EQ(files[1], "file2.txt");
}

TEST(MoveTest, MoveAssignMixedArgTypes) {
  ArgParser parser1("prog1", "desc1");
  bool verbose = false;
  int count = 0;
  std::string name;
  std::vector<std::string> files;

  parser1.add_flag("v,verbose", "Verbose", verbose);
  parser1.add_flag("c,count", "Count", count);
  parser1.add_option("n,name", "Name", name);
  parser1.add_positional("files", "Files", files);

  ArgParser parser2("prog2", "desc2");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog",         "-v",    "-c",    "-c",
                                "--name=world", "a.txt", "b.txt", "c.txt"};
  parser2.parse(args.size(), args.data());

  EXPECT_TRUE(verbose);
  EXPECT_EQ(count, 2);
  EXPECT_EQ(name, "world");
  ASSERT_EQ(files.size(), 3);
  EXPECT_EQ(files[0], "a.txt");
  EXPECT_EQ(files[1], "b.txt");
  EXPECT_EQ(files[2], "c.txt");
}

// ============================================================
// 4. Move with count() functionality
// ============================================================

TEST(MoveTest, MoveConstructCountWorks) {
  ArgParser parser1("prog", "description");
  bool flag = false;
  std::string name;
  parser1.add_flag("f,flag", "A flag", flag);
  parser1.add_option("n,name", "Name", name);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "-f", "-f", "--name=test"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(parser2["flag"].count(), 2);
  EXPECT_EQ(parser2["name"].count(), 1);
}

TEST(MoveTest, MoveAssignCountWorks) {
  ArgParser parser1("prog1", "desc1");
  bool flag = false;
  std::string name;
  parser1.add_flag("f,flag", "A flag", flag);
  parser1.add_option("n,name", "Name", name);

  ArgParser parser2("prog2", "desc2");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog", "-f", "--name=hi"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(parser2["flag"].count(), 1);
  EXPECT_EQ(parser2["name"].count(), 1);
}

// ============================================================
// 5. Move with negatable flags and negative flags
// ============================================================

TEST(MoveTest, MoveConstructNegatableFlag) {
  ArgParser parser1("prog", "description");
  bool flag = true;
  parser1.add_flag("f,flag", "A flag", flag).negatable(true);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "--no-flag"};
  parser2.parse(args.size(), args.data());

  EXPECT_FALSE(flag);
}

TEST(MoveTest, MoveConstructNegativeFlag) {
  ArgParser parser1("prog", "description");
  int val = 5;
  parser1.add_negative_flag("d,dec", "Decrement", val);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "-d", "--dec"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(val, 3);
}

TEST(MoveTest, MoveAssignNegatableFlag) {
  ArgParser parser1("prog1", "desc1");
  bool flag = true;
  parser1.add_flag("f,flag", "A flag", flag).negatable(true);

  ArgParser parser2("prog2", "desc2");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog", "--no-flag"};
  parser2.parse(args.size(), args.data());

  EXPECT_FALSE(flag);
}

// ============================================================
// 6. Move with choices
// ============================================================

TEST(MoveTest, MoveConstructChoicesWork) {
  ArgParser parser1("prog", "description");
  std::string color;
  parser1.add_option("color", "Color", color)
      .choices({{"red", ""}, {"green", ""}, {"blue", ""}});

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "--color", "green"};
  parser2.parse(args.size(), args.data());

  EXPECT_EQ(color, "green");
}

TEST(MoveTest, MoveConstructInvalidChoiceThrows) {
  ArgParser parser1("prog", "description");
  std::string color;
  parser1.add_option("color", "Color", color)
      .choices({{"red", ""}, {"green", ""}, {"blue", ""}});

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "--color", "yellow"};
  EXPECT_THROW(parser2.parse(args.size(), args.data()), std::invalid_argument);
}

// ============================================================
// 7. Move with nested subcommands
// ============================================================

TEST(MoveTest, MoveConstructNestedSubcommands) {
  ArgParser parser1("git", "version control");
  std::string remote_name;
  std::string branch_name;
  bool force = false;

  parser1.add_command("remote", "Manage remotes")
      .add_positional("name", "Remote name", remote_name);
  parser1.add_command("branch", "Manage branches")
      .add_option("d,delete", "Delete branch", branch_name);
  parser1.add_command("push", "Push changes")
      .add_flag("f,force", "Force", force);

  ArgParser parser2(std::move(parser1));

  // Test branch subcommand
  {
    std::vector<const char*> args{"git", "branch", "-d", "feature-x"};
    parser2.parse(args.size(), args.data());
    EXPECT_EQ(branch_name, "feature-x");
    EXPECT_FALSE(force);
  }

  // Reset for next test
  remote_name.clear();
  force = false;
  branch_name.clear();

  // Test remote subcommand
  {
    std::vector<const char*> args{"git", "remote", "origin"};
    parser2.parse(args.size(), args.data());
    EXPECT_EQ(remote_name, "origin");
  }

  // Test push subcommand
  {
    std::vector<const char*> args{"git", "push", "-f"};
    parser2.parse(args.size(), args.data());
    EXPECT_TRUE(force);
  }
}

// ============================================================
// 8. Move with optional values
// ============================================================

TEST(MoveTest, MoveConstructOptionalValue) {
  ArgParser parser1("prog", "description");
  std::optional<std::string> name;
  std::optional<int> age;
  parser1.add_option("n,name", "Name", name);
  parser1.add_option("a,age", "Age", age);

  ArgParser parser2(std::move(parser1));

  {
    std::vector<const char*> args{"prog", "--name=alice"};
    parser2.parse(args.size(), args.data());
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name.value(), "alice");
    EXPECT_FALSE(age.has_value());
  }

  name.reset();
  age.reset();

  {
    std::vector<const char*> args{"prog", "--age=30"};
    parser2.parse(args.size(), args.data());
    EXPECT_FALSE(name.has_value());
    ASSERT_TRUE(age.has_value());
    EXPECT_EQ(age.value(), 30);
  }
}

// ============================================================
// 9. Move-and-use pattern: configure then move into final parser
// ============================================================

TEST(MoveTest, MoveIntoFinalParser) {
  // Simulate a "builder" pattern: configure a temporary parser, then move
  // it into the final location.
  ArgParser final_parser("prog", "description");

  {
    ArgParser temp("temp", "");
    bool flag = false;
    std::string name;
    temp.add_flag("f,flag", "Flag", flag);
    temp.add_option("n,name", "Name", name);

    // Move into final location
    final_parser = std::move(temp);

    // temp is now in a moved-from state; we should not use it further
  }

  // final_parser should work correctly
  std::vector<const char*> args{"prog", "-f", "--name=test"};
  final_parser.parse(args.size(), args.data());
}

// ============================================================
// 10. Move after parse (is_parsed_ flag)
// ============================================================

TEST(MoveTest, MoveConstructAfterParseStillWorks) {
  ArgParser parser1("prog", "description");
  bool flag = false;
  parser1.add_flag("f,flag", "Flag", flag);

  // Parse with parser1 first
  {
    std::vector<const char*> args{"prog", "-f"};
    parser1.parse(args.size(), args.data());
    EXPECT_TRUE(flag);
  }

  // Now move construct parser2 from parser1
  flag = false;
  ArgParser parser2(std::move(parser1));

  // parser2 should work independently
  std::vector<const char*> args{"prog", "-f"};
  parser2.parse(args.size(), args.data());
  EXPECT_TRUE(flag);
}

// ============================================================
// 11. Move with remain_positionals
// ============================================================

TEST(MoveTest, MoveConstructRemainPositionals) {
  ArgParser parser1("prog", "description");
  bool flag = false;
  std::vector<std::string> remaining;
  parser1.add_flag("f,flag", "Flag", flag);
  parser1.add_positional("remaining", "Remaining args", remaining);
  parser1.set_treat_remaining_as_positional(true);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "-f", "extra1", "extra2"};
  // Should not throw because treat_remaining_as_positional is true and
  // remaining is a multi-valued positional
  EXPECT_NO_THROW(parser2.parse(args.size(), args.data()));
  EXPECT_TRUE(flag);
  ASSERT_EQ(remaining.size(), 2);
  EXPECT_EQ(remaining[0], "extra1");
  EXPECT_EQ(remaining[1], "extra2");
}

TEST(MoveTest, MoveAssignRemainPositionals) {
  ArgParser parser1("prog", "description");
  bool flag = false;
  std::vector<std::string> remaining;
  parser1.add_flag("f,flag", "Flag", flag);
  parser1.add_positional("remaining", "Remaining args", remaining);
  parser1.set_treat_remaining_as_positional(true);

  ArgParser parser2("other", "");
  parser2 = std::move(parser1);

  std::vector<const char*> args{"prog", "-f", "a", "b", "c"};
  EXPECT_NO_THROW(parser2.parse(args.size(), args.data()));
  EXPECT_TRUE(flag);
  ASSERT_EQ(remaining.size(), 3);
  EXPECT_EQ(remaining[0], "a");
  EXPECT_EQ(remaining[1], "b");
  EXPECT_EQ(remaining[2], "c");
}

// ============================================================
// 12. Move with map and container types
// ============================================================

TEST(MoveTest, MoveConstructMapOption) {
  ArgParser parser1("prog", "description");
  std::map<std::string, std::string> config;
  parser1.add_option("c,config", "Config", config, '=');

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "-c", "key1=val1", "-c", "key2=val2"};
  parser2.parse(args.size(), args.data());

  ASSERT_EQ(config.size(), 2);
  EXPECT_EQ(config["key1"], "val1");
  EXPECT_EQ(config["key2"], "val2");
}

TEST(MoveTest, MoveConstructVectorOfPairs) {
  ArgParser parser1("prog", "description");
  std::vector<std::pair<std::string, std::string>> pairs;
  parser1.add_option("p,pair", "Pairs", pairs, ':');

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "--pair", "a:1", "--pair", "b:2"};
  parser2.parse(args.size(), args.data());

  ASSERT_EQ(pairs.size(), 2);
  EXPECT_EQ(pairs[0].first, "a");
  EXPECT_EQ(pairs[0].second, "1");
  EXPECT_EQ(pairs[1].first, "b");
  EXPECT_EQ(pairs[1].second, "2");
}

// ============================================================
// 13. Default move operations compile-time checks
// ============================================================

// ArgParser move operations should be usable in standard library containers
TEST(MoveTest, PutArgParserIntoContainer) {
  std::vector<ArgParser> parsers;
  parsers.emplace_back("prog1", "desc1");

  bool flag = false;
  parsers.back().add_flag("f", "Flag", flag);

  // Move within vector (e.g., during reallocation) should work
  std::vector<const char*> args{"prog", "-f"};
  parsers[0].parse(args.size(), args.data());
  EXPECT_TRUE(flag);
}

TEST(MoveTest, SwapArgParsers) {
  ArgParser parser1("prog1", "desc1");
  bool flag1 = false;
  parser1.add_flag("a", "Flag A", flag1);

  ArgParser parser2("prog2", "desc2");
  bool flag2 = false;
  parser2.add_flag("b", "Flag B", flag2);

  std::swap(parser1, parser2);

  // After swap, parser1 should have flag "b" and parser2 should have flag "a"
  {
    std::vector<const char*> args{"prog", "-b"};
    parser1.parse(args.size(), args.data());
    EXPECT_TRUE(flag2);
    EXPECT_FALSE(flag1);
  }

  flag1 = false;
  flag2 = false;

  {
    std::vector<const char*> args{"prog", "-a"};
    parser2.parse(args.size(), args.data());
    EXPECT_TRUE(flag1);
    EXPECT_FALSE(flag2);
  }
}

// ============================================================
// 14. Move with aliases
// ============================================================

TEST(MoveTest, MoveConstructWithAliases) {
  ArgParser parser1("prog", "description");
  std::string url;
  parser1.add_option("url", "URL", url);
  parser1.add_alias("google", "url", "https://www.google.com");
  parser1.add_alias("bing", "url", "https://www.bing.com");

  ArgParser parser2(std::move(parser1));

  {
    std::vector<const char*> args{"prog", "--google"};
    parser2.parse(args.size(), args.data());
    EXPECT_EQ(url, "https://www.google.com");
  }

  {
    std::vector<const char*> args{"prog", "--bing"};
    parser2.parse(args.size(), args.data());
    EXPECT_EQ(url, "https://www.bing.com");
  }
}

// ============================================================
// 15. Move with hidden commands
// ============================================================

TEST(MoveTest, MoveConstructHiddenCommand) {
  ArgParser parser1("prog", "description");
  bool flag = false;
  auto& cmd = parser1.add_command("hidden_cmd", "A hidden command")
                  .add_flag("f,flag", "Flag", flag);
  cmd.hidden(true);

  ArgParser parser2(std::move(parser1));

  std::vector<const char*> args{"prog", "hidden_cmd", "-f"};
  parser2.parse(args.size(), args.data());
  EXPECT_TRUE(flag);
}
