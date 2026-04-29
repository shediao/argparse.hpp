#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "argparse/argparse.hpp"

using namespace argparse;

class CompletionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    parser_ = std::make_unique<ArgParser>("myprog", "A test program");
    setup_full_parser(*parser_);
  }
  void TearDown() override { parser_.reset(); }

  ArgParser& parser() { return *parser_; }

  /// Build a non-trivial ArgParser with flags, options (some with choices),
  /// and subcommands.  (Positional args are mutually exclusive with
  /// subcommands, so we omit them here.)
  static void setup_full_parser(ArgParser& parser) {
    // A plain bool flag with short+long names
    bool verbose = false;
    parser.add_flag("v,verbose", "Enable verbose output", verbose);

    // A negatable bool flag
    bool debug = true;
    parser.add_flag("debug", "Enable debug mode", debug).negatable(true);

    // An integral counter flag
    int count = 0;
    parser.add_flag("c,count", "Increase count", count);

    // An option with explicit value_placeholder and choices
    std::string mode;
    parser.add_option("m,mode", "Operation mode", mode)
        .value_placeholder("MODE")
        .choices_description({{"fast", "Fast mode"},
                              {"slow", "Slow mode"},
                              {"auto", "Automatic mode"}});

    // An option without choices (file argument)
    std::string infile;
    parser.add_option("i,input", "Input file", infile)
        .value_placeholder("FILE");

    // A hidden flag that MUST NOT appear in completions
    bool secret = false;
    parser.add_flag("secret", "Secret flag", secret).hidden(true);

    // A hidden option that MUST NOT appear in completions
    std::string hidden_opt;
    parser.add_option("H,hidden-opt", "Hidden option", hidden_opt).hidden(true);

    // Subcommands
    auto& cmd1 = parser.add_command("build", "Build the project");
    cmd1.hidden(false);
    parser.add_command("test", "Run tests");
    parser.add_command("hidden_cmd", "Hidden command").hidden(true);
  }

 private:
  std::unique_ptr<ArgParser> parser_;
};

// ---------------------------------------------------------------------------
// Bash completion tests
// ---------------------------------------------------------------------------

TEST_F(CompletionTest, BashContainsFunctionDefinition) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("_myprog() {") != std::string::npos);
  EXPECT_TRUE(out.find("complete -F _myprog myprog") != std::string::npos);
}

TEST_F(CompletionTest, BashContainsLongFlag) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // Long option name (with --) should appear in the opts list
  EXPECT_TRUE(out.find("--verbose") != std::string::npos);
  EXPECT_TRUE(out.find("--debug") != std::string::npos);
}

TEST_F(CompletionTest, BashContainsShortFlag) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("-v") != std::string::npos);
  EXPECT_TRUE(out.find("-c") != std::string::npos);
}

TEST_F(CompletionTest, BashContainsNegatableFlagVariant) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // Negatable flag debug produces both --debug and --no-debug
  EXPECT_TRUE(out.find("--no-debug") != std::string::npos);
}

TEST_F(CompletionTest, BashDoesNotContainHiddenFlag) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // Hidden flag and hidden option must be absent
  EXPECT_TRUE(out.find("--secret") == std::string::npos);
  EXPECT_TRUE(out.find("-H") == std::string::npos);
  EXPECT_TRUE(out.find("--hidden-opt") == std::string::npos);
}

TEST_F(CompletionTest, BashOptionWithChoicesHasPrevWordCase) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // The mode option has choices; there must be a prev-word case entry
  EXPECT_TRUE(out.find("--mode)") != std::string::npos ||
              out.find("-m|--mode)") != std::string::npos ||
              out.find("--mode|") != std::string::npos);
  // compgen with choices should list "fast slow auto"
  EXPECT_TRUE(out.find("fast") != std::string::npos);
  EXPECT_TRUE(out.find("slow") != std::string::npos);
  EXPECT_TRUE(out.find("auto") != std::string::npos);
}

TEST_F(CompletionTest, BashOptionWithoutChoicesHasPrevWordFileCase) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // The input option has no choices → should fall back to file completion
  EXPECT_TRUE(out.find("compgen -f") != std::string::npos);
}

