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
  std::optional<std::string> reasoning_effort; // Added based on usage
};

int main(int argc, const char* argv[]) {
  argparse::ArgParser parser("openai-cli",
                             "OpenAI API Compatible Command Line Chatbot");
  ParsedArgs args;

  parser.add_flag("d,debug", "Enable debug mode", args.debug);
  parser.add_flag("h,help", "Show help", args.help);

  parser.add_flag("i,interactive", "Enable interactive mode", args.interactive);
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

  parser.add_option("t,temperature", "Model temperature", args.temperature)
      .range(0, 2.0);
  parser.add_option("top-p", "Model top-p parameter", args.top_p)
      .range(0.0, 1.0);

  parser
      .add_option("reasoning-effort", "reasoning effort", args.reasoning_effort)
      .choices({"low", "medium", "high", "none"});

  parser.add_option("u,url", "OpenAI API Compatible URL", args.api_url)
      .default_value("https://api.openai.com/v1/chat/completions");

  parser.add_positional("prompt", "Prompt", args.prompt)
      .checker([](const std::string& val) { return !val.empty(); },
               "prompt is a non empty string");

  try {
    parser.parse(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 1;
  }
}
```

## API Details

This section provides more details on the primary methods for defining command-line arguments.

### `add_flag`

The `add_flag` method is used to define a boolean flag. When the flag is present in the command-line arguments, the bound variable will be set to `true`. By default, flags are `false` unless specified.

-   **Syntax**: `parser.add_flag(names, description, variable_to_bind);`
-   **`names`**: A string containing comma-separated short and long names (e.g., `"h,help"`, `"verbose"`). A single name is also valid (e.g., `"debug"`).
-   **`description`**: A string describing the flag's purpose, used in generated help messages.
-   **`variable_to_bind`**: A reference to a `bool` variable (which will be set to `true` or `false` based on the flag's presence or explicit negation) or an integral variable (which can be incremented/decremented).

**Example:**

```cpp
struct MyArgs {
  bool verbose = false;
  bool enable_feature = false;
};

MyArgs args;
argparse::ArgParser parser("my_program");

// Defines flags: -v, --verbose
parser.add_flag("v,verbose", "Enable verbose output", args.verbose);
// Defines flag: --enable-feature
parser.add_flag("enable-feature", "Enable a specific feature", args.enable_feature);

// After parser.parse(argc, argv);
// if "./my_program -v" is called, args.verbose will be true.
// if "./my_program --enable-feature" is called, args.enable_feature will be true.
```

**Negatable Flags:**

The library provides ways to handle flags that can be explicitly enabled or disabled:

1.  **`add_negative_flag(names, description, variable_to_bind)`**:
    This is useful if a feature is enabled by default (variable initialized to `true`), and you want a flag to disable it (set variable to `false`). The bound variable must be a `bool`.
    ```cpp
    struct Settings {
      bool debug = true; // Enabled by default
    };
    Settings settings;
    parser.add_flag("debug", "Debug", settings.debug);
    parser.add_negative_flag("release", "Release", settings.debug);
    // If --debug is passed, settings.debug becomes true.
    // If --release is passed, settings.debug becomes false.
    ```

2.  **`.negatable()` modifier**:
    Chain this to an `add_flag` call. If your flag is named `--feature`, this automatically creates a `--no-feature` counterpart. The bound `bool` variable should typically be initialized to `false`.
    ```cpp
    struct Settings {
      bool use_gpu = false; // Disabled by default
    };
    Settings settings;
    parser.add_flag("gpu", "Enable GPU acceleration", settings.use_gpu)
          .negatable(); // Creates --gpu and --no-gpu
    // --gpu sets use_gpu to true.
    // --no-gpu sets use_gpu to false.
    ```

### `add_option`

The `add_option` method defines an option that expects an argument (e.g., `--file <filename>`, `-n <number>`).

-   **Syntax**: `parser.add_option(names, description, variable_to_bind);`
-   **`names`**: Comma-separated short and long names (e.g., `"f,file"`, `"output"`).
-   **`description`**: A string describing the option's purpose.
-   **`variable_to_bind`**: A reference to a variable where the option's argument value will be stored.

**Bindable Types:**
`add_option` can bind to a wide variety of types:
-   Fundamental C++ types: `bool` (for options taking an explicit true/false like `--enable=true`), `int`, `long`, `double`, `float`, `char`, `std::string`.
-   `std::optional<T>`: For options that are not required. `T` can be any of the other supported types here. If the option is not provided, the `std::optional` remains `std::nullopt`.
-   Containers (e.g., `std::vector<T>`, `std::list<T>`, `std::set<T>`): The option can be specified multiple times on the command line (e.g., `-I /path1 -I /path2`), and all values will be collected into the container. `T` can be a fundamental, custom, or string-constructible tuple-like type.
-   Custom types: User-defined types that are:
    -   Constructible from `std::string` (e.g., `MyType(const std::string& s)`). The library primarily uses this mechanism for custom types.
-   Tuple-like types (`std::pair`, `std::tuple`, `std::array`): Generally supported if their elements are string-constructible, allowing parsing from a single argument string (e.g., "val1,val2" with a specified delimiter).

**Example:**

```cpp
struct AppSettings {
  std::string output_file;
  std::optional<int> retries;
  double threshold;
  std::vector<std::string> include_paths;
  // Assuming MyCustomType has a string constructor
  // MyCustomType custom_setting;
};

AppSettings settings;
argparse::ArgParser parser("my_app");

// To make an option effectively required, bind to a non-optional type
// and do not provide a default_value. Parsing will expect it or an error may occur
// if your program logic depends on it being set.
// Or, use .checker() to ensure a value is provided and valid.
parser.add_option("o,output", "Specify output file", settings.output_file);
// If "o,output" is critical, consider checking settings.output_file after parse,
// or use .checker() if specific validation (like non-empty) is needed.

parser.add_option("r,retries", "Number of retries for an operation", settings.retries);
// If -r 5 is given, settings.retries will contain 5.
// If -r is not given, settings.retries will be std::nullopt.

parser.add_option("t,threshold", "Set threshold value", settings.threshold)
      .default_value("0.5"); // Provides a default if the option isn't specified.

parser.add_option("I,include", "Include path (can be specified multiple times)", settings.include_paths);
// Command: ./my_app -I /usr/local/include -I ./project/include
// settings.include_paths will contain {"/usr/local/include", "./project/include"}

// Example for a custom type (MyCustomType would need to be defined)
// struct MyCustomType {
//   std::string value;
//   MyCustomType() = default;
//   MyCustomType(const std::string& s) : value("processed:" + s) {}
// };
// MyCustomType custom_val;
// parser.add_option("custom", "Custom value processed from string", custom_val);
// ./my_app --custom hello -> custom_val.value would be "processed:hello"
```

**Common Modifiers for Options (and Positionals):**
These methods are chained after `add_option` or `add_positional`:
-   `.default_value(value)`: Sets a default value if the option/argument is not provided. The type of `value` must std::string.
-   `.choices({val1, val2, ...})`: Restricts the argument to one of the specified choices. Works for string, numeric, and custom types that are comparable.
    ```cpp
    // std::string mode;
    // parser.add_option("mode", "Operation mode", mode)
    //       .choices({"fast", "slow", "balanced"});
    ```
-   `.range(min, max)`: Restricts numerical arguments to a specific range (inclusive).
    ```cpp
    // int percentage;
    // parser.add_option("p,percentage", "Percentage value", percentage)
    //       .range(0, 100);
    ```
-   `.callback(callback_function)`: Defines a custom callback function (`std::function<void(const ParsedValueType&)>`) to be executed with the parsed value when the option/argument is parsed. `ParsedValueType` is the type of the bound variable (or its `value_type` for `std::optional` or containers).
-   `.checker(predicate_function, error_message)`: Provides custom validation logic. One version takes a predicate (`std::function<bool(const std::string&)>`) that operates on the raw string value before parsing. Another version (more commonly used for value validation) takes a predicate (`std::function<bool(const ParsedValueType&)>`) that operates on the parsed value. If the predicate returns `false`, parsing fails with the given `error_message`.
    ```cpp
    // std::string filename;
    // parser.add_option("f,file", "Input file", filename)
    //       .checker([](const std::string& fname) { return !fname.empty() && fname.length() > 3; },
    //                "Filename must not be empty and longer than 3 chars.");
    ```

### `add_positional`

The `add_positional` method defines a positional argument. These are arguments that are not preceded by a flag or option name (e.g., `my_program input.txt output.txt`). Their order on the command line matters.

-   **Syntax**: `parser.add_positional(name, description, variable_to_bind);`
-   **`name`**: A symbolic name for the positional argument (used in help messages and error reporting).
-   **`description`**: A string describing the argument's purpose.
-   **`variable_to_bind`**: A reference to a variable where the argument's value will be stored.

**Bindable Types:**
Bindable types are the same as for `add_option` (Fundamental types, `std::optional<T>`, Containers, Custom types, Tuple-like types).

**Example:**

```cpp
struct FileCopyArgs {
  std::string source_file;
  std::string destination_file;
  std::optional<int> log_level;
  std::vector<std::string> additional_files;
};

FileCopyArgs fc_args;
argparse::ArgParser parser("copy_tool");

// For required positional arguments, bind to a non-optional type and do not provide a default.
// The parser will expect them in order. Use .checker() for further validation if needed.
parser.add_positional("source", "Source file to copy", fc_args.source_file);
parser.add_positional("destination", "Destination path", fc_args.destination_file);

parser.add_positional("log-level", "Optional logging level (integer)", fc_args.log_level);
// Usage: ./copy_tool src.txt dst.txt 3  -> fc_args.log_level will contain 3
// Usage: ./copy_tool src.txt dst.txt    -> fc_args.log_level will be std::nullopt

// Example with a variadic (greedy) positional argument for multiple files:
// argparse::ArgParser variadic_parser("process_items");
// variadic_parser.add_positional("items", "Items to process", fc_args.additional_files);
// Command: ./process_items item1.txt item2.txt item3.txt
// fc_args.additional_files would contain {"item1.txt", "item2.txt", "item3.txt"}
```
**Important Notes on Positional Arguments:**
-   The order of definition for `add_positional` calls must match the expected order on the command line.
-   A required positional argument cannot follow an optional positional argument or a variadic (container-based) positional argument.
-   Typically, you can have at most one variadic positional argument, and it should be the last one defined.

**Modifiers:**
Positional arguments can also use modifiers like `.required()`, `.choices({...})`, `.checker(...)`, and `.default_value(...)` (for optional positionals) similar to `add_option`.

### Subcommands

`argparse.hpp` supports subcommands (also known as subparsers), allowing you to create complex command-line interfaces where different actions have their own distinct sets of options and positional arguments (e.g., `docker image ls`, `docker container run`).

To define a subcommand, you call `add_subparser()` on an existing `ArgParser` instance. This returns a reference to a new `ArgParser` object, which you then configure with its own flags, options, and positional arguments.

**Example:**

```cpp
#include <argparse/argparse.hpp>
#include <iostream>
#include <string>
#include <vector>    // Required for std::vector
#include <optional>  // Required for std::optional

// Common arguments for the main command and all subcommands
struct GlobalArgs {
    bool verbose = false;
};

// Arguments specific to the 'clone' subcommand
struct CloneSubcommandArgs {
    std::string repository_url;
    std::optional<std::string> target_directory;
    std::optional<int> depth; // Use optional for clarity if 0 is a valid depth
};

// Arguments specific to the 'commit' subcommand
struct CommitSubcommandArgs {
    std::string message;
    bool all_changes = false;
};

int main(int argc, const char* argv[]) {
    argparse::ArgParser program("git-like-cli", "A CLI with subcommands");
    GlobalArgs global_args;

    program.add_flag("v,verbose", "Enable verbose output", global_args.verbose);

    // --- Define the 'clone' subcommand ---
    CloneSubcommandArgs clone_args;
    auto& clone_parser = program.add_command("clone", "Clone a repository");
    clone_parser.add_positional("repo-url", "Repository URL", clone_args.repository_url); // Effectively required if not optional and no default
    clone_parser.add_positional("dir", "Target directory (optional)", clone_args.target_directory);
    clone_parser.add_option("depth", "Clone depth", clone_args.depth);

    // --- Define the 'commit' subcommand ---
    CommitSubcommandArgs commit_args;
    auto& commit_parser = program.add_command("commit", "Record changes");
    commit_parser.add_option("m,message", "Commit message", commit_args.message); // Effectively required
    commit_parser.add_flag("a,all", "Stage all modified/deleted files", commit_args.all_changes);

    argparse::Command* active_command = nullptr;
    try {
        // The parse method returns a reference to the activated command (main or subcommand)
        active_command = &program.parse(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        // Print usage for the main program in case of error during parsing.
        // The library automatically prints help for --help/-h for the relevant command/subcommand.
        std::cerr << program.usage() << std::endl;
        return 1;
    }

    if (active_command) {
        if (active_command->name() == "clone") {
            // clone_args are populated by the parse call if 'clone' was the subcommand
            std::cout << "Executing 'clone'..." << std::endl;
            if (global_args.verbose) std::cout << "Verbose mode." << std::endl;
            std::cout << "Repo: " << clone_args.repository_url << std::endl;
            if (clone_args.target_directory) std::cout << "Dir: " << *clone_args.target_directory << std::endl;
            if (clone_args.depth) std::cout << "Depth: " << *clone_args.depth << std::endl;
        } else if (active_command->name() == "commit") {
            // commit_args are populated
            std::cout << "Executing 'commit'..." << std::endl;
            if (global_args.verbose) std::cout << "Verbose mode." << std::endl;
            std::cout << "Message: " << commit_args.message << std::endl;
            if (commit_args.all_changes) std::cout << "All changes." << std::endl;
        } else if (active_command->name() == program.name()) {
            // No subcommand used, or the main command itself was invoked.
            // Handle global flags or show help.
            if (global_args.verbose) std::cout << "Verbose mode (no subcommand)." << std::endl;

            // If program.parse() was called with no arguments (argc == 1),
            // and no default actions/subcommands are defined,
            // the built-in --help usually handles showing usage.
            // This explicit check might be for custom behavior for 'my_program' with no args.
            if (argc == 1) {
                std::cout << program.usage() << std::endl;
            }
            // Note: --help or --version flags (if defined as in the "Basics" example)
            // usually cause the program to print help/version and exit, handled by their callbacks.
        }
    }

    return 0;
}
```

**Using Subcommands:**
-   Call `program.parse(argc, argv)`. This method returns a reference to the `argparse::Command` object that was activated (either the main program's parser or a subcommand's parser).
-   Check the `name()` of the returned command object (e.g., `active_command->name() == "subcommand_name"`) to determine which subcommand was invoked.
-   Arguments bound to the main parser (like `global_args.verbose`) are parsed and available regardless of the subcommand.
-   Arguments specific to a subcommand are only parsed if that subcommand is used.
-   Help messages are context-aware: `my_program subcommand --help` shows help for that subcommand.

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

## TODO
- [ ] bash/zsh completion
- [ ] windows style

## docs
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/shediao/argparse.hpp)
