## Feature Comparison

| Feature               | argparse.hpp | [cxxopts](https://github.com/jarro2783/cxxopts) | [CLI11](https://github.com/CLIUtils/CLI11) | [Boost](https://www.boost.org/doc/libs/1_84_0/doc/html/program_options.html) | [gflags](https://github.com/gflags/gflags) |
| --------------------- | ------------ | ----------------------------------------------- | ------------------------------------------ | -------------------------------------------------------------------------------------------- | ------------------------------------------ |
| Header-only           | ✅           | ✅                                              | ✅                                         | ❌                                                                                           | ❌                                         |
| Dependencies          | None         | None                                            | None                                       | Boost                                                                                        | None                                       |
| C++ Standard          | C++20        | C++11                                           | C++11                                      | C++11                                                                                        | C++11                                      |
| GNU-style options     | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |
| Positional arguments  | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |
| Default values        | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ✅                                         |
| Type-safe             | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ✅                                         |
| Container support     | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |
| Custom types          | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |
| Subcommands           | ✅           | ❌                                              | ✅                                         | ❌                                                                                           | ❌                                         |
| Config file           | ❌           | ❌                                              | ✅                                         | ✅                                                                                           | ✅                                         |
| Help generation       | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ✅                                         |
| Unicode support       | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |
| Combined flags (-abc) | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |
| Required options      | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ✅                                         |
| Option groups         | ❌           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |
| Validation            | ✅           | ✅                                              | ✅                                         | ✅                                                                                           | ❌                                         |

## Quick start

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

Each flag, option, and positional argument is bound to an actual variable, which can be of the following types:
- Fundamental C++ types (`bool`, `int`, `long`, `double`, etc.)
- Custom types (must be constructible from `std::string`)
- Tuple-like types (`std::pair`, `std::tuple`, `std::array`)
- Containers of any of the above element types
- std::optional<T>, T is any of the above types(Fundamental,string-constructible,tuple-like,container)

## Basics

```cpp
#include <argparse/argparse.hpp>  // cmake Dependencies
#include "argparse.hpp"           // copy to your project
```

```cpp

struct ParsedArgs {
    std::string prompt;
    std::optional<std::string> system_prompt;
    std::optional<std::string> model;
    std::optional<std::string> api_key;
    std::optional<std::string> api_url;
    std::optional<std::string> proxy;
    std::optional<double> temperature;
    std::optional<double> top_p;
    bool stream = false;
    bool interactive = false;
    bool debug = false;
    bool verbose = false;
    bool help = false;
    bool version = false;
};

int main(int argc, const char* argv[]) {
    argparse::ArgParser parser("openai-cli",
                               "OpenAI API Compatible Command Line Chatbot");
    ParsedArgs args;

    parser.add_flag("d,debug", "Enable debug mode", args.debug);
    parser.add_flag("h,help", "Show help", args.help);
    parser.add_flag("i,interactive", "Enable interactive mode",
                    args.interactive);
    parser.add_negative_flag("I", "Disable interactive mode", args.interactive);
    parser.add_flag("stream", "Enable streaming mode", args.stream)
        .negatable();  // --no-stream
    parser.add_flag("v,verbose", "Enable verbose mode", args.verbose);
    parser.add_flag("version", "Show version", args.version);

    parser.add_option("k,key", "OpenAI API key", args.api_key);
    parser.add_option("m,model", "Model to use", args.model)
        .default_value("gpt-3.5-turbo");
    parser.add_option("p,prompt", "Prompt", args.prompt);
    parser.add_option("proxy", "Use proxy", args.proxy);
    parser.add_option("s,system-prompt", "System prompt", args.system_prompt);
    parser.add_option("t,temperature", "Model temperature", args.temperature);
    parser.add_option("top-p", "Model top-p parameter", args.top_p);
    parser.add_option("u,url", "OpenAI API Compatible URL", args.api_url)
        .default_value("https://api.openai.com/v1/chat/completions");
    parser.add_positional("prompt", "Prompt", args.prompt);

    try {
        parser.parse(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
```

## Usage

### 1. cmake

```
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/shediao/argparse.hpp
    GIT_TAG v0.0.5
)
FetchContent_MakeAvailable(argparse)



target_link_libraries(xxx PRIVATE argparse::argparse ...)
```

OR

```
git clone https://github.com/shediao/argparse.hpp /path/to/argparse.hpp
add_subdirectory(/path/to/argparse.hpp)
```

```
#include <argparse.hpp>
```

### 2. others

copy `argparse.hpp` to myproject/dir/include

## docs
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/shediao/argparse.hpp)