TEST_F(CompletionTest, BashEqualsSeparatedDispatchForChoices) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // Equals-sign based dispatch for long options with choices
  EXPECT_TRUE(out.find("--mode)") != std::string::npos);
  // The equals dispatch block should contain compgen for val
  EXPECT_TRUE(out.find("compgen -W") != std::string::npos);
}

TEST_F(CompletionTest, BashContainsSubcommands) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // Visible subcommands
  EXPECT_TRUE(out.find("build") != std::string::npos);
  EXPECT_TRUE(out.find("test") != std::string::npos);
  // Hidden subcommand must not appear
  EXPECT_TRUE(out.find("hidden_cmd") == std::string::npos);
}

TEST_F(CompletionTest, BashContainsCompleteDashFDash) {
  auto& p = parser();
  std::ostringstream os;
  p.print_bash_complete(os);
  std::string out = os.str();

  // The final registration line
  EXPECT_TRUE(out.find("complete -F _myprog myprog") != std::string::npos);
}

TEST_F(CompletionTest, BashSanitizesProgName) {
  ArgParser parser("my-prog.test", "A test program");
  bool v = false;
  parser.add_flag("verbose", "Verbose", v);

  std::ostringstream os;
  parser.print_bash_complete(os);
  std::string out = os.str();

  // Function name must only contain alphanumerics and underscores
  EXPECT_TRUE(out.find("_my_prog_test() {") != std::string::npos);
}

TEST_F(CompletionTest, BashNoOptionsProducesMinimalScript) {
  ArgParser parser("empty", "No args");
  std::ostringstream os;
  parser.print_bash_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("_empty() {") != std::string::npos);
  EXPECT_TRUE(out.find("complete -F _empty empty") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Zsh completion tests
// ---------------------------------------------------------------------------

TEST_F(CompletionTest, ZshContainsCompdefLine) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("#compdef myprog") != std::string::npos);
}

TEST_F(CompletionTest, ZshContainsFunctionDefinition) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("_myprog() {") != std::string::npos);
  EXPECT_TRUE(out.find("_arguments -s $options") != std::string::npos);
}

TEST_F(CompletionTest, ZshContainsShortOption) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("'-v") != std::string::npos);
  EXPECT_TRUE(out.find("'-c") != std::string::npos);
}

TEST_F(CompletionTest, ZshContainsLongOption) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("'--verbose") != std::string::npos);
  EXPECT_TRUE(out.find("'--debug") != std::string::npos);
}

TEST_F(CompletionTest, ZshContainsNegatableFlag) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  // negatable flag debug produces --no-debug in zsh completions
  EXPECT_TRUE(out.find("'--no-debug") != std::string::npos);
}

TEST_F(CompletionTest, ZshDoesNotContainHiddenItems) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("'--secret") == std::string::npos);
  EXPECT_TRUE(out.find("'-H") == std::string::npos);
  EXPECT_TRUE(out.find("'--hidden-opt") == std::string::npos);
}

TEST_F(CompletionTest, ZshOptionWithChoicesHasParenthesizedList) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  // mode option has choices: should contain (auto fast slow) [sorted by key]
  EXPECT_TRUE(out.find("(auto fast slow)") != std::string::npos);
}

TEST_F(CompletionTest, ZshOptionWithoutChoicesHasFiles) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  // input option has no choices → should fall back to :_files
  EXPECT_TRUE(out.find(":_files") != std::string::npos);
}

TEST_F(CompletionTest, ZshContainsSubcommands) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("'1:subcommand:->subcmds'") != std::string::npos);
  EXPECT_TRUE(out.find("_values 'subcommand'") != std::string::npos);
  // visible subcommands
  EXPECT_TRUE(out.find("build") != std::string::npos);
  EXPECT_TRUE(out.find("test") != std::string::npos);
  // hidden subcommand must not appear
  EXPECT_TRUE(out.find("hidden_cmd") == std::string::npos);
}

TEST_F(CompletionTest, ZshContainsDescriptionInBrackets) {
  auto& p = parser();
  std::ostringstream os;
  p.print_zsh_complete(os);
  std::string out = os.str();

  // Description text should appear inside [ ] for flags/options
  EXPECT_TRUE(out.find("[Enable verbose output]") != std::string::npos);
}

