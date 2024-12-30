//
// Created by shediao on 24-12-29.
//

#ifndef ARG_PASER_HPP
#define ARG_PASER_HPP

#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace arg::parser {
namespace detail {

// 前置声明
template <typename T>
T from_string(std::string const &s);

template <typename T>
std::string join(const std::vector<std::string> &v, const T &delim) {
  std::stringstream result;
  auto it = v.begin();
  if (it != v.end()) {
    result << *it;
    ++it;
  }
  for (; it != v.end(); ++it) {
    result << delim << *it;
  }
  return result.str();
}

inline std::vector<std::string> split(std::string const &s, char delim,
                                      int max) {
  std::vector<std::string> result;
  auto unsplit_it = s.begin();
  int n = 1;
  while ((n++ < max || max == -1) && unsplit_it != s.end()) {
    auto it = std::find(unsplit_it, s.end(), delim);
    if (it == s.end()) {
      break;
    }
    result.emplace_back(unsplit_it, it);
    unsplit_it = std::next(it);
  }
  result.emplace_back(unsplit_it, s.end());
  return result;
}
template <typename T>
concept is_container = requires(T t) {
  t.insert(t.end(), std::declval<typename T::value_type>());
};

template <typename T>
concept is_tuple = requires(T t) { std::get<0>(std::declval<T &>()); };

template <>
inline std::string from_string<std::string>(std::string const &s) {
  return s;
}
// bool 特化
template <>
inline bool from_string<bool>(std::string const &s) {
  if (s == "true" || s == "on" || s == "1") {
    return true;
  }
  if (s == "false" || s == "off" || s == "0") {
    return false;
  }
  throw std::invalid_argument("Invalid string for bool: " + s);
}

#define ARGPARSER_TYPE_FROM_STRING(type, std_stox)                         \
  template <>                                                              \
  inline type from_string<type>(std::string const &s) {                    \
    try {                                                                  \
      size_t pos = 0;                                                      \
      const type result{std_stox(s, &pos)};                                \
      if (pos != s.size()) {                                               \
        throw std::invalid_argument("Invalid string for " #type ": " + s); \
      }                                                                    \
      return result;                                                       \
    } catch (const std::invalid_argument &e) {                             \
      throw std::invalid_argument("Invalid string for " #type ": " + s);   \
    } catch (const std::out_of_range &e) {                                 \
      throw std::out_of_range("Out of range for " #type ": " + s);         \
    }                                                                      \
  }
ARGPARSER_TYPE_FROM_STRING(int, stoi)
ARGPARSER_TYPE_FROM_STRING(long, stol)
ARGPARSER_TYPE_FROM_STRING(unsigned long, stoul)
ARGPARSER_TYPE_FROM_STRING(long long, stoll)
ARGPARSER_TYPE_FROM_STRING(unsigned long long, stoull)
ARGPARSER_TYPE_FROM_STRING(float, stof)
ARGPARSER_TYPE_FROM_STRING(double, stod)
ARGPARSER_TYPE_FROM_STRING(long double, stold)
#undef ARGPARSER_TYPE_FROM_STRING

template <typename T, std::size_t... I>
T tuple_from_vector(std::vector<std::string> const &v,
                    std::integer_sequence<std::size_t, I...>) {
  return T{from_string<std::decay_t<std::tuple_element_t<I, T>>>(v[I])...};
}

// for tuple & pair
template <typename T>
  requires is_tuple<T> && (!is_container<T>)
T from_string(std::string const &s, const char delim) {
  auto v = detail::split(s, delim, std::tuple_size_v<std::decay_t<T>>);
  if (v.size() != std::tuple_size_v<std::decay_t<T>>) {
    throw std::invalid_argument(
        "Invalid string for split " +
        std::to_string(std::tuple_size_v<std::decay_t<T>>) + "th element:" + s);
  }
  return tuple_from_vector<T>(
      v, std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>());
}

template <typename T>
concept can_from_string_without_delim =
    requires(T t) { from_string<T>(std::declval<std::string>()); };

template <typename T>
  requires is_container<T> &&
           can_from_string_without_delim<typename T::value_type> &&
           (!is_tuple<T>)
T from_string(std::string const &s, const char delim) {
  const auto strings = detail::split(s, delim, -1);
  T result;
  for (auto const &item : strings) {
    result.insert(result.end(), from_string<typename T::value_type>(item));
  }
  return result;
}

template <typename T>
concept can_from_string_with_delim = requires(T t) {
  from_string<T>(std::declval<std::string>(), std::declval<char>());
};

template <typename T>
  requires is_container<T> &&
           can_from_string_with_delim<typename T::value_type> && (!is_tuple<T>)
T from_string(std::string const &s, const char delim, const char delim2) {
  auto strings = detail::split(s, delim, -1);
  T result;
  for (auto const &item : strings) {
    result.insert(result.end(),
                  from_string<typename T::value_type>(item, delim2));
  }
  return result;
}

inline std::vector<std::string> parse_option_name(const std::string &name) {
  std::vector<std::string> result;
  auto names = detail::split(name, ',', -1);
  for (auto const &item : names) {
    result.push_back(item);
  }
  return result;
}

template <typename T>
  requires(!is_container<T>)
void set_value(T &value, T new_value) {
  value = std::move(new_value);
}

template <typename T>
  requires is_container<T>
void set_value(T &value, T new_value) {
  value.insert(value.end(), std::make_move_iterator(new_value.begin()),
               std::make_move_iterator(new_value.end()));
}
}  // namespace detail

class ArgBase {
  friend class ArgParser;

 public:
  ArgBase(const std::string &name, const std::string &description)
      : names(detail::parse_option_name(name)), description(description) {}
  size_t count() const { return set_count; }
  const std::vector<std::string> get_short_names() const {
    std::vector<std::string> result;
    for (const auto &name : names) {
      if (name.size() == 1) {
        result.push_back(name);
      }
    }
    return result;
  }
  const std::vector<std::string> get_long_names() const {
    std::vector<std::string> result;
    for (const auto &name : names) {
      if (name.size() > 1) {
        result.push_back(name);
      }
    }
    return result;
  }

  const std::string &get_description() const { return description; }
  virtual std::string get_usage(int option_width = 24,
                                int padding_char = ' ') const = 0;
  virtual ~ArgBase() = default;
  virtual bool is_flag() const = 0;
  virtual bool is_option() const = 0;
  virtual bool is_positional() const = 0;
  virtual void parse(const std::string &arg_value) = 0;

 protected:
  size_t set_count{0};
  std::vector<std::string> names;
  std::string description;
};

class Flag : public ArgBase {
 public:
  Flag(const std::string &name, const std::string &description)
      : ArgBase(name, description) {}
  bool is_flag() const override { return true; }
  bool is_option() const override { return false; }
  bool is_positional() const override { return false; }
  void parse(const std::string &arg_value) override {
    detail::set_value(value, detail::from_string<bool>(arg_value));
    set_count++;
  }
  std::string get_usage(int option_width, int padding_char) const override {
    std::stringstream usage_str;
    auto short_names = get_short_names();
    std::vector<std::string> short_options;
    std::transform(begin(short_names), end(short_names),
                   std::back_inserter(short_options),
                   [](const auto &arg) { return "-" + arg; });
    auto long_names = get_long_names();
    std::vector<std::string> long_options;
    std::transform(begin(long_names), end(long_names),
                   std::back_inserter(long_options),
                   [](const auto &arg) { return "--" + arg; });
    std::string short_option_str = detail::join(short_options, ",");
    std::string long_option_str = detail::join(long_options, ",");
    int padding_size =
        option_width - short_option_str.size() - long_option_str.size();
    std::string padding =
        std::string(padding_size > 0 ? padding_size : 2, padding_char);
    usage_str << std::format("  {}, {}{}{}", short_option_str, long_option_str,
                             padding, get_description());
    return usage_str.str();
  }
  bool value{false};
};

class OptionBase : public ArgBase {
 public:
  OptionBase(const std::string &name, const std::string &description)
      : ArgBase(name, description) {}
  bool is_flag() const override { return false; }
  template <typename T>
  void set_default_value_help() {
    if constexpr (std::is_integral_v<T>) {
      value_help = "<N>";
    } else if constexpr (std::is_floating_point_v<T>) {
      value_help = "<0.f>";
    } else if constexpr (std::is_same_v<T, std::string>) {
      value_help = "<TEXT>";
    } else {
      value_help = "<ARG>";
    }
  }
  void set_value_help(const std::string &value_help) {
    this->value_help = value_help;
  }
  const std::string &get_value_help() const { return value_help; }
  std::string get_usage(int option_width, int padding_char) const override {
    std::stringstream usage_str;
    if (is_option()) {
      auto short_names = get_short_names();
      std::vector<std::string> short_options;
      std::transform(begin(short_names), end(short_names),
                     std::back_inserter(short_options),
                     [](const auto &arg) { return "-" + arg; });
      auto long_names = get_long_names();
      std::vector<std::string> long_options;
      std::transform(begin(long_names), end(long_names),
                     std::back_inserter(long_options),
                     [](const auto &arg) { return "--" + arg; });
      std::string short_option_str = detail::join(short_options, ",");
      std::string long_option_str = detail::join(long_options, ",");

      std::string value_help = get_value_help();
      int padding_size = option_width - short_option_str.size() -
                         long_option_str.size() - value_help.size() - 1;
      std::string padding =
          std::string(padding_size > 0 ? padding_size : 2, padding_char);
      usage_str << std::format("  {}, {} {}{}{}", short_option_str,
                               long_option_str, value_help, padding,
                               get_description());
    } else {
      auto positional_str = names[0];
      int padding_size =
          option_width - positional_str.size() - value_help.size() - 1;
      std::string padding =
          std::string(padding_size > 0 ? padding_size : 2, padding_char);
      usage_str << std::format("  {} {}{}{}", positional_str, value_help,
                               padding, get_description());
    }
    return usage_str.str();
  }

  std::string value_help;
  std::string arg_value;
};

template <typename T>
class Option : public OptionBase {
 public:
  Option(const std::string &name, const std::string &description, T &bind_value)
      : OptionBase(name, description), bind_value(bind_value) {
    set_default_value_help<T>();
  }
  bool is_option() const override { return true; }
  bool is_positional() const override { return false; }
  void parse(const std::string &arg_value) override {
    this->arg_value = arg_value;
    detail::set_value(bind_value, detail::from_string<T>(arg_value));
    set_count++;
  }
  T &bind_value;
};

template <typename T>
class Positional : public OptionBase {
 public:
  Positional(const std::string &name, const std::string &description,
             T &bind_value)
      : OptionBase(name, description), bind_value(bind_value) {
    set_default_value_help<T>();
    set_value_help(names[0]);
  }
  bool is_option() const override { return false; }
  bool is_positional() const override { return true; }
  void parse(const std::string &arg_value) override {
    this->arg_value = arg_value;
    detail::set_value(bind_value, detail::from_string<T>(arg_value));
    set_count++;
  }
  T &bind_value;
};

class ArgParser {
 public:
  ArgParser(int argc, const char *argv[])
      : commands(argc > 0 ? argv : nullptr, argc > 0 ? argv + argc : nullptr) {}
  void add_flag(const std::string &name, const std::string &description) {
    args.push_back(std::make_unique<Flag>(name, description));
  }

  template <typename T>
    requires detail::can_from_string_without_delim<T>
  void add_option(const std::string &name, const std::string &description,
                  T &bind_value) {
    args.push_back(std::make_unique<Option<T>>(name, description, bind_value));
  }

  template <typename T>
    requires detail::can_from_string_with_delim<T>
  void add_option(const std::string &name, const std::string &description,
                  T &bind_value, char delim) {
    args.push_back(
        std::make_unique<Option<T>>(name, description, bind_value, delim));
  }

  template <typename T>
    requires detail::can_from_string_with_delim<T> &&
             detail::can_from_string_with_delim<typename T::value_type>
  void add_option(const std::string &name, const std::string &description,
                  T &bind_value, char delim, char delim2) {
    args.push_back(std::make_unique<Option<T>>(name, description, bind_value,
                                               delim, delim2));
  }

  template <typename T>
    requires detail::can_from_string_without_delim<T>
  void add_positional(const std::string &name, const std::string &description,
                      T &bind_value) {
    args.push_back(
        std::make_unique<Positional<T>>(name, description, bind_value));
  }

  template <typename T>
    requires detail::can_from_string_with_delim<T>
  void add_positional(const std::string &name, const std::string &description,
                      T &bind_value, char delim) {
    args.push_back(
        std::make_unique<Positional<T>>(name, description, bind_value, delim));
  }

  template <typename T>
    requires detail::can_from_string_with_delim<T> &&
             detail::can_from_string_with_delim<typename T::value_type>
  void add_positional(const std::string &name, const std::string &description,
                      T &bind_value, char delim, char delim2) {
    args.push_back(std::make_unique<Positional<T>>(name, description,
                                                   bind_value, delim, delim2));
  }

  void print_usage() const {
    std::stringstream usage_str;
    usage_str << "Usage: \n  " << commands[0];
    if (std::ranges::find_if(args, [](const auto &arg) {
          return arg->is_option() || arg->is_flag();
        }) != args.end()) {
      usage_str << " [OPTIONS]...";
    }
    if (std::ranges::find_if(args, [](const auto &arg) {
          return arg->is_positional();
        }) != args.end()) {
      usage_str << " <POSITIONAL>...";
    }
    if (std::ranges::find_if(args, [](const auto &arg) {
          return arg->is_option() || arg->is_flag();
        }) != args.end()) {
      usage_str << "\n\nOptions:\n";
    }
    for (const auto &arg : args) {
      if (arg->is_option() || arg->is_flag()) {
        usage_str << arg->get_usage(30, '.') << '\n';
      }
    }

    if (std::ranges::find_if(args, [](const auto &arg) {
          return arg->is_positional();
        }) != args.end()) {
      usage_str << "\n\nPositionals:\n";
    }
    for (const auto &arg : args) {
      if (arg->is_positional()) {
        usage_str << arg->get_usage(30, '.') << '\n';
      }
    }
    std::cout << usage_str.str() << std::endl;
  }
  void print_version() const {
    std::cout << "Version: " << version << std::endl;
  }
  void parse();

 private:
  std::vector<std::unique_ptr<ArgBase>> args;
  std::string version;
  std::string usage;
  std::vector<std::string> commands;
};

}  // namespace arg::parser

#endif  // ARG_PASER_HPP
