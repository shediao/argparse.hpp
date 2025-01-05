#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

#include "argparser.hpp"

using namespace argparse;

class ArgParserTest : public ::testing::Test {
   protected:
    void SetUp() override {}
    void TearDown() override {}

    template <typename... Args>
    std::vector<const char*> make_args(Args... args) {
        return {args...};
    }
};

// Flag 测试
TEST_F(ArgParserTest, BoolFlagTest) {
    auto args = make_args("prog", "-v", "--verbose");
    bool v1 = false, v2 = false;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("v", "Verbose mode 1", v1);
    parser.add_flag("verbose", "Verbose mode 2", v2);

    parser.parse();

    EXPECT_TRUE(v1);
    EXPECT_TRUE(v2);
}

TEST_F(ArgParserTest, IntFlagTest) {
    auto args = make_args("prog", "-v", "-v", "--verbose");
    int count1 = 0, count2 = 0;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("v", "Counter 1", count1, increment<int>);
    parser.add_flag("verbose", "Counter 2", count2, increment<int>);

    parser.parse();

    EXPECT_EQ(count1, 2);
    EXPECT_EQ(count2, 1);
}

// Option 测试
TEST_F(ArgParserTest, StringOptionTest) {
    auto args = make_args("prog", "-o", "output.txt", "--input=input.txt");
    std::string output, input;

    ArgParser parser(args.size(), args.data());
    parser.add_option("o", "Output file", output);
    parser.add_option("input", "Input file", input);

    parser.parse();

    EXPECT_EQ(output, "output.txt");
    EXPECT_EQ(input, "input.txt");
}

TEST_F(ArgParserTest, NumberOptionTest) {
    auto args = make_args("prog", "-n", "42", "--float=3.14");
    int number = 0;
    float pi = 0.0f;

    ArgParser parser(args.size(), args.data());
    parser.add_option("n", "Some number", number);
    parser.add_option("float", "Some float", pi);

    parser.parse();

    EXPECT_EQ(number, 42);
    EXPECT_FLOAT_EQ(pi, 3.14f);
}

// 容器类型测试
TEST_F(ArgParserTest, VectorOptionTest) {
    auto args = make_args("prog", "-n", "1", "-n", "2", "-n", "3");
    std::vector<int> numbers;

    ArgParser parser(args.size(), args.data());
    parser.add_option("n", "Numbers", numbers);

    parser.parse();

    ASSERT_EQ(numbers.size(), 3);
    EXPECT_EQ(numbers[0], 1);
    EXPECT_EQ(numbers[1], 2);
    EXPECT_EQ(numbers[2], 3);
}

// Tuple 类型测试
TEST_F(ArgParserTest, TupleOptionTest) {
    auto args = make_args("prog", "--point", "1,2");
    std::tuple<int, int> point;

    ArgParser parser(args.size(), args.data());
    parser.add_option("point", "A point", point, ',');

    parser.parse();

    EXPECT_EQ(std::get<0>(point), 1);
    EXPECT_EQ(std::get<1>(point), 2);
}

// Positional 参数测试
TEST_F(ArgParserTest, PositionalTest) {
    auto args = make_args("prog", "input.txt", "output.txt");
    std::string input, output;

    ArgParser parser(args.size(), args.data());
    parser.add_positional("input", "Input file", input);
    parser.add_positional("output", "Output file", output);

    parser.parse();

    EXPECT_EQ(input, "input.txt");
    EXPECT_EQ(output, "output.txt");
}

TEST_F(ArgParserTest, PositionalVectorTest) {
    auto args = make_args("prog", "1", "2", "3", "4");
    std::vector<int> numbers;

    ArgParser parser(args.size(), args.data());
    parser.add_positional("numbers", "List of numbers", numbers);

    parser.parse();

    ASSERT_EQ(numbers.size(), 4);
    EXPECT_EQ(numbers[0], 1);
    EXPECT_EQ(numbers[1], 2);
    EXPECT_EQ(numbers[2], 3);
    EXPECT_EQ(numbers[3], 4);
}

// 错误处理测试
TEST_F(ArgParserTest, MissingOptionValueTest) {
    auto args = make_args("prog", "--name");
    std::string name;

    ArgParser parser(args.size(), args.data());
    parser.add_option("name", "Your name", name);

    EXPECT_THROW(parser.parse(), std::runtime_error);
}

TEST_F(ArgParserTest, UnknownOptionTest) {
    auto args = make_args("prog", "--unknown");

    ArgParser parser(args.size(), args.data());

    EXPECT_THROW(parser.parse(), std::runtime_error);
}

TEST_F(ArgParserTest, RequiredOptionTest) {
    auto args = make_args("prog");
    std::string required;

    ArgParser parser(args.size(), args.data());
    parser.add_option("r,required", "Required option", required);
    parser.get_arg("required")->require();

    EXPECT_THROW(parser.parse(), std::runtime_error);
}

// 组合短选项测试
TEST_F(ArgParserTest, CombinedShortOptionsTest) {
    auto args = make_args("prog", "-abc", "value");
    bool a = false, b = false;
    std::string c;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("a", "Flag A", a);
    parser.add_flag("b", "Flag B", b);
    parser.add_option("c", "Option C", c);

    parser.parse();

    EXPECT_TRUE(a);
    EXPECT_TRUE(b);
    EXPECT_EQ(c, "value");
}

