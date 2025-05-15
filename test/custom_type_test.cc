#include <gtest/gtest.h>

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "argparse/argparse.hpp"

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

// std::filesystem
TEST_F(ArgParserTest, FileSystem) {
    std::filesystem::path path;
    ArgParser parser("test", "Test filesystem path parsing");
    parser.add_option("p,path", "Path to file", path);

    auto args = make_args("--path", "/tmp/test.txt");
    parser.parse(args.size(), args.data());

    EXPECT_EQ(path.string(), "/tmp/test.txt");
}

// Custom type test
struct Point {
    int x, y;

    Point() : x(0), y(0) {}
    explicit Point(const std::string& s) {
        std::istringstream iss(s);
        char comma;
        iss >> x >> comma >> y;
        if (comma != ',' || iss.fail()) {
            throw std::invalid_argument("Invalid point format. Expected: x,y");
        }
    }

    std::string toString() const {
        return std::to_string(x) + "," + std::to_string(y);
    }
};

TEST_F(ArgParserTest, CustomType) {
    Point point;
    std::vector<Point> points;

    ArgParser parser("test", "Test custom type parsing");
    parser.add_option("point", "Single point (x,y)", point);
    parser.add_option("points", "Multiple points (x,y)", points);

    auto args =
        make_args("--point", "10,20", "--points", "1,2", "--points", "3,4");
    parser.parse(args.size(), args.data());

    EXPECT_EQ(point.x, 10);
    EXPECT_EQ(point.y, 20);
    EXPECT_EQ(points.size(), 2);
    EXPECT_EQ(points[0].toString(), "1,2");
    EXPECT_EQ(points[1].toString(), "3,4");
}