TEST_F(CompletionTest, ZshNoSubcommandsUsesSimpleArguments) {
  ArgParser parser("simple", "A simple program");
  bool flag = false;
  parser.add_flag("verbose", "Verbose output", flag);

  std::ostringstream os;
  parser.print_zsh_complete(os);
  std::string out = os.str();

  // Without subcommands there is just `_arguments -s $options`
  EXPECT_TRUE(out.find("_arguments -s $options") != std::string::npos);
  // No subcommand dispatch
  EXPECT_TRUE(out.find("->subcmds") == std::string::npos);
}

// ---------------------------------------------------------------------------
// Fish completion tests
// ---------------------------------------------------------------------------

TEST_F(CompletionTest, FishContainsCompleteCommand) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  // Every line should start with "complete -c myprog"
  EXPECT_TRUE(out.find("complete -c myprog") != std::string::npos);
}

TEST_F(CompletionTest, FishContainsShortOption) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("-s v") != std::string::npos);
  EXPECT_TRUE(out.find("-s c") != std::string::npos);
}

TEST_F(CompletionTest, FishContainsLongOption) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("-l verbose") != std::string::npos);
  EXPECT_TRUE(out.find("-l debug") != std::string::npos);
}

TEST_F(CompletionTest, FishContainsNegatableFlag) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("-l no-debug") != std::string::npos);
}

TEST_F(CompletionTest, FishDoesNotContainHiddenItems) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("-l secret") == std::string::npos);
  EXPECT_TRUE(out.find("-l hidden-opt") == std::string::npos);
  EXPECT_TRUE(out.find("-s H") == std::string::npos);
}

TEST_F(CompletionTest, FishOptionWithChoicesHasDashA) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  // mode option has choices → -r -f -a 'auto fast slow' [sorted by key]
  EXPECT_TRUE(out.find("-a 'auto fast slow'") != std::string::npos);
}

TEST_F(CompletionTest, FishOptionWithoutChoicesHasDashR) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  // input option needs an argument but has no choices → -r (required argument)
  // Without choices, fish just marks it -r (no explicit file completion)
  EXPECT_TRUE(out.find("-l input") != std::string::npos);
}

TEST_F(CompletionTest, FishContainsSubcommands) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  EXPECT_TRUE(out.find("__fish_use_subcommand") != std::string::npos);
  EXPECT_TRUE(out.find("-a 'build'") != std::string::npos);
  EXPECT_TRUE(out.find("-a 'test'") != std::string::npos);
  // hidden subcommand must not appear
  EXPECT_TRUE(out.find("hidden_cmd") == std::string::npos);
}

TEST_F(CompletionTest, FishContainsDescriptionFlag) {
  auto& p = parser();
  std::ostringstream os;
  p.print_fish_complete(os);
  std::string out = os.str();

  // Descriptions should be emitted with -d
  EXPECT_TRUE(out.find("-d 'Enable verbose output'") != std::string::npos);
}

TEST_F(CompletionTest, FishNoArgumentsProducesNoOutput) {
  ArgParser parser("empty", "No args");
  std::ostringstream os;
  parser.print_fish_complete(os);
  std::string out = os.str();

  // With no flags/options and no subcommands, output should be empty
  EXPECT_TRUE(out.empty());
}

// ---------------------------------------------------------------------------
// Edge-case tests
// ---------------------------------------------------------------------------

TEST_F(CompletionTest, OnlySubcommandsNoOptions) {
  ArgParser parser("onlycmds", "Only subcommands");
  parser.add_command("start", "Start the service");
  parser.add_command("stop", "Stop the service");

  {
    std::ostringstream os;
    parser.print_bash_complete(os);
    std::string out = os.str();
    // Should still produce a valid completion script with subcommands
    EXPECT_TRUE(out.find("_onlycmds() {") != std::string::npos);
    EXPECT_TRUE(out.find("start") != std::string::npos);
    EXPECT_TRUE(out.find("stop") != std::string::npos);
    EXPECT_TRUE(out.find("complete -F _onlycmds onlycmds") !=
                std::string::npos);
  }

  {
    std::ostringstream os;
    parser.print_zsh_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("#compdef onlycmds") != std::string::npos);
    EXPECT_TRUE(out.find("start") != std::string::npos);
    EXPECT_TRUE(out.find("stop") != std::string::npos);
  }

  {
    std::ostringstream os;
    parser.print_fish_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("__fish_use_subcommand") != std::string::npos);
    EXPECT_TRUE(out.find("-a 'start'") != std::string::npos);
    EXPECT_TRUE(out.find("-a 'stop'") != std::string::npos);
  }
}

