

# Quick start

This is a lightweight & typesafe C++ option parser library, supporting the standard GNU style syntax for options.

Options can be given as:

    -a
    -ab
    --long
    -c argument
    --long-opt argument
    --long-opt=argument
    -abc argument

where c takes an argument, but a and b do not.

Additionally, anything after `--` will be parsed as a positional argument.

## Basics

```cpp
#include <argparse.hpp>
```


```cpp
std::vector<const char*> args{"ls", "-l", "-a", "-h", "--color=auto", "/path/to/dir"};
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
```
