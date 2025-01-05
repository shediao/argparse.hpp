#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "argparser.hpp"

using namespace argparse;

class GNUCoreutilsTest : public ::testing::Test {
   protected:
    void SetUp() override {}
    void TearDown() override {}

    template <typename... Args>
    std::vector<const char*> make_args(Args... args) {
        return {args...};
    }
};

// 文件操作命令测试
TEST_F(GNUCoreutilsTest, LSCommandTest) {
    auto args =
        make_args("ls", "-l", "-a", "-h", "--color=auto", "/path/to/dir");
    bool list_long = false;
    bool show_all = false;
    bool human_readable = false;
    std::string color_mode;
    std::vector<std::string> paths;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("l", "List in long format", list_long);
    parser.add_flag("a,all", "Show hidden entries", show_all);
    parser.add_flag("h,human-readable", "Human readable sizes", human_readable);
    parser.add_option("color", "Colorize output", color_mode);
    parser.add_positional("paths", "Paths to list", paths);

    parser.parse();

    EXPECT_TRUE(list_long);
    EXPECT_TRUE(show_all);
    EXPECT_TRUE(human_readable);
    EXPECT_EQ(color_mode, "auto");
    ASSERT_EQ(paths.size(), 1);
    EXPECT_EQ(paths[0], "/path/to/dir");
}

TEST_F(GNUCoreutilsTest, CPCommandTest) {
    auto args =
        make_args("cp", "-r", "-v", "--preserve=all", "source.txt", "dest.txt");
    bool recursive = false;
    bool verbose = false;
    std::vector<std::string> preserve;
    std::vector<std::string> files;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("r,recursive", "Copy directories recursively", recursive);
    parser.add_flag("v,verbose", "Explain what is being done", verbose);
    parser.add_option("preserve", "Preserve attributes", preserve);
    parser.add_positional("files", "Source and destination", files);

    parser.parse();

    EXPECT_TRUE(recursive);
    EXPECT_TRUE(verbose);
    ASSERT_EQ(preserve.size(), 1);
    EXPECT_EQ(preserve[0], "all");
    ASSERT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "source.txt");
    EXPECT_EQ(files[1], "dest.txt");
}

// 文本处理命令测试
TEST_F(GNUCoreutilsTest, GrepCommandTest) {
    auto args = make_args("grep", "-i", "-n", "--color=always", "pattern",
                          "file1.txt", "file2.txt");
    bool ignore_case = false;
    bool line_number = false;
    std::string color;
    std::string pattern;
    std::vector<std::string> files;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("i,ignore-case", "Ignore case distinctions", ignore_case);
    parser.add_flag("n,line-number", "Print line number", line_number);
    parser.add_option("color", "Colorize output", color);
    parser.add_positional("pattern", "Search pattern", pattern);
    parser.add_positional("files", "Input files", files);

    parser.parse();

    EXPECT_TRUE(ignore_case);
    EXPECT_TRUE(line_number);
    EXPECT_EQ(color, "always");
    EXPECT_EQ(pattern, "pattern");
    ASSERT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "file1.txt");
    EXPECT_EQ(files[1], "file2.txt");
}

// 系统信息命令测试
TEST_F(GNUCoreutilsTest, PSCommandTest) {
    auto args = make_args("ps", "-e", "-f", "--sort=-%cpu");
    bool show_all = false;
    bool full_format = false;
    std::string sort_key;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("e", "Show all processes", show_all);
    parser.add_flag("f", "Full format listing", full_format);
    parser.add_option("sort", "Sort by key", sort_key);

    parser.parse();

    EXPECT_TRUE(show_all);
    EXPECT_TRUE(full_format);
    EXPECT_EQ(sort_key, "-%cpu");
}

// 文件系统命令测试
TEST_F(GNUCoreutilsTest, DFCommandTest) {
    auto args = make_args("df", "-h", "-T", "--exclude-type=tmpfs", "/");
    bool human_readable = false;
    bool show_type = false;
    std::vector<std::string> exclude_types;
    std::vector<std::string> paths;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("h,human-readable", "Human readable sizes", human_readable);
    parser.add_flag("T,print-type", "Print file system type", show_type);
    parser.add_option("exclude-type", "Exclude fs type", exclude_types);
    parser.add_positional("paths", "Paths to show", paths);

    parser.parse();

    EXPECT_TRUE(human_readable);
    EXPECT_TRUE(show_type);
    ASSERT_EQ(exclude_types.size(), 1);
    EXPECT_EQ(exclude_types[0], "tmpfs");
    ASSERT_EQ(paths.size(), 1);
    EXPECT_EQ(paths[0], "/");
}

// 文本工具测试
TEST_F(GNUCoreutilsTest, SortCommandTest) {
    auto args = make_args("sort", "-n", "-r", "-k2,2", "-t:", "file.txt");
    bool numeric_sort = false;
    bool reverse = false;
    std::vector<std::pair<int, int>> key_positions;
    std::string field_sep = "\0";
    std::vector<std::string> files;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("n,numeric-sort", "Sort numerically", numeric_sort);
    parser.add_flag("r,reverse", "Reverse sort", reverse);
    parser.add_option("k,key", "Sort key positions", key_positions, ',');
    parser.add_option("t,field-separator", "Field separator", field_sep);
    parser.add_positional("files", "Input files", files);

    parser.parse();

    EXPECT_TRUE(numeric_sort);
    EXPECT_TRUE(reverse);
    ASSERT_EQ(key_positions.size(), 1);
    EXPECT_EQ(key_positions[0].first, 2);
    EXPECT_EQ(key_positions[0].second, 2);
    EXPECT_EQ(field_sep, ":");
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "file.txt");
}

// 系统工具测试
TEST_F(GNUCoreutilsTest, ChmodCommandTest) {
    auto args = make_args("chmod", "-R", "--reference=ref.txt", "644",
                          "file1.txt", "file2.txt");
    bool recursive = false;
    std::string reference_file;
    std::string mode;
    std::vector<std::string> files;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("R,recursive", "Change files recursively", recursive);
    parser.add_option("reference", "Reference file", reference_file);
    parser.add_positional("mode", "File mode", mode);
    parser.add_positional("files", "Target files", files);

    parser.parse();

    EXPECT_TRUE(recursive);
    EXPECT_EQ(reference_file, "ref.txt");
    EXPECT_EQ(mode, "644");
    ASSERT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "file1.txt");
    EXPECT_EQ(files[1], "file2.txt");
}

// 实用工具测试
TEST_F(GNUCoreutilsTest, Base64CommandTest) {
    auto args =
        make_args("base64", "-w76", "-i", "input.txt", "-o", "output.txt");
    int wrap_cols = 0;
    bool ignore_garbage = false;
    std::string input_file;
    std::string output_file;

    ArgParser parser(args.size(), args.data());
    parser.add_option("w,wrap-cols", "Wrap lines at cols", wrap_cols);
    parser.add_flag("i,ignore-garbage", "Ignore non-alphabet chars",
                    ignore_garbage);
    parser.add_option("o,output", "Output file", output_file);
    parser.add_positional("input", "Input file", input_file);

    parser.parse();

    EXPECT_EQ(wrap_cols, 76);
    EXPECT_TRUE(ignore_garbage);
    EXPECT_EQ(input_file, "input.txt");
    EXPECT_EQ(output_file, "output.txt");
}
