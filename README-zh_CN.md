
[![cmake-multi-platform](https://github.com/shediao/argparse.hpp/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/shediao/argparse.hpp/actions/workflows/cmake-multi-platform.yml)
[![msys2](https://github.com/shediao/argparse.hpp/actions/workflows/msys2.yml/badge.svg)](https://github.com/shediao/argparse.hpp/actions/workflows/msys2.yml)

## 功能比较

| 功能 | argparse.hpp | [cxxopts](https://github.com/jarro2783/cxxopts) | [CLI11](https://github.com/CLIUtils/CLI11) | [Boost](https://www.boost.org/doc/libs/1_84_0/doc/html/program_options.html) | [gflags](https://github.com/gflags/gflags) |
| --------------------- | ------------ | ----------------------------------------------- | ------------------------------------------ | -------------------------------------------------------------------------------------------- | ------------------------------------------ |
| 仅头文件 | ✅ | ✅ | ✅ | ❌ | ❌ |
| 依赖 | 无 | 无 | 无 | Boost | 无 |
| C++ 标准 | C++20 | C++11 | C++11 | C++11 | C++11 |
| GNU风格选项 | ✅ | ✅ | ✅ | ✅ | ❌ |
| 位置参数 | ✅ | ✅ | ✅ | ✅ | ❌ |
| 默认值 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 类型安全 | ✅ | ❓ | ❓ | ❓ | ❓ |
| 容器支持 | ✅ | ✅ | ✅ | ✅ | ❌ |
| 自定义类型 | ✅ | ✅ | ✅ | ✅ | ❌ |
| 子命令 | ✅ | ❌ | ✅ | ❌ | ❌ |
| 配置文件 | ❌ | ❌ | ✅ | ✅ | ✅ |
| 帮助信息生成 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 组合标志 (-abc) | ✅ | ✅ | ✅ | ✅ | ❌ |
| 必填选项 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 选项组 | ❌ | ✅ | ✅ | ✅ | ❌ |
| 校验 | ✅ | ✅ | ✅ | ✅ | ❌ |

## 关键区别：直接变量绑定

`argparse.hpp` 相对于许多其他 C++ 命令行解析库的一个显著优势是其参数处理方式。它不要求用户在解析后通过模板函数如 `parser.get<Type>("arg_name")` 或 `results["arg_name"].as<Type>()` 来检索参数值，而是将标志、选项和位置参数直接绑定到用户定义的变量上。

这意味着：
- **编译时真正的类型安全**：在定义参数时，类型就已知并被检查。
- **访问时无需运行时类型转换/强制转换**：一旦 `parser.parse()` 被调用，绑定的变量会直接填充解析和转换后的值。您可以立即使用这些变量，无需 `get<T>()`、`cast<T>()` 或类似的访问方法。
- **更简单、更清晰的代码**：访问解析后的值就像使用变量本身一样简单（例如，`if (args.verbose) {
  ... }`，`std::string filename = args.output_file;`）。

这种设计哲学通过更有效地利用 C++ 的类型系统，使得命令行参数解析的代码更加健壮、可读和易于维护。

## 快速入门

这是一个轻量级且类型安全的 C++ 选项解析库，支持标准的 GNU 风格选项语法。

选项可以这样给出：

    -a
    -ab
    --long
    -c argument
    --long-opt argument
    --long-opt=argument
    -abc argument

其中 c 需要一个参数，而 a 和 b 不需要。

此外，`--` 之后的所有内容都将被解析为位置参数。

每个标志、选项和位置参数都绑定到一个实际变量，该变量可以是以下类型之一：
- C++ 基本类型（`bool`, `int`, `long`, `double` 等）
- 自定义类型（必须可从 `std::string` 构造）
- 元组类类型（`std::pair`, `std::tuple`, `std::array`）
- 容器（例如，`std::vector<U>`, `std::list<U>`, `std::set<U>`），其中 `U` 可以是基本类型、可从字符串构造的自定义类型或元组类类型。要检查基于容器的选项/位置参数是否已提供，请直接绑定到容器并在解析后检查其 `empty()` 状态或 `size()`。
- `std::optional<T>`，其中 `T` 是非容器类型之一（基本类型、可从字符串构造的自定义类型、元组类类型）。这用于可选参数；如果未提供参数，`std::optional` 变量将保持 `std::nullopt`。

## 基础

```cpp
#include <argparse/argparse.hpp>  // 使用 cmake 依赖

#include "argparse.hpp"  // 直接复制到您的项目中
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
  std::optional<std::string> reasoning_effort;
};

int main(int argc, const char* argv[]) {
  argparse::ArgParser parser("openai-cli", "OpenAI API 兼容的命令行聊天机器人");
  ParsedArgs args;

  parser.add_flag("d,debug", "启用调试模式", args.debug);
  parser.add_flag("h,help", "显示帮助信息", args.help);

  parser.add_flag("i,interactive", "启用交互模式", args.interactive);
  parser.add_negative_flag("I", "禁用交互模式", args.interactive);

  parser.add_flag("stream", "启用流模式", args.stream)
      .negatable();  // --no-stream
  parser.add_flag("v,verbose", "启用详细模式", args.verbose);
  parser.add_flag("version", "显示版本信息", args.version);

  parser.add_option("k,key", "OpenAI API 密钥", args.api_key);

  parser.add_option("m,model", "要使用的模型", args.model)
      .default_value("gpt-3.5-turbo");

  parser.add_option("p,prompt", "提示", args.prompt);
  parser.add_option("proxy", "使用代理", args.proxy);
  parser.add_option("s,system-prompt", "系统提示", args.system_prompt);

  parser.add_option("t,temperature", "模型温度", args.temperature)
      .range(0, 2.0);
  parser.add_option("top-p", "模型 top-p 参数", args.top_p).range(0.0, 1.0);

  parser.add_option("reasoning-effort", "推理强度", args.reasoning_effort)
      .choices({"low", "medium", "high", "none"});

  parser.add_option("u,url", "OpenAI API 兼容的 URL", args.api_url)
      .default_value("https://api.openai.com/v1/chat/completions");

  parser.add_positional("prompt", "提示", args.prompt)
      .checker([](const std::string& val) {
        return std::pair{!val.empty(), "提示必须为非空字符串"};
      });

  try {
    parser.parse(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 1;
  }
}
```

## API 详解

本节提供有关定义命令行参数的主要方法的更多详细信息。

### `add_flag`

`add_flag` 方法用于定义一个布尔标志。当命令行参数中存在该标志时，绑定的变量将被设置为 `true`。

- **语法**: `parser.add_flag(names, description, variable_to_bind);`
- **`names`**: 包含逗号分隔的短名称和长名称的字符串（例如 `h,help`、`verbose`）。单个名称也有效（例如 `debug`）。
- **`description`**: 描述标志用途的字符串，用于生成的帮助信息中。
- **`variable_to_bind`**: 对 `bool`、`std::optional<bool>`、整型（如 `int`、`size_t`）或 `std::optional<整型>` 的引用。布尔类变量通常被设置为 `true`（如果是否定形式则为 `false`）。整型通常会递增（如果是否定形式则递减）。

**示例：**

```cpp
struct MyArgs {
  bool verbose = false;
  bool enable_feature = false;
};

MyArgs args;
argparse::ArgParser parser("my_program");

// 定义标志：-v, --verbose
parser.add_flag("v,verbose", "启用详细输出", args.verbose);
// 定义标志：--enable-feature
parser.add_flag("enable-feature", "启用特定功能", args.enable_feature);

// 在 parser.parse(argc, argv) 之后;
// 如果调用 "./my_program -v"，args.verbose 将为 true。
// 如果调用 "./my_program --enable-feature"，args.enable_feature 将为 true。
```

**标志的修饰符：**
- `.negatable()`: （下文介绍）允许像 `--feature` 这样的标志有一个 `--no-feature` 的对应项。
- `.callback(callback_function)`: 定义一个自定义回调函数 (`std::function<void(const FlagValueType&)>`)，当解析到该标志时，会使用标志的有效值（例如，`bool` 或 `int`，即使绑定到 `std::optional`）执行该函数。
    ```cpp
    bool processed = false;
    parser.add_flag("p,process", "处理数据", args.process_flag)
          .callback([&](bool val){
  if (val) processed = true; });
    ```
- `.hidden()`: 在帮助信息中隐藏该标志。
    ```cpp
    parser.add_flag("secret-debug", "秘密调试标志", args.secret_debug).hidden();
    ```

**可否定的标志：**

该库提供了处理可以显式启用或禁用的标志的方法：

1. **`add_negative_flag(names, description, variable_to_bind)`**:
    如果某个功能默认启用（变量初始化为 `true`），并且您希望有一个标志来禁用它（将变量设置为 `false`），这很有用。绑定的变量可以是 `bool`、`std::optional<bool>`、`int` 或 `std::optional<int>`。对于布尔类型，它将值设置为 `false`。对于整型，它通常会递减该值。
    ```cpp
    struct Settings {
  bool debug = true;  // 默认启用
    };
    Settings settings;
    parser.add_flag("debug", "调试", settings.debug);
    parser.add_negative_flag("release", "发布", settings.debug);
    // 如果传递了 --debug，settings.debug 变为 true。
    // 如果传递了 --release，settings.debug 变为 false。
    // 使用 int 的例子：
    int counter = 0;
    parser.add_negative_flag("decrease", "减少计数器", counter); // --decrease 使 counter = -1
    ```

2. **`.negatable()` 修饰符**:
    将其链接到 `add_flag` 调用。如果您的标志名为 `--feature`，这将自动创建一个 `--no-feature` 的对应项。绑定的 `bool` 变量通常应初始化为 `false`。
    ```cpp
    struct Settings {
  bool use_gpu = false;  // 默认禁用
    };
    Settings settings;
    parser.add_flag("gpu", "启用 GPU 加速", settings.use_gpu)
          .negatable(); // 创建 --gpu 和 --no-gpu
    // --gpu 将 use_gpu 设置为 true。
    // --no-gpu 将 use_gpu 设置为 false。
    ```

### `add_option`

`add_option` 方法定义了一个需要参数的选项（例如 `--file <filename>`、`-n <number>`）。

- **语法**: `parser.add_option(names, description, variable_to_bind [, char delimiter]);`
- **`names`**: 逗号分隔的短名称和长名称（例如 `"f,file"`、`"output"`）。
- **`description`**: 描述选项用途的字符串。
- **`variable_to_bind`**: 一个变量的引用，选项的参数值将存储在该变量中。
- **`delimiter`** (可选): 一个字符，用于将单个字符串参数拆分为多个值，用于元组类类型（如 `std::pair<int, int>`）或元组类类型的容器。例如，如果 `delimiter` 是 `,`，则参数 `10,20` 可以填充 `std::pair<int, int>`。

**可绑定类型：**
`add_option` 可以绑定到多种类型：
- C++ 基本类型：`bool`（用于接受显式 true/false 的选项，如 `--enable=true`）、`int`、`long`、`double`、`float`、`std::string`。
- `std::optional<T>`：用于非必需的选项，其中 `T` 是 C++ 基本类型、可从字符串构造的自定义类型或元组类类型（即，对于此特定的存在性检查，不是容器类型）。如果命令行上未提供该选项，绑定的 `std::optional<T>` 变量将保持 `std::nullopt`。这使您可以轻松检查用户是否指定了此特定选项。
- 容器（例如 `std::vector<U>`、`std::list<U>`、`std::set<U>`）：该选项可以在命令行上多次指定（例如 `-I /path1 -I /path2`），所有提供的值都将收集到容器中。`U` 可以是基本类型、可从字符串构造的自定义类型或元组类类型。要确定用户是否指定了绑定到容器的选项，您应该在解析后检查容器是否为空（例如 `my_vector.empty()`）或其 `size()`。不要为了检查存在性而将容器类型本身包装在 `std::optional` 中；直接绑定到容器（例如 `std::vector<std::string> include_paths;`）。
- 自定义类型：用户定义的类型，它们：
    - 可从 `std::string` 构造（例如 `MyType(const std::string& s)`）。该库主要使用此机制处理自定义类型。
- 元组类类型（`std::pair`、`std::tuple`、`std::array`）：如果其元素是可从字符串构造的，则通常支持，允许从单个参数字符串解析（例如，使用指定的分隔符的 "val1,val2"）。

**示例：**

```cpp
struct AppSettings {
  std::string output_file;
  std::optional<int> retries;
  double threshold;
  std::vector<std::string> include_paths;
  // 假设 MyCustomType 有一个字符串构造函数
  // MyCustomType custom_setting;
};

AppSettings settings;
argparse::ArgParser parser("my_app");

// 要使选项实际上是必需的，请绑定到非 optional 类型
// 并且不提供 default_value。解析时会期望它，否则如果您的程序逻辑
// 依赖于它的设置，可能会发生错误。
// 或者，使用 .checker() 来确保提供了有效的值。
parser.add_option("o,output", "指定输出文件", settings.output_file);
// 如果 "o,output" 很关键，请考虑在解析后检查 settings.output_file，
// 或者如果需要特定的验证（如非空），则使用 .checker()。

parser.add_option("r,retries", "操作的重试次数", settings.retries);
// 如果给出 -r 5，settings.retries 将包含 5。
// 如果未给出 -r，settings.retries 将是 std::nullopt。

parser.add_option("t,threshold", "设置阈值", settings.threshold)
      .default_value("0.5"); // 如果未指定该选项，则提供默认值。

parser.add_option("I,include", "包含路径（可多次指定）", settings.include_paths);
// 命令：./my_app -I /usr/local/include -I ./project/include
// settings.include_paths 将包含 {"/usr/local/include", "./project/include"}

// 自定义类型的示例（需要定义 MyCustomType）
struct MyCustomType {
  std::string value;
  MyCustomType() = default;
  MyCustomType(const std::string& s) : value("processed:" + s) {}
};
MyCustomType custom_val;
parser.add_option("custom", "从字符串处理的自定义值", custom_val);
// ./my_app --custom hello -> custom_val.value 将是 "processed:hello"
```

**选项（和位置参数）的通用修饰符：**
这些方法在 `add_option` 或 `add_positional` 之后链接调用：
- `.default_value(value_string)`: 如果未提供选项/参数，则设置默认值。对于接受多个值的选项（例如，绑定到 `std::vector`），您可以使用字符串的初始化列表：`.default_value({"val1", "val2"})`。提供的值必须是字符串，它们将被解析为目标类型。
- `.choices({val1, val2, ...})`: 将参数限制为指定的选项之一。适用于字符串、数字和可比较的自定义类型。值可以是目标类型（例如 `std::vector<int>{1,2,3}`）或字符串（例如 `std::vector<std::string>{"A", "B"}`）。
    ```cpp
    std::string mode;
    parser.add_option("mode", "操作模式", mode)
          .choices({"fast", "slow", "balanced"}); // 用于 std::string
    int level;
    parser.add_option("level", "调试级别", level)
          .choices({0, 1, 2}); // 用于 int
    ```
- `.choices_description({{value1, desc1}, {value2, desc2}, ...})`: 为每个选项提供描述性帮助文本，以增强帮助信息。
    ```cpp
    std::string optimization;
    parser.add_option("opt", "优化级别", optimization)
          .choices_description({{"0", "无优化"}, {"1", "基本优化"}});
    ```
- `.range(min, max)`: 将数字参数限制在特定范围（含）内。
    ```cpp
    int percentage;
    parser.add_option("p,percentage", "百分比值", percentage)
          .range(0, 100);
    ```
- `.callback(callback_function)`: 定义一个自定义回调函数 (`std::function<void(const ValueType&)>`)，当解析选项/参数时，使用解析后的值执行。`ValueType` 是绑定变量的类型（或对于 `std::optional` 或容器，是其 `value_type`，即解包后的类型）。
- `.checker(validation_function)`: 提供自定义验证逻辑。验证函数接受一个值并返回一个 `std::pair<bool, std::string>`。如果 `bool` 为 `false`，解析将失败，并将 `std::string` 用作错误消息。
    - `checker(std::function<std::pair<bool, std::string>(const std::string&)>)`: 在解析前对原始字符串值进行操作。
    - `checker(std::function<std::pair<bool, std::string>(const ValueType&)>)`: 对解析后的值进行操作（`ValueType` 是解包后的类型，例如 `std::optional<int>` 的 `int`）。
- `.env(environment_variable_name)`: 如果命令行上未提供，则允许从指定的环境变量中填充选项或位置参数。
    ```cpp
    std::string apiKey;
    parser.add_option("k,key", "API 密钥", apiKey)
          .env("MYAPP_API_KEY");
    ```
- `.hidden()`: 在帮助信息中隐藏选项或位置参数。
    ```cpp
    parser.add_option("internal-setting", "仅供内部使用", settings.internal)
          .hidden();
    ```
- `.value_help(placeholder_string)`: 自定义帮助信息中参数值的占位符字符串（例如 `<FILE>`、`<NUMBER>`）。
    ```cpp
    parser.add_option("f,file", "输入文件", settings.inputFile)
          .value_help("FILENAME");
    ```
    ```cpp
    std::string filename;
    parser.add_option("f,file", "输入文件", filename)
          .checker([](const std::string& fname) {
            bool is_valid = !fname.empty() && fname.length() > 3;
            return std::pair{is_valid, "文件名不能为空且长度必须大于3个字符。"};
          });
    ```

### `add_positional`

`add_positional` 方法定义了一个位置参数。这些参数前面没有标志或选项名称（例如 `my_program input.txt output.txt`）。它们在命令行上的顺序很重要。

- **语法**: `parser.add_positional(name, description, variable_to_bind [, char delimiter]);`
- **`name`**: 位置参数的符号名称（用于帮助信息和错误报告）。
- **`description`**: 描述参数用途的字符串。
- **`variable_to_bind`**: 一个变量的引用，参数值将存储在该变量中。
- **`delimiter`** (可选): 类似于 `add_option`，一个用于为元组类类型或元组类类型的容器拆分单个字符串参数的字符。

**可绑定类型：**
可绑定类型与 `add_option` 相同（基本类型、`std::optional<T>`、容器、自定义类型、元组类类型）。

**示例：**

```cpp
struct FileCopyArgs {
  std::string source_file;
  std::string destination_file;
  std::optional<int> log_level;
  std::vector<std::string> additional_files;
};

FileCopyArgs fc_args;
argparse::ArgParser parser("copy_tool");

// 对于必需的位置参数，绑定到非 optional 类型且不提供默认值。
// 解析器将按顺序期望它们。如果需要，可使用 .checker() 进行进一步验证。
parser.add_positional("source", "要复制的源文件", fc_args.source_file);
parser.add_positional("destination", "目标路径", fc_args.destination_file);

parser.add_positional("log-level", "可选的日志记录级别（整数）", fc_args.log_level);
// 用法: ./copy_tool src.txt dst.txt 3  -> fc_args.log_level 将包含 3
// 用法: ./copy_tool src.txt dst.txt    -> fc_args.log_level 将是 std::nullopt

// 使用可变参数（贪婪）位置参数处理多个文件的示例：
// argparse::ArgParser variadic_parser("process_items");
// variadic_parser.add_positional("items", "要处理的项目", fc_args.additional_files);
// 命令: ./process_items item1.txt item2.txt item3.txt
// fc_args.additional_files 将包含 {"item1.txt", "item2.txt", "item3.txt"}
```
**关于位置参数的重要说明：**
- 位置参数可以出现在命令行的任何位置，只要它们不被解释为某个选项的值。
- 它们按照使用 `add_positional` 定义的顺序进行解析。
- **绑定到非容器类型：** 当位置参数绑定到非容器类型（例如 `int`、`std::string`）时，它会消耗一个命令行参数。然后解析器会根据其定义期望下一个位置参数。
- **绑定到编译时大小的容器：** 如果绑定到具有编译时已知大小的容器（例如 `std::pair`、`std::tuple`、`std::array<T, N>`），它会消耗 N 个命令行参数来顺序填充容器的元素。
- **绑定到动态大小的容器（可变参数）：** 如果绑定到动态大小的容器（例如 `std::vector<T>`、`std::list<T>`、`std::set<T>`），它会贪婪地消耗所有后续被识别为位置参数的命令行参数。这是一个“可变参数”位置参数。
- **排序多个位置参数：**
    - 绑定到非容器类型或编译时大小容器的位置参数必须在任何可变参数位置参数*之前*定义。
    - 最多只能有一个可变参数位置参数，并且它必须被定义为*最后*一个位置参数。

**修饰符：**
位置参数可以使用前面描述的“选项（和位置参数）的通用修饰符”（例如 `.default_value()`、`.choices()`、`.checker()`、`.env()`、`.hidden()`、`.value_help()`、`.callback()`）。

### 子命令

`argparse.hpp` 支持子命令（也称为子解析器），允许您创建复杂的命令行界面，其中不同的操作有其自己独特的选项和位置参数集（例如 `git commit -m "message"`、`docker image ls`）。

要定义一个子命令，您可以在现有的 `ArgParser` 实例（或另一个 `Command` 实例）上调用 `add_command()`。这将返回一个对新的 `Command` 对象的引用，然后您可以用它自己的标志、选项和位置参数来配置它。

**示例：**

```cpp
#include <argparse/argparse.hpp>
#include <iostream>
#include <optional>  // 需要 std::optional
#include <string>
#include <vector>  // 需要 std::vector

// 主命令和所有子命令的通用参数
struct GlobalArgs {
  bool verbose = false;
};

// 'clone' 子命令特有的参数
struct CloneSubcommandArgs {
  std::string repository_url;
  std::optional<std::string> target_directory;
  std::optional<int> depth;  // 如果 0 是有效深度，使用 optional 更清晰
};

// 'commit' 子命令特有的参数
struct CommitSubcommandArgs {
  std::string message;
  bool all_changes = false;
};

int main(int argc, const char* argv[]) {
  argparse::ArgParser program("git-like-cli", "一个带子命令的CLI");
  GlobalArgs global_args;

  program.add_flag("v,verbose", "启用详细输出", global_args.verbose);

  // --- 定义 'clone' 子命令 ---
  CloneSubcommandArgs clone_args;
  auto& clone_parser = program.add_command("clone", "克隆一个仓库");
  clone_parser.add_positional(
      "repo-url", "仓库 URL",
      clone_args
          .repository_url);  // 如果不是 optional 且没有默认值，则实际上是必需的
  clone_parser.add_positional("dir", "目标目录（可选）",
                              clone_args.target_directory);
  clone_parser.add_option("depth", "克隆深度", clone_args.depth);

  // --- 定义 'commit' 子命令 ---
  CommitSubcommandArgs commit_args;
  auto& commit_parser = program.add_command("commit", "记录更改");
  commit_parser.add_option("m,message", "提交信息",
                           commit_args.message);  // 实际上是必需的
  commit_parser.add_flag("a,all", "暂存所有修改/删除的文件",
                         commit_args.all_changes);

  argparse::Command* active_command = nullptr;
  try {
    // parse 方法返回对激活的命令（主命令或子命令）的引用
    active_command = &program.parse(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << "错误: " << err.what() << std::endl;
    // 如果在解析期间发生错误，打印主程序的用法。
    // 库会自动为相关命令/子命令的 --help/-h 打印帮助信息。
    std::cerr << program.usage() << std::endl;
    return 1;
  }

  if (active_command) {
    if (active_command->name() == "clone") {
      // 如果 'clone' 是子命令，则 clone_args 会被 parse 调用填充
      std::cout << "正在执行 'clone'..." << std::endl;
      if (global_args.verbose) std::cout << "详细模式。" << std::endl;
      std::cout << "仓库: " << clone_args.repository_url << std::endl;
      if (clone_args.target_directory)
        std::cout << "目录: " << *clone_args.target_directory << std::endl;
      if (clone_args.depth)
        std::cout << "深度: " << *clone_args.depth << std::endl;
    } else if (active_command->name() == "commit") {
      // commit_args 被填充
      std::cout << "正在执行 'commit'..." << std::endl;
      if (global_args.verbose) std::cout << "详细模式。" << std::endl;
      std::cout << "信息: " << commit_args.message << std::endl;
      if (commit_args.all_changes) std::cout << "所有更改。" << std::endl;
    } else if (active_command->name() == program.name()) {
      // 没有使用子命令，或者主命令本身被调用。
      // 处理全局标志或显示帮助。
      if (global_args.verbose)
        std::cout << "详细模式（无子命令）。" << std::endl;

      // 如果 program.parse() 在没有参数的情况下被调用（argc == 1），
      // 并且没有定义默认操作/子命令，
      // 内置的 --help 通常会处理显示用法。
      // 这个显式检查可能是为了处理没有参数的 'my_program' 的自定义行为。
      if (argc == 1) {
        std::cout << program.usage() << std::endl;
      }
      // 注意：--help 或 --version 标志（如果像“基础”示例中那样定义）
      // 通常会导致程序打印帮助/版本并退出，由它们的回调处理。
    }
  }

  return 0;
}
```

**使用子命令：**
- 调用 `program.parse(argc, argv)`。此方法返回一个对被激活的 `argparse::Command` 对象的引用（主程序的解析器或其子命令之一的解析器）。
- 检查返回的命令对象的 `name()`（例如 `active_command->name() == "clone"`）以确定调用了哪个子命令。
- 绑定到主解析器的参数（全局参数，如 `global_args.verbose`）无论使用哪个子命令（或者没有使用子命令）都会被解析并可用。
- 特定于子命令的参数只有在该子命令实际在命令行上被调用时才会被解析和填充。
- 帮助信息是上下文感知的：`./my_program clone --help` 显示 `clone` 子命令的特定帮助，而 `./my_program --help` 显示主程序的帮助并列出可用的子命令。
- 每个命令（主解析器或子命令）也可以有一个 `.callback(std::function<void()>)`，如果该命令是被解析的那个，就会被调用。

## 其他功能

### 选项别名

您可以创建一个别名，其行为类似于一个标志，但实际上将特定选项设置为预定义的值。这对于创建快捷方式很有用。

- **语法**: `parser.add_alias(alias_names, original_option_name, value_for_original_option);`
- **`alias_names`**: 新别名的逗号分隔名称（例如 `"f,fast"`）。
- **`original_option_name`**: 现有选项的名称（短名称或长名称，不带破折号）。
- **`value_for_original_option`**: 使用别名时原始选项应设置的字符串值。

**示例：**
```cpp
struct Settings {
  std::string mode;
};
Settings settings;
argparse::ArgParser parser("my_app");
parser.add_option("mode", "设置执行模式", settings.mode)
      .default_value("normal");
parser.add_alias("fast", "mode", "fast_mode"); // 添加 --fast（或 -f 如果是 "f,fast"）
// 如果调用 ./my_app --fast，settings.mode 将是 "fast_mode"。
// --fast 的帮助信息将显示 "等同于 --mode fast_mode"。
```

### 自定义帮助信息输出宽度

您可以调整用于格式化帮助信息的宽度，特别是为选项名称在其描述之前分配的空间。

- **语法**: `parser.set_option_width(width_in_characters);`

**示例：**
```cpp
argparse::ArgParser parser("my_app", "我的应用程序");
parser.set_option_width(40); // 为选项名称列分配40个字符
// ... 添加参数 ...
```

## 构建系统

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

或者

```
git clone https://github.com/shediao/argparse.hpp /path/to/argparse.hpp
add_subdirectory(/path/to/argparse.hpp)
```

```
#include <argparse.hpp>
```

### 2. 其他

将 `argparse.hpp` 复制到 myproject/dir/include

## TODO
- [ ] bash/zsh 自动补全
- [ ] Windows 风格

## 文档
[![在 DeepWiki 中提问](https://deepwiki.com/badge.svg)](https://deepwiki.com/shediao/argparse.hpp)