// 混合使用测试
TEST_F(ArgParserTest, MixedArgumentsTest) {
    auto args =
        make_args("prog", "-v", "pos1", "--name=test", "pos2", "-n", "42");
    bool verbose = false;
    std::string name;
    int number = 0;
    std::vector<std::string> positionals;

    ArgParser parser(args.size(), args.data());
    parser.add_flag("v", "Verbose", verbose);
    parser.add_option("name", "Name", name);
    parser.add_option("n", "Number", number);
    parser.add_positional("files", "Input files", positionals);

    parser.parse();

    EXPECT_TRUE(verbose);
    EXPECT_EQ(name, "test");
    EXPECT_EQ(number, 42);
    ASSERT_EQ(positionals.size(), 2);
    EXPECT_EQ(positionals[0], "pos1");
    EXPECT_EQ(positionals[1], "pos2");
}

// Vector of Tuples 测试
TEST_F(ArgParserTest, VectorOfTuplesOptionTest) {
    auto args = make_args("prog", "-p", "1,2", "-p", "3,4", "-p", "5,6");
    std::vector<std::tuple<int, int>> points;

    ArgParser parser(args.size(), args.data());
    parser.add_option("p", "Points", points, ',');

    parser.parse();

    ASSERT_EQ(points.size(), 3);
    EXPECT_EQ(std::get<0>(points[0]), 1);
    EXPECT_EQ(std::get<1>(points[0]), 2);
    EXPECT_EQ(std::get<0>(points[1]), 3);
    EXPECT_EQ(std::get<1>(points[1]), 4);
    EXPECT_EQ(std::get<0>(points[2]), 5);
    EXPECT_EQ(std::get<1>(points[2]), 6);
}

TEST_F(ArgParserTest, VectorOfPairsOptionTest) {
    auto args = make_args("prog", "--kv", "key1:value1", "--kv", "key2:value2");
    std::vector<std::pair<std::string, std::string>> key_values;

    ArgParser parser(args.size(), args.data());
    parser.add_option("kv", "Key-Value pairs", key_values, ':');

    parser.parse();

    ASSERT_EQ(key_values.size(), 2);
    EXPECT_EQ(key_values[0].first, "key1");
    EXPECT_EQ(key_values[0].second, "value1");
    EXPECT_EQ(key_values[1].first, "key2");
    EXPECT_EQ(key_values[1].second, "value2");
}

// Vector of Tuples Positional 测试
TEST_F(ArgParserTest, VectorOfTuplesPositionalTest) {
    auto args = make_args("prog", "1,2,3", "4,5,6", "7,8,9");
    std::vector<std::tuple<int, int, int>> coordinates;

    ArgParser parser(args.size(), args.data());
    parser.add_positional("coords", "3D coordinates", coordinates, ',');

    parser.parse();

    ASSERT_EQ(coordinates.size(), 3);
    EXPECT_EQ(std::get<0>(coordinates[0]), 1);
    EXPECT_EQ(std::get<1>(coordinates[0]), 2);
    EXPECT_EQ(std::get<2>(coordinates[0]), 3);
    EXPECT_EQ(std::get<0>(coordinates[1]), 4);
    EXPECT_EQ(std::get<1>(coordinates[1]), 5);
    EXPECT_EQ(std::get<2>(coordinates[1]), 6);
    EXPECT_EQ(std::get<0>(coordinates[2]), 7);
    EXPECT_EQ(std::get<1>(coordinates[2]), 8);
    EXPECT_EQ(std::get<2>(coordinates[2]), 9);
}

TEST_F(ArgParserTest, VectorOfPairsPositionalTest) {
    auto args = make_args("prog", "name=alice", "age=20", "city=beijing");
    std::vector<std::pair<std::string, std::string>> attributes;

    ArgParser parser(args.size(), args.data());
    parser.add_positional("attributes", "User attributes", attributes, '=');

    parser.parse();

    ASSERT_EQ(attributes.size(), 3);
    EXPECT_EQ(attributes[0].first, "name");
    EXPECT_EQ(attributes[0].second, "alice");
    EXPECT_EQ(attributes[1].first, "age");
    EXPECT_EQ(attributes[1].second, "20");
    EXPECT_EQ(attributes[2].first, "city");
    EXPECT_EQ(attributes[2].second, "beijing");
}

// 混合类型的复杂测试
TEST_F(ArgParserTest, ComplexTupleTest) {
    auto args = make_args("prog", "--complex", "hello,42,3.14");
    std::tuple<std::string, int, float> complex_value;

    ArgParser parser(args.size(), args.data());
    parser.add_option("complex", "Complex tuple value", complex_value, ',');

    parser.parse();

    EXPECT_EQ(std::get<0>(complex_value), "hello");
    EXPECT_EQ(std::get<1>(complex_value), 42);
    EXPECT_FLOAT_EQ(std::get<2>(complex_value), 3.14f);
}

TEST_F(ArgParserTest, VectorOfComplexTuplesTest) {
    auto args = make_args("prog", "-t", "str1,1,1.1", "-t", "str2,2,2.2");
    std::vector<std::tuple<std::string, int, double>> values;

    ArgParser parser(args.size(), args.data());
    parser.add_option("t", "Complex tuple values", values, ',');

    parser.parse();

    ASSERT_EQ(values.size(), 2);
    EXPECT_EQ(std::get<0>(values[0]), "str1");
    EXPECT_EQ(std::get<1>(values[0]), 1);
    EXPECT_DOUBLE_EQ(std::get<2>(values[0]), 1.1);
    EXPECT_EQ(std::get<0>(values[1]), "str2");
    EXPECT_EQ(std::get<1>(values[1]), 2);
    EXPECT_DOUBLE_EQ(std::get<2>(values[1]), 2.2);
}