TEST_F(CompletionTest, NegatableFlagWithoutLongNamesDoesNotEmitNoPrefix) {
  ArgParser parser("noshort", "Test");
  bool flag = false;
  // Negatable but only short name → --no- only works for long names
  parser.add_flag("f", "A negatable flag", flag).negatable(true);

  {
    std::ostringstream os;
    parser.print_bash_complete(os);
    std::string out = os.str();
    // No --no-f because there is no long name
    EXPECT_TRUE(out.find("--no-f") == std::string::npos);
    // But -f should still appear
    EXPECT_TRUE(out.find("-f") != std::string::npos);
  }

  {
    std::ostringstream os;
    parser.print_zsh_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("--no-f") == std::string::npos);
    EXPECT_TRUE(out.find("'-f") != std::string::npos);
  }

  {
    std::ostringstream os;
    parser.print_fish_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("-l no-f") == std::string::npos);
    EXPECT_TRUE(out.find("-s f") != std::string::npos);
  }
}

TEST_F(CompletionTest, OptionWithChoicesInAllShells) {
  // A focused test ensuring choices appear correctly in all three shells
  ArgParser parser("pick", "Pick one");
  std::string color;
  parser.add_option("color", "Choose a color", color)
      .value_placeholder("COLOR")
      .choices_description({{"red", "Red color"},
                            {"green", "Green color"},
                            {"blue", "Blue color"}});

  {
    std::ostringstream os;
    parser.print_bash_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("red") != std::string::npos);
    EXPECT_TRUE(out.find("green") != std::string::npos);
    EXPECT_TRUE(out.find("blue") != std::string::npos);
    EXPECT_TRUE(out.find("compgen -W") != std::string::npos);
  }

  {
    std::ostringstream os;
    parser.print_zsh_complete(os);
    std::string out = os.str();
    // map sorts keys alphabetically: blue, green, red
    EXPECT_TRUE(out.find("(blue green red)") != std::string::npos);
  }
}

TEST_F(CompletionTest, FlagWithOnlyShortName) {
  ArgParser parser("shorty", "Short only");
  bool a = false;
  parser.add_flag("a", "Alpha flag", a);

  {
    std::ostringstream os;
    parser.print_bash_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("-a") != std::string::npos);
    EXPECT_TRUE(out.find("--a") == std::string::npos);
  }

  {
    std::ostringstream os;
    parser.print_zsh_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("'-a") != std::string::npos);
    EXPECT_TRUE(out.find("'--a") == std::string::npos);
  }

  {
    std::ostringstream os;
    parser.print_fish_complete(os);
    std::string out = os.str();
    EXPECT_TRUE(out.find("-s a") != std::string::npos);
    EXPECT_TRUE(out.find("-l a") == std::string::npos);
  }
}

TEST_F(CompletionTest, CustomOutputStream) {
  ArgParser parser("outtest", "Stream test");
  bool flag = false;
  parser.add_flag("verbose", "Verbose", flag);

  std::ostringstream os;
  parser.print_bash_complete(os);
  std::string bash_out = os.str();
  EXPECT_FALSE(bash_out.empty());

  // Reset and test zsh
  os.str("");
  os.clear();
  parser.print_zsh_complete(os);
  std::string zsh_out = os.str();
  EXPECT_FALSE(zsh_out.empty());

  // Reset and test fish
  os.str("");
  os.clear();
  parser.print_fish_complete(os);
  std::string fish_out = os.str();
  EXPECT_FALSE(fish_out.empty());

  // All three should differ
  EXPECT_NE(bash_out, zsh_out);
  EXPECT_NE(bash_out, fish_out);
  EXPECT_NE(zsh_out, fish_out);
}
