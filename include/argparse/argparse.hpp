//
// Created by shediao.xsd on 24-12-29.
//

#ifndef __ARGPARSE_ARGPARSE_HPP__
#define __ARGPARSE_ARGPARSE_HPP__

#include <algorithm>
#include <cctype>
#include <concepts>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#endif  // _WIN32

#if defined(ARG_PARSE_DEBUG)
#define ARG_PARSER_DEBUG(msg)                   \
  do {                                          \
    std::cerr << "[ArgParser] " << msg << '\n'; \
  } while (0)
#else
#define ARG_PARSER_DEBUG(msg) \
  do {                        \
  } while (0)
#endif

namespace argparse {

static constexpr size_t OPTION_NAME_WIDTH = 32;

namespace detail {

template <typename...>
using void_t = void;

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_optional_v = is_optional<T>::value;

template <typename T>
constexpr bool is_integral_v =
    std::is_integral_v<T> && !std::is_same_v<bool, T>;

template <typename T>
struct is_string : std::false_type {};

template <typename charT>
struct is_string<std::basic_string<charT>> : std::true_type {};

template <typename T>
constexpr bool is_string_v = is_string<T>::value;

template <typename T>
concept is_container = requires(T t) {
  requires(!is_string_v<T>);
  t.insert(t.end(), std::declval<typename T::value_type>());
};

template <typename T, typename = void>
struct has_std_to_string : std::false_type {};

template <typename T>
struct has_std_to_string<T, void_t<decltype(std::to_string(std::declval<T>()))>>
    : std::is_same<decltype(std::to_string(std::declval<T>())), std::string> {};

template <typename T>
constexpr bool has_std_to_string_v = has_std_to_string<T>::value;

template <typename T>
concept ParseFromStringBasicType =
    is_string_v<std::remove_cv_t<T>> ||
    std::same_as<std::remove_cv_t<T>, bool> ||
    std::same_as<std::remove_cv_t<T>, char> ||
    std::same_as<std::remove_cv_t<T>, unsigned int> ||
    std::same_as<std::remove_cv_t<T>, int> ||
    std::same_as<std::remove_cv_t<T>, long> ||
    std::same_as<std::remove_cv_t<T>, unsigned long> ||
    std::same_as<std::remove_cv_t<T>, long long> ||
    std::same_as<std::remove_cv_t<T>, unsigned long long> ||
    std::same_as<std::remove_cv_t<T>, float> ||
    std::same_as<std::remove_cv_t<T>, double> ||
    std::same_as<std::remove_cv_t<T>, long double>;

template <typename T>
concept ParseFromStringCustomType =
    (std::is_constructible_v<std::remove_cv_t<T>, std::string> ||
     std::convertible_to<std::string, std::remove_cv_t<T>>) &&
    (!ParseFromStringBasicType<std::remove_cv_t<T>>);

template <typename T>
concept ParseFromStringSingleType =
    ParseFromStringBasicType<T> || ParseFromStringCustomType<T>;

template <typename T>
concept ParseFromStringTupleLikeType = requires {
  std::get<0>(std::declval<T &>());
  requires std::tuple_size_v<T> > 0;
  typename std::tuple_element<0, T>::type;
} && []<std::size_t... I>(std::integer_sequence<std::size_t, I...>) constexpr {
  return (ParseFromStringSingleType<std::tuple_element_t<I, T>> && ...);
}(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T>
concept ParseFromStringOptionalSingleType =
    detail::is_optional_v<T> &&
    ParseFromStringSingleType<typename T::value_type>;

template <typename T>
concept ParseFromStringOptionalTupleLikeType =
    detail::is_optional_v<T> &&
    ParseFromStringTupleLikeType<typename T::value_type>;

template <typename T>
concept ParseFromStringType =
    ParseFromStringSingleType<T> || ParseFromStringTupleLikeType<T> ||
    ParseFromStringOptionalSingleType<T> ||
    ParseFromStringOptionalTupleLikeType<T>;

template <typename T>
concept ParseFromStringContainerType =
    is_container<T> && (ParseFromStringSingleType<typename T::value_type> ||
                        ParseFromStringTupleLikeType<typename T::value_type>);

template <typename T>
struct extract_value_type {
  using type = T;
};

template <typename T>
struct extract_value_type<std::optional<T>> {
  using type = T;
};

template <ParseFromStringContainerType T>
struct extract_value_type<T> {
  using type = typename T::value_type;
};

template <typename T>
using extract_value_type_t = typename extract_value_type<T>::type;

template <typename T>
concept BindableType =
    ParseFromStringContainerType<T> || ParseFromStringSingleType<T> ||
    ParseFromStringTupleLikeType<T> || ParseFromStringOptionalSingleType<T> ||
    ParseFromStringOptionalTupleLikeType<T>;

template <typename T>
concept BindableWithoutDelimiterType =
    ParseFromStringSingleType<T> || ParseFromStringOptionalSingleType<T> ||
    (ParseFromStringContainerType<T> &&
     ParseFromStringSingleType<typename T::value_type>);

template <typename T>
concept BindableWithDelimiterType =
    ParseFromStringTupleLikeType<T> ||
    ParseFromStringOptionalTupleLikeType<T> ||
    (ParseFromStringContainerType<T> &&
     ParseFromStringTupleLikeType<typename T::value_type>);

template <typename T>
concept is_parse_from_string_basic_type =
    std::same_as<T, std::string> || std::same_as<T, bool> ||
    std::same_as<T, int> || std::same_as<T, long> ||
    std::same_as<T, unsigned long> || std::same_as<T, long long> ||
    std::same_as<T, unsigned long long> || std::same_as<T, float> ||
    std::same_as<T, double> || std::same_as<T, long double> ||
    requires(std::string const &s) { T(s); };

template <ParseFromStringBasicType T>
T parse_from_string(std::string const &s) {
  size_t pos = 0;
  if constexpr (std::is_same_v<T, int>) {
    auto result = std::stoi(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for int: " + s);
    }
    return result;
  } else if constexpr (std::is_same_v<T, long>) {
    auto result = std::stol(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for long: " + s);
    }
    return result;
  } else if constexpr (std::is_same_v<T, unsigned long>) {
    auto result = std::stoul(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for unsigned long: " + s);
    }
    return result;
  } else if constexpr (std::is_same_v<T, long long>) {
    auto result = std::stoll(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for long long: " + s);
    }
    return result;
  } else if constexpr (std::is_same_v<T, unsigned long long>) {
    auto result = std::stoull(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for unsigned long long: " +
                                  s);
    }
    return result;
  } else if constexpr (std::is_same_v<T, float>) {
    auto result = std::stof(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for float: " + s);
    }
    return result;
  } else if constexpr (std::is_same_v<T, double>) {
    auto result = std::stod(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for double: " + s);
    }
    return result;
  } else if constexpr (std::is_same_v<T, long double>) {
    auto result = std::stold(s, &pos);
    if (pos != s.size()) {
      throw std::invalid_argument("Invalid string for long double: " + s);
    }
    return result;
  }
  if constexpr (std::is_same_v<T, bool>) {
    return parse_from_string<bool>(s);
  }
  if constexpr (std::is_same_v<T, std::string>) {
    return s;
  }
  throw std::invalid_argument("Invalid type for parse_from_string");
}

template <ParseFromStringCustomType T>
T parse_from_string(std::string const &s) {
  return T(s);
}

template <>
inline bool parse_from_string<bool>(std::string const &s) {
  if (s == "true" || s == "on" || s == "1") {
    return true;
  }
  if (s == "false" || s == "off" || s == "0") {
    return false;
  }
  throw std::invalid_argument("Invalid string for bool: " + s);
}
template <>
inline char parse_from_string<char>(std::string const &s) {
  if (s.length() == 1) {
    return s[0];
  }
  throw std::invalid_argument("Invalid string for char: " + s);
}

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

template <typename CharT, typename F, typename C>
  requires std::is_same_v<bool,
                          decltype(std::declval<F>()(std::declval<CharT>()))>
C &split_to_if(C &to, const std::basic_string<CharT> &str, F f,
               int max_count = -1, bool is_compress_token = false) {
  auto begin = str.begin();
  auto delimiter = begin;
  size_t MAX_COUNT = max_count < 0 ? (std::numeric_limits<size_t>::max())
                                   : static_cast<size_t>(max_count);
  size_t count = 0;

  while (count++ < MAX_COUNT &&
         (delimiter = std::find_if(begin, str.end(), f)) != str.end()) {
    to.insert(to.end(), {begin, delimiter});
    if (is_compress_token) {
      begin = std::find_if_not(delimiter, str.end(), f);
    } else {
      begin = std::next(delimiter);
    }
  }

  to.insert(to.end(), {begin, str.end()});

  return to;
}

inline std::vector<std::string> split(std::string const &s, char delim,
                                      int max) {
  std::vector<std::string> result;
  split_to_if(result, s, [delim](char c) { return c == delim; }, max, false);
  return result;
}

template <typename T, std::size_t... I>
T make_tuple_from_container_impl(std::vector<std::string> const &v,
                                 std::integer_sequence<std::size_t, I...>) {
  return T{
      parse_from_string<std::decay_t<std::tuple_element_t<I, T>>>(v[I])...};
}

template <ParseFromStringTupleLikeType T>
T make_tuple_from_container(std::vector<std::string> const &v) {
  return make_tuple_from_container_impl<T>(
      v, std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>());
}

// for tuple like types
template <ParseFromStringTupleLikeType T>
T parse_from_string(std::string const &s, const char delim) {
  auto v = split(s, delim, std::tuple_size_v<std::decay_t<T>> - 1);
  if (v.size() != std::tuple_size_v<std::decay_t<T>>) {
    throw std::invalid_argument(
        "Invalid string for split " +
        std::to_string(std::tuple_size_v<std::decay_t<T>>) + "th element:" + s);
  }
  return make_tuple_from_container<T>(v);
}

inline std::string format(std::string const &option_name, size_t width,
                          std::string const &description) {
  std::string ret = option_name;

  auto last_option_name_width = option_name.length();
  if (auto p = option_name.rfind('\n'); p != std::string::npos) {
    last_option_name_width = option_name.length() - p - 1;
  }

  if (last_option_name_width >= width) {
    ret += " ";
  } else {
    ret += std::string(width - last_option_name_width, ' ');
  }

  size_t pos = 0;
  auto desc = description;
  auto replacement = "\n" + std::string(width, ' ');
  while ((pos = desc.find("\n", pos)) != std::string::npos) {
    desc.replace(pos, 1, replacement);
    pos += replacement.length();
  }

  ret += desc;

  return ret;
}

inline std::optional<std::string> getenv(std::string const &name) {
#if defined(_WIN32)
  auto const size = GetEnvironmentVariableA(name.c_str(), nullptr, 0);
  if (size == 0) {
    return GetLastError() == ERROR_ENVVAR_NOT_FOUND
               ? std::nullopt
               : std::optional<std::string>{""};
  }
  std::vector<char> value(size);
  GetEnvironmentVariableA(name.c_str(), value.data(), size);
  return std::string{value.data()};
#else
  auto *env = ::getenv(name.c_str());
  if (env) {
    return std::string(env);
  }
  return std::nullopt;
#endif
}

}  // namespace detail

inline void store_true(bool &value) { value = true; }

inline void store_false(bool &value) { value = false; }

template <typename T>
  requires std::integral<T>
inline void increment(T &value) {
  ++value;
}

template <typename T>
  requires std::integral<T>
inline void decrement(T &value) {
  --value;
}

class ArgBase {
  friend class Command;
  friend class ArgParser;

 public:
  ArgBase(const std::string &name, const std::string &description)
      : description_(description) {
    for (auto &&opt_name : detail::split(name, ',', -1)) {
      if (opt_name.empty()) {
        continue;
      }
      if (opt_name[0] == '-') {
        throw std::invalid_argument("Invalid option name: " + name + ", " +
                                    opt_name + " starts with '-'");
      }
      if (std::find_if(opt_name.begin(), opt_name.end(), [](unsigned char c) {
            return std::isblank(c);
          }) != opt_name.end()) {
        throw std::invalid_argument("Invalid option name: " + name + ", " +
                                    opt_name + " contains blank");
      }
      if (opt_name.length() == 1) {
        short_opt_names_.push_back(std::move(opt_name));
      } else if (opt_name.length() > 1) {
        long_opt_names_.push_back(std::move(opt_name));
      } else {
        throw std::invalid_argument("Invalid option name: " + name);
      }
    }
  }
  size_t count() const { return count_; }
  virtual ~ArgBase() = default;

  ArgBase &hidden(bool v = true) {
    hidden_ = v;
    return *this;
  }

  ArgBase &env(std::string const &env) {
    env_key_ = env;
    return *this;
  }

 protected:
  virtual bool is_flag() const = 0;
  virtual bool is_option() const = 0;
  virtual bool is_positional() const = 0;
  virtual std::string usage() const = 0;
  size_t option_width() const { return option_width_; }
  void set_option_width(size_t width) { option_width_ = width; }
  const std::string &description() const { return description_; }
  size_t count_{0};
  std::vector<std::string> short_opt_names_;
  std::vector<std::string> long_opt_names_;
  std::string description_;
  std::string env_key_;
  bool hidden_{false};
  size_t option_width_{OPTION_NAME_WIDTH};
};

class FlagBase : public ArgBase {
  friend class Command;
  friend class ArgParser;

 public:
  FlagBase(const std::string &name, const std::string &description)
      : ArgBase(name, description) {}

  void negatable(bool v = true) { this->negatable_ = v; }
  bool is_negatable() { return this->negatable_; }

 protected:
  bool is_flag() const override final { return true; }
  bool is_option() const override final { return false; }
  bool is_positional() const override final { return false; }
  virtual void parse() = 0;
  virtual void negatable_parse() = 0;
  bool negatable_ = false;
  std::string usage() const override {
    std::string delimiter = ",";
    std::string delimiter_of_short_and_long = ", ";
    std::stringstream usage_str;

    auto flag_names_length = std::accumulate(
        short_opt_names_.begin(), short_opt_names_.end(), (size_t)0,
        [negatable = negatable_](size_t t, const std::string &s) {
          return t + s.length() + 1 + (negatable ? 5 : 0);
          ;
        });
    flag_names_length = std::accumulate(
        long_opt_names_.begin(), long_opt_names_.end(), flag_names_length,
        [negatable = negatable_](size_t t, const std::string &s) {
          return t + s.length() + 2 + (negatable ? 5 : 0);
        });

    flag_names_length +=
        (short_opt_names_.size() + long_opt_names_.size() - 2) *
        delimiter.length();

    if (!short_opt_names_.empty() && !long_opt_names_.empty()) {
      flag_names_length += delimiter_of_short_and_long.length();
    }

    std::vector<std::string> short_names_with_dash;
    std::vector<std::string> long_names_with_dash_dash;
    std::ranges::transform(
        short_opt_names_, back_inserter(short_names_with_dash),
        [no_long_names = long_opt_names_.empty(),
         negatable = negatable_](auto const &s) {
          return no_long_names && negatable ? ("[--no]-" + s) : ("-" + s);
        });
    std::ranges::transform(long_opt_names_,
                           back_inserter(long_names_with_dash_dash),
                           [negatable = negatable_](auto const &s) {
                             return negatable ? ("--[no-]" + s) : ("--" + s);
                           });

    bool use_multiple_lines = flag_names_length > option_width();

    if (use_multiple_lines) {
      std::vector<std::string> all_names{short_names_with_dash};
      all_names.insert(all_names.end(), long_names_with_dash_dash.begin(),
                       long_names_with_dash_dash.end());
      auto end = all_names.end() - 1;
      for (auto it = all_names.begin(); it != end; ++it) {
        usage_str << *it << '\n';
      }
      usage_str << detail::format(*end, option_width(), description());
    } else {
      auto s = detail::join(short_names_with_dash, delimiter);
      auto l = detail::join(long_names_with_dash_dash, delimiter);
      if (short_names_with_dash.empty()) {
        std::string options_str =
            "  " + std::string(delimiter_of_short_and_long.length(), ' ') + l;
        usage_str << detail::format(options_str, option_width(), description());
      } else {
        std::string options_str = long_names_with_dash_dash.empty()
                                      ? s
                                      : (s + delimiter_of_short_and_long + l);
        usage_str << detail::format(options_str, option_width(), description());
      }
    }

    return usage_str.str();
  }
};

template <typename T = bool>
  requires detail::is_integral_v<T> || std::is_same_v<T, bool> ||
           (detail::is_optional_v<T> &&
            (detail::is_integral_v<typename T::value_type> ||
             std::is_same_v<typename T::value_type, bool>))
class Flag final : public FlagBase {
  friend class Command;
  friend class ArgParser;

 public:
  using value_type = std::conditional_t<detail::is_optional_v<T>,
                                        detail::extract_value_type_t<T>, T>;
  using parsed_value_type = detail::extract_value_type_t<T>;

 public:
  Flag(const std::string &name, const std::string &description, T &bind_value)
      : FlagBase(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_{store_true},
        parse_negatable_function_{store_false} {}
  Flag(const std::string &name, const std::string &description, T &bind_value,
       std::function<void(parsed_value_type &)> action,
       std::function<void(parsed_value_type &)> negatable_action)
      : FlagBase(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_(std::move(action)),
        parse_negatable_function_(std::move(negatable_action)) {}

  Flag<T> &callback(std::function<void(parsed_value_type)> cb) {
    callback_ = std::move(cb);
    return *this;
  }

  T const &value() const { return bind_value_; }

 protected:
  void parse() override {
    if constexpr (detail::is_optional_v<T>) {
      auto &flag_value = bind_value_.get();
      if (!flag_value.has_value()) {
        flag_value = typename T::value_type{};
      }
      parse_function_(flag_value.value());
      if (callback_) {
        callback_(flag_value.value());
      }
    } else {
      parse_function_(bind_value_.get());
      if (callback_) {
        callback_(bind_value_.get());
      }
    }
    count_++;
  }
  void negatable_parse() override {
    if constexpr (detail::is_optional_v<T>) {
      auto &flag_value = bind_value_.get();
      if (!flag_value.has_value()) {
        flag_value = typename T::value_type{};
      }
      parse_negatable_function_(flag_value.value());
      if (callback_) {
        callback_(flag_value.value());
      }
    } else {
      parse_negatable_function_(bind_value_.get());
      if (callback_) {
        callback_(bind_value_.get());
      }
    }
    count_++;
  }

 private:
  std::reference_wrapper<T> bind_value_;
  std::function<void(parsed_value_type &)> parse_function_;
  std::function<void(parsed_value_type &)> parse_negatable_function_;
  std::function<void(parsed_value_type)> callback_;
};

template <typename T>
class OptionValueChecker {
 public:
  OptionValueChecker(std::function<bool(const T &)> check,
                     std::string error_message)
      : check_(std::move(check)), error_message_(std::move(error_message)) {}
  bool operator()(const T &value) const { return check_(value); }
  const std::string &error_message() const { return error_message_; }

 private:
  std::function<bool(const T &)> check_;
  std::string error_message_;
};

class OptionBase : public ArgBase {
  friend class Command;
  friend class ArgParser;
  friend class OptionAlias;

 public:
  OptionBase(const std::string &name, const std::string &description)
      : ArgBase(name, description) {}

 protected:
  bool is_flag() const override final { return false; }
  virtual void parse(const std::string &opt_value) {
    for (const auto &checker : parse_befor_value_checker_) {
      if (!checker(opt_value)) {
        std::string err_msg = "check failed: ";
        err_msg += detail::join(long_opt_names_, ',');
        err_msg += detail::join(short_opt_names_, ',');
        err_msg += "==> ";
        err_msg += opt_value;
        err_msg += " is a invalid value. ";
        throw std::invalid_argument(err_msg + checker.error_message());
      }
    }
    this->opt_values.push_back(opt_value);
    count_++;
  }

  virtual bool is_multiple() const = 0;
  virtual void use_default_if_needed() = 0;
  virtual std::optional<std::string> get_default_value() const = 0;

  template <typename T>
  void set_default_value_help() {
    if constexpr (detail::is_optional_v<T>) {
      set_default_value_help<typename T::value_type>();
    } else {
      if constexpr (std::is_integral_v<T>) {
        value_help_ = "<N>";
      } else if constexpr (std::is_floating_point_v<T>) {
        value_help_ = "<0.0>";
      } else {
        value_help_ = "<arg>";
      }
    }
  }

  void use_env_if_needed() {
    if (count() != 0 || env_key_.empty()) {
      return;
    }
    if (auto env = detail::getenv(env_key_); env.has_value()) {
      parse(env.value());
    }
  }
  std::string usage() const override {
    constexpr int max_width = 80;
    std::stringstream usage_str;
    if (is_option()) {
      std::string delimiter = ",";
      std::string delimiter_of_short_and_long = ", ";

      auto opt_names_length = std::accumulate(
          short_opt_names_.begin(), short_opt_names_.end(), (size_t)0,
          [](size_t t, const std::string &s) { return t + s.length() + 1; });
      opt_names_length = std::accumulate(
          long_opt_names_.begin(), long_opt_names_.end(), opt_names_length,
          [](size_t t, const std::string &s) { return t + s.length() + 2; });

      opt_names_length +=
          (short_opt_names_.size() + long_opt_names_.size() - 2) *
          delimiter.length();
      if (!short_opt_names_.empty() && !long_opt_names_.empty()) {
        opt_names_length += delimiter_of_short_and_long.length();
      }

      opt_names_length += (value_help_.length() + 1);

      std::vector<std::string> short_names_with_dash;
      std::vector<std::string> long_names_with_dash_dash;
      std::ranges::transform(short_opt_names_,
                             back_inserter(short_names_with_dash),
                             [](auto const &s) { return "-" + s; });
      std::ranges::transform(long_opt_names_,
                             back_inserter(long_names_with_dash_dash),
                             [](auto const &s) { return "--" + s; });

      bool use_multiple_lines = opt_names_length > option_width();

      if (use_multiple_lines) {
        std::vector<std::string> all_names{short_names_with_dash};
        all_names.insert(all_names.end(), long_names_with_dash_dash.begin(),
                         long_names_with_dash_dash.end());
        auto end = all_names.end() - 1;
        for (auto it = all_names.begin(); it != end; ++it) {
          usage_str << *it << ' ' << value_help_ << '\n';
        }
        usage_str << detail::format(*end + " " + value_help_, option_width(),
                                    description());
      } else {
        auto s = detail::join(short_names_with_dash, delimiter);
        auto l = detail::join(long_names_with_dash_dash, delimiter);
        if (short_names_with_dash.empty()) {
          std::string options_str =
              "  " + std::string(delimiter_of_short_and_long.length(), ' ') + l;
          usage_str << detail::format(options_str + " " + value_help_,
                                      option_width(), description());
        } else {
          std::string options_str = long_names_with_dash_dash.empty()
                                        ? s
                                        : (s + delimiter_of_short_and_long + l);
          usage_str << detail::format(options_str + " " + value_help_,
                                      option_width(), description());
        }
      }

      if (auto default_value = get_default_value(); default_value.has_value()) {
        auto default_value_string = " (default:" + *default_value;
        if (!env_key_.empty()) {
          default_value_string += ", ENV:" + env_key_;
        }
        default_value_string += ")";
        if (option_width() + description().length() +
                default_value_string.length() >
            max_width) {
          usage_str << "\n"
                    << detail::format("", option_width(), default_value_string);
        } else {
          usage_str << default_value_string;
        }
      }
      if (!this->choices_descriptions_.empty()) {
        for (auto const &[value, help] : this->choices_descriptions_) {
          usage_str << '\n'
                    << detail::format("      [" + value + "]", option_width(),
                                      " " + help);
        }
        usage_str << '\n';
      }
    } else {
      std::string options_str{long_opt_names_[0]};
      usage_str << detail::format(options_str, option_width(), description());
      if (auto default_value = get_default_value(); default_value.has_value()) {
        auto default_value_string = " (default: " + *default_value + ")";
        if (option_width() + description().length() +
                default_value_string.length() >
            max_width) {
          usage_str << "\n"
                    << detail::format("", option_width(), default_value_string)
                    << '\n';
        } else {
          usage_str << default_value_string;
        }
      }
    }
    return usage_str.str();
  }
  std::string value_help_;
  std::vector<std::string> opt_values;
  std::vector<OptionValueChecker<std::string>> parse_befor_value_checker_;
  std::map<std::string, std::string> choices_descriptions_;
};

template <typename Derived>
class OptionBaseCRTP : public OptionBase {
 public:
  using OptionBase::OptionBase;
  Derived &checker(std::function<bool(std::string const &)> f,
                   std::string description = "") {
    parse_befor_value_checker_.push_back(
        OptionValueChecker<std::string>(std::move(f), std::move(description)));

    return static_cast<Derived &>(*this);
  }

  Derived &choices(std::vector<std::string> const &allowed_values) {
    OptionBaseCRTP<Derived>::checker(
        [allowed_values](const std::string &value) {
          return std::ranges::find(allowed_values, value) !=
                 std::ranges::end(allowed_values);
        },
        "not in choices: [" + detail::join(allowed_values, ',') + "]");
    return static_cast<Derived &>(*this);
  }

  Derived &value_help(std::string const &value_help) {
    this->value_help_ = "<" + value_help + ">";
    return static_cast<Derived &>(*this);
  }

  Derived &choices_description(
      std::map<std::string, std::string> const &choices_description) {
    this->choices_descriptions_ = choices_description;
    return static_cast<Derived &>(*this);
  }
  Derived &hidden(bool v = true) {
    ArgBase::hidden(v);
    return static_cast<Derived &>(*this);
  }
  Derived &env(std::string const &env) {
    ArgBase::env(env);
    return static_cast<Derived &>(*this);
  }
};

template <detail::BindableType T>
class Option final : public OptionBaseCRTP<Option<T>> {
  friend class Command;
  friend class ArgParser;

 public:
  using value_type = std::conditional_t<detail::is_optional_v<T>,
                                        detail::extract_value_type_t<T>, T>;
  using parsed_value_type = detail::extract_value_type_t<T>;

 public:
  Option(const std::string &name, const std::string &description, T &bind_value)
    requires detail::BindableWithoutDelimiterType<T>
      : OptionBaseCRTP<Option<T>>(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_(detail::parse_from_string<parsed_value_type>) {
    OptionBase::set_default_value_help<T>();
  }
  Option(const std::string &name, const std::string &description, T &bind_value,
         char delim)
    requires detail::BindableWithDelimiterType<T>
      : OptionBaseCRTP<Option<T>>(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_([delim](std::string const &opt_value) {
          return detail::parse_from_string<parsed_value_type>(opt_value, delim);
        }) {
    OptionBase::set_default_value_help<T>();
  }
  Option<T> &default_value(const std::string &default_value)
    requires(!detail::ParseFromStringContainerType<T>)
  {
    this->default_value_ = default_value;
    return *this;
  }
  Option<T> &default_value(std::initializer_list<std::string> default_value)
    requires detail::ParseFromStringContainerType<T>
  {
    this->default_value_ = default_value;
    return *this;
  }

  Option<T> &callback(std::function<void(value_type const &)> cb) {
    callback_ = std::move(cb);
    return *this;
  }

  using OptionBaseCRTP<Option<T>>::checker;
  Option<T> &checker(
      std::function<bool(const parsed_value_type &)> check_function,
      std::string description = "") {
    value_checker_.push_back(OptionValueChecker<parsed_value_type>(
        std::move(check_function), std::move(description)));
    return *this;
  }

  Option<T> &range(parsed_value_type r_min, parsed_value_type r_max)
    requires std::is_arithmetic_v<parsed_value_type>
  {
    value_checker_.push_back(OptionValueChecker<parsed_value_type>(
        [r_min, r_max](const parsed_value_type &val) {
          return r_min <= val && val <= r_max;
        },
        "not in range: [" + std::to_string(r_min) + "~" +
            std::to_string(r_max) + "]"));
    return *this;
  }

  using OptionBaseCRTP<Option<T>>::choices;
  Option<T> &choices(std::vector<parsed_value_type> const &allowed_values) {
    std::string err_msg{};
    if constexpr (std::is_same_v<parsed_value_type, std::string> ||
                  detail::has_std_to_string_v<parsed_value_type> ||
                  std::is_constructible_v<parsed_value_type, std::string>) {
      std::vector<std::string> tmp;
      std::transform(
          begin(allowed_values), end(allowed_values), std::back_inserter(tmp),
          [](parsed_value_type const &v) -> std::string {
            if constexpr (detail::has_std_to_string_v<parsed_value_type>) {
              return std::to_string(v);
            } else {
              return v;
            }
          });
      err_msg = "not in choices: [" + detail::join(tmp, ',') + "]";
    }
    Option<T>::checker(
        [allowed_values](parsed_value_type const &val) {
          return std::find(begin(allowed_values), end(allowed_values), val) !=
                 end(allowed_values);
        },
        err_msg);
    return *this;
  }

  T const &value() const { return bind_value_; }

 protected:
  bool is_option() const override final { return true; }
  bool is_positional() const override final { return false; }
  void parse(const std::string &opt_value) override {
    OptionBaseCRTP<Option<T>>::parse(opt_value);
    auto parsed_value = parse_function_(opt_value);
    for (const auto &checker : value_checker_) {
      if (!checker(parsed_value)) {
        std::string err_msg = "check failed: ";
        err_msg += detail::join(ArgBase::long_opt_names_, ',');
        err_msg += detail::join(ArgBase::short_opt_names_, ',');
        err_msg += ": `";
        err_msg += opt_value;
        err_msg += "` is a invalid value. ";
        throw std::invalid_argument(err_msg + checker.error_message());
      }
    }
    if constexpr (detail::ParseFromStringContainerType<T>) {
      bind_value_.get().insert(bind_value_.get().end(),
                               std::move(parsed_value));
    } else {
      bind_value_.get() = std::move(parsed_value);
    }
    if (callback_) {
      if constexpr (detail::is_optional_v<T>) {
        callback_(bind_value_.get().value());
      } else {
        callback_(bind_value_.get());
      }
    }
  }
  bool is_multiple() const override {
    if constexpr (detail::ParseFromStringContainerType<T>) {
      return true;
    } else {
      return false;
    }
  }
  void use_default_if_needed() override {
    if (ArgBase::count() != 0) {
      return;
    }
    if constexpr (detail::ParseFromStringContainerType<T>) {
      if (default_value_.has_value()) {
        for (const auto &value : default_value_.value()) {
          parse(value);
        }
      }
    } else {
      if (default_value_.has_value()) {
        parse(default_value_.value());
      }
    }
    ArgBase::count_ = 0;
  }
  std::optional<std::string> get_default_value() const override {
    if constexpr (detail::ParseFromStringContainerType<T>) {
      if (default_value_.has_value()) {
        return "{" + detail::join(default_value_.value(), ',') + "}";
      }
    } else {
      if (default_value_.has_value()) {
        return default_value_.value();
      }
    }
    return std::nullopt;
  }

 private:
  std::reference_wrapper<T> bind_value_;
  std::function<parsed_value_type(std::string const &)> parse_function_;
  std::conditional_t<detail::ParseFromStringContainerType<T>,
                     std::optional<std::vector<std::string>>,
                     std::optional<std::string>>
      default_value_;
  std::function<void(value_type const &)> callback_;
  std::vector<OptionValueChecker<parsed_value_type>> value_checker_;
};

class OptionAlias : public FlagBase {
 public:
  OptionAlias(std::string const &name, OptionBase *option,
              std::string const &opt_value)
      : FlagBase(name, "same as " +
                           (option->long_opt_names_.empty()
                                ? (option->short_opt_names_.empty()
                                       ? std::string("")
                                       : ("-" + option->short_opt_names_[0]))
                                : ("--" + option->long_opt_names_[0])) +
                           " " + opt_value),
        option_{option},
        opt_value_{opt_value} {}
  void parse() override {
    if (option_) {
      option_->parse(opt_value_);
    }
  }

  void negatable_parse() override {}

 private:
  OptionBase *option_{nullptr};
  std::string opt_value_{};
};

template <detail::BindableType T>
class Positional final : public OptionBaseCRTP<Positional<T>> {
  friend class Command;
  friend class ArgParser;

 public:
  using value_type = std::conditional_t<detail::is_optional_v<T>,
                                        detail::extract_value_type_t<T>, T>;
  using parsed_value_type = detail::extract_value_type_t<T>;

 public:
  Positional(const std::string &name, const std::string &description,
             T &bind_value)
    requires detail::BindableWithoutDelimiterType<T>
      : OptionBaseCRTP<Positional<T>>(name, description),
        bind_value_(std::ref(bind_value)) {
    OptionBase::set_default_value_help<T>();
    if constexpr (detail::ParseFromStringContainerType<T>) {
      parse_function_ = [](std::string const &opt_value) {
        return detail::parse_from_string<parsed_value_type>(opt_value);
      };
    } else if constexpr (detail::is_optional_v<T>) {
      parse_function_ = [](std::string const &opt_value) {
        return detail::parse_from_string<parsed_value_type>(opt_value);
      };
    } else {
      parse_function_ = [](std::string const &opt_value) {
        return detail::parse_from_string<T>(opt_value);
      };
    }
  }
  Positional(const std::string &name, const std::string &description,
             T &bind_value, char delim)
    requires detail::BindableWithDelimiterType<T>
      : OptionBaseCRTP<Positional<T>>(name, description),
        bind_value_(std::ref(bind_value)) {
    OptionBase::set_default_value_help<T>();
    if constexpr (detail::ParseFromStringContainerType<T>) {
      parse_function_ = [delim](std::string const &opt_value) {
        return detail::parse_from_string<parsed_value_type>(opt_value, delim);
      };
    } else if constexpr (detail::is_optional_v<T>) {
      parse_function_ = [delim](std::string const &opt_value) {
        return detail::parse_from_string<parsed_value_type>(opt_value, delim);
      };
    } else {
      parse_function_ = [delim](std::string const &opt_value) {
        return detail::parse_from_string<T>(opt_value, delim);
      };
    }
  }
  Positional<T> &default_value(const std::string &default_value)
    requires(!detail::ParseFromStringContainerType<T>)
  {
    this->default_value_ = default_value;
    return *this;
  }
  Positional<T> &default_value(std::initializer_list<std::string> default_value)
    requires detail::ParseFromStringContainerType<T>
  {
    this->default_value_ = default_value;
    return *this;
  }

  Positional<T> &callback(std::function<void(value_type const &)> cb) {
    callback_ = std::move(cb);
  }

  T const &value() const { return bind_value_; }

  Positional<T> &checker(
      std::function<bool(const parsed_value_type &)> check_function,
      std::string description = "") {
    value_checker_.push_back(OptionValueChecker<parsed_value_type>(
        std::move(check_function), std::move(description)));
    return *this;
  }

  Positional<T> &range(parsed_value_type r_min, parsed_value_type r_max)
    requires std::is_arithmetic_v<parsed_value_type>
  {
    value_checker_.push_back(OptionValueChecker<parsed_value_type>(
        [r_min, r_max](const parsed_value_type &val) {
          return r_min <= val && val <= r_max;
        },
        "not in range: [" + std::to_string(r_min) + "~" +
            std::to_string(r_max) + "]"));
    return *this;
  }

  using OptionBaseCRTP<Positional<T>>::choices;
  Positional<T> &choices(std::vector<parsed_value_type> const &allowed_values) {
    std::string err_msg{};
    if constexpr (std::is_same_v<parsed_value_type, std::string> ||
                  detail::has_std_to_string_v<parsed_value_type> ||
                  std::is_constructible_v<parsed_value_type, std::string>) {
      std::vector<std::string> tmp;
      std::transform(
          begin(allowed_values), end(allowed_values), std::back_inserter(tmp),
          [](parsed_value_type const &v) -> std::string {
            if constexpr (detail::has_std_to_string_v<parsed_value_type>) {
              return std::to_string(v);
            } else {
              return v;
            }
          });
      err_msg = "not in choices: [" + detail::join(tmp, ',') + "]";
    }
    Positional<T>::checker(
        [allowed_values](parsed_value_type const &val) {
          return std::find(begin(allowed_values), end(allowed_values), val) !=
                 end(allowed_values);
        },
        err_msg);
    return *this;
  }

 protected:
  bool is_option() const override final { return false; }
  bool is_positional() const override final { return true; }
  void parse(const std::string &opt_value) override {
    OptionBaseCRTP<Positional<T>>::parse(opt_value);
    auto parsed_value = parse_function_(opt_value);
    for (const auto &checker : value_checker_) {
      if (!checker(parsed_value)) {
        std::string err_msg = "check failed: ";
        err_msg += detail::join(ArgBase::long_opt_names_, ',');
        err_msg += detail::join(ArgBase::short_opt_names_, ',');
        err_msg += ": `";
        err_msg += opt_value;
        err_msg += "` is a invalid value. ";
        throw std::invalid_argument(err_msg + checker.error_message());
      }
    }
    if constexpr (detail::ParseFromStringContainerType<T>) {
      bind_value_.get().insert(bind_value_.get().end(),
                               std::move(parsed_value));
    } else {
      bind_value_.get() = std::move(parsed_value);
    }
    if (callback_) {
      if constexpr (detail::is_optional_v<T>) {
        callback_(bind_value_.get().value());
      } else {
        callback_(bind_value_.get());
      }
    }
  }
  bool is_multiple() const override {
    if constexpr (detail::ParseFromStringContainerType<T>) {
      return true;
    } else {
      return false;
    }
  }
  void use_default_if_needed() override {
    if (ArgBase::count() != 0) {
      return;
    }
    if constexpr (detail::ParseFromStringContainerType<T>) {
      if (default_value_.has_value()) {
        for (const auto &value : default_value_.value()) {
          parse(value);
        }
      }
    } else {
      if (default_value_.has_value()) {
        parse(default_value_.value());
      }
    }
    ArgBase::count_ = 0;
  }
  std::optional<std::string> get_default_value() const override {
    if constexpr (detail::ParseFromStringContainerType<T>) {
      if (default_value_.has_value()) {
        return "{" + detail::join(default_value_.value(), ',') + "}";
      }
    } else {
      if (default_value_.has_value()) {
        return default_value_.value();
      }
    }
    return std::nullopt;
  }

 private:
  std::reference_wrapper<T> bind_value_;
  std::function<parsed_value_type(std::string const &)> parse_function_;
  std::conditional_t<detail::ParseFromStringContainerType<T>,
                     std::optional<std::vector<std::string>>,
                     std::optional<std::string>>
      default_value_;
  std::function<void(value_type const &)> callback_;
  std::vector<OptionValueChecker<parsed_value_type>> value_checker_;
};

class Command {
  template <typename T>
    requires std::same_as<T, bool> || std::same_as<std::optional<bool>, T>
  Flag<T> &add_flag_bool(
      const std::string &name, const std::string &description, T &bind_value,
      std::function<void(detail::extract_value_type_t<T> &)> action =
          store_true,
      std::function<void(detail::extract_value_type_t<T> &)> negatable_action =
          store_false) {
    auto flag = std::make_unique<Flag<T>>(name, description, bind_value,
                                          std::move(action),
                                          std::move(negatable_action));
    auto &ret = *(flag.get());
    if (flag_or_option_exists(ret)) {
      throw std::runtime_error("Flag or Option already exists: " + name);
    }
    args_.push_back(std::move(flag));
    return ret;
  }
  template <typename T>
    requires detail::is_integral_v<T> ||
             (detail::is_optional_v<T> &&
              detail::is_integral_v<typename T::value_type>)
  Flag<T> &add_flag_int(
      const std::string &name, const std::string &description, T &bind_value,
      std::function<void(detail::extract_value_type_t<T> &)> action =
          increment<detail::extract_value_type_t<T>>,
      std::function<void(detail::extract_value_type_t<T> &)> negatable_action =
          decrement<detail::extract_value_type_t<T>>) {
    auto flag = std::make_unique<Flag<T>>(name, description, bind_value,
                                          std::move(action),
                                          std::move(negatable_action));
    auto &ret = *(flag.get());
    if (flag_or_option_exists(ret)) {
      throw std::runtime_error("Flag or Option already exists: " + name);
    }
    args_.push_back(std::move(flag));
    return ret;
  }

 public:
  Command(std::string cmd, std::string description)
      : command_{std::move(cmd)}, description_(std::move(description)) {}
  virtual ~Command() {}
  template <typename T>
    requires std::same_as<T, bool> || std::same_as<std::optional<bool>, T>
  Flag<T> &add_flag(const std::string &name, const std::string &description,
                    T &bind_value) {
    std::function<void(detail::extract_value_type_t<T> &)> action = store_true;
    std::function<void(detail::extract_value_type_t<T> &)> negatable_action =
        store_false;
    return add_flag_bool(name, description, bind_value, std::move(action),
                         std::move(negatable_action));
  }
  template <typename T>
    requires detail::is_integral_v<T> ||
             (detail::is_optional_v<T> &&
              detail::is_integral_v<typename T::value_type>)
  Flag<T> &add_flag(const std::string &name, const std::string &description,
                    T &bind_value) {
    std::function<void(detail::extract_value_type_t<T> &)> action =
        increment<detail::extract_value_type_t<T>>;
    std::function<void(detail::extract_value_type_t<T> &)> negatable_action =
        decrement<detail::extract_value_type_t<T>>;
    return add_flag_int(name, description, bind_value, std::move(action),
                        std::move(negatable_action));
  }

  template <typename T>
    requires std::same_as<T, bool> || std::same_as<std::optional<bool>, T>
  Flag<T> &add_negative_flag(const std::string &name,
                             const std::string &description, T &bind_value) {
    std::function<void(detail::extract_value_type_t<T> &)> action = store_false;
    std::function<void(detail::extract_value_type_t<T> &)> negatable_action =
        store_true;
    return add_flag_bool(name, description, bind_value, std::move(action),
                         std::move(negatable_action));
  }
  template <typename T>
    requires std::same_as<T, int> || std::same_as<std::optional<int>, T>
  Flag<T> &add_negative_flag(const std::string &name,
                             const std::string &description, T &bind_value) {
    std::function<void(detail::extract_value_type_t<T> &)> action =
        decrement<detail::extract_value_type_t<T>>;
    std::function<void(detail::extract_value_type_t<T> &)> negatable_action =
        increment<detail::extract_value_type_t<T>>;
    return add_flag_int(name, description, bind_value, std::move(action),
                        std::move(negatable_action));
  }

  FlagBase &add_alias(const std::string &name, const std::string &opt_name,
                      const std::string &opt_value) {
    auto *opt = get(opt_name);
    if (!opt) {
      throw std::runtime_error("Not found option: " + opt_name);
    }
    if (!opt->is_option()) {
      throw std::runtime_error("Not an option: " + opt_name);
    }

    auto alias =
        std::make_unique<OptionAlias>(name, (OptionBase *)opt, opt_value);

    auto &ret = *(alias.get());
    if (flag_or_option_exists(ret)) {
      throw std::runtime_error("Flag or Option already exists: " + name);
    }
    args_.push_back(std::move(alias));
    return ret;
  }

  template <detail::BindableWithoutDelimiterType T>
  Option<T> &add_option(const std::string &name, const std::string &description,
                        T &bind_value) {
    auto option = std::make_unique<Option<T>>(name, description, bind_value);
    auto &ret = *(option.get());
    if (flag_or_option_exists(ret)) {
      throw std::runtime_error("Flag or Option already exists: " + name);
    }
    args_.push_back(std::move(option));
    return ret;
  }

  template <detail::BindableWithDelimiterType T>
  Option<T> &add_option(const std::string &name, const std::string &description,
                        T &bind_value, char delim) {
    auto option =
        std::make_unique<Option<T>>(name, description, bind_value, delim);
    auto &ret = *(option.get());
    if (flag_or_option_exists(ret)) {
      throw std::runtime_error("Flag or Option already exists: " + name);
    }
    args_.push_back(std::move(option));
    return ret;
  }

  template <detail::BindableWithoutDelimiterType T>
  Positional<T> &add_positional(const std::string &name,
                                const std::string &description, T &bind_value) {
    if (!subcommands_.empty()) {
      throw std::runtime_error(
          "Cannot add positional parameter when there are sub commands");
    }
    if (std::ranges::find_if(args_, [](const auto &arg) {
          return arg->is_positional() &&
                 dynamic_cast<OptionBase *>(arg.get())->is_multiple();
        }) != args_.end()) {
      throw std::runtime_error(
          "Positional argument only support one container and it "
          "must be the last one");
    }
    auto positional =
        std::make_unique<Positional<T>>(name, description, bind_value);
    auto &ret = *(positional.get());
    if (positional_exists(ret)) {
      throw std::runtime_error("Positional already exists: " + name);
    }
    args_.push_back(std::move(positional));
    return ret;
  }

  template <detail::BindableWithDelimiterType T>
  Positional<T> &add_positional(const std::string &name,
                                const std::string &description, T &bind_value,
                                char delim) {
    if (!subcommands_.empty()) {
      throw std::runtime_error(
          "Cannot add positional parameter when there are sub commands");
    }
    auto positional =
        std::make_unique<Positional<T>>(name, description, bind_value, delim);
    auto &ret = *(positional.get());
    if (positional_exists(ret)) {
      throw std::runtime_error("Positional already exists: " + name);
    }
    args_.push_back(std::move(positional));
    return ret;
  }

  ArgBase *get(const std::string &name, bool find_from_parent = true) {
    if (name.length() == 1) {
      auto it = std::ranges::find_if(args_, [name](const auto &arg) {
        return std::find(arg->short_opt_names_.begin(),
                         arg->short_opt_names_.end(),
                         name) != arg->short_opt_names_.end();
      });
      return it != args_.end()
                 ? it->get()
                 : ((find_from_parent && parent_) ? parent_->get(name)
                                                  : nullptr);
    } else {
      auto it = std::ranges::find_if(args_, [name](const auto &arg) {
        return std::find(arg->long_opt_names_.begin(),
                         arg->long_opt_names_.end(),
                         name) != arg->long_opt_names_.end();
      });
      return it != args_.end()
                 ? it->get()
                 : ((find_from_parent && parent_) ? parent_->get(name)
                                                  : nullptr);
    }
  }
  ArgBase &operator[](const std::string &name) {
    if (auto *arg = get(name); arg != nullptr) {
      return *arg;
    } else {
      throw std::runtime_error("Unknown option: " + name);
    }
  }

  Command &callback(std::function<void()> cb) {
    this->callback_ = std::move(cb);
    return *this;
  }

  void parse(int argc, char const *const *argv) {
    this->is_parsed_ = true;
    std::vector<const char *> commands{argv, argv + argc};
    ARG_PARSER_DEBUG(
        detail::join(std::vector<std::string>{argv, argv + argc}, ' '));
    size_t i = 1;  // Skip program name
    if (!commands.empty() && commands[0] != nullptr && commands[0][0] == '-') {
      i = 0;
    }
    std::vector<ArgBase *> positionals;

    // Collect all positional arguments
    for (const auto &arg : args_) {
      if (arg->is_positional()) {
        positionals.push_back(arg.get());
      }
    }

    size_t pos_index = 0;

    while (i < commands.size()) {
      const std::string &arg = commands[i];

      // Handle long options (--option)
      if (arg.size() > 2 && arg.substr(0, 2) == "--") {
        std::string name = arg.substr(2);
        auto eq_pos = name.find('=');
        std::string value;

        if (eq_pos != std::string::npos) {
          value = name.substr(eq_pos + 1);
          name = name.substr(0, eq_pos);
        }

        if (auto *option = get(name); option != nullptr) {
          if (option->is_flag()) {
            auto *flag = dynamic_cast<FlagBase *>(option);
            ARG_PARSER_DEBUG("flag: " << name);
            flag->parse();
          } else if (option->is_option()) {
            auto *opt = dynamic_cast<OptionBase *>(option);
            if (eq_pos != std::string::npos) {
              ARG_PARSER_DEBUG("option: " << name << "=" << value);
              opt->parse(value);
            } else if (i + 1 < commands.size()) {
              auto val = commands[++i];
              ARG_PARSER_DEBUG("option: " << name << "=" << val);
              opt->parse(val);
            } else {
              throw std::runtime_error("Missing value for option: " + name);
            }
          }
        } else if (name.length() > 3 && name.substr(0, 3) == "no-") {
          name = name.substr(3);
          if (auto *option = get(name); option != nullptr) {
            if (option->is_flag() &&
                dynamic_cast<FlagBase *>(option)->is_negatable()) {
              auto *flag = dynamic_cast<FlagBase *>(option);
              flag->negatable_parse();
            } else {
              throw std::runtime_error("Unknown option: " + name);
            }
          }
        } else {
          throw std::runtime_error("Unknown option: " + name);
        }
      }
      // Handle short options (-o)
      else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
        std::string opts = arg.substr(1);

        // Handle combined short options
        for (size_t j = 0; j < opts.size(); ++j) {
          std::string name(1, opts[j]);
          if (auto *option = get(name)) {
            if (option->is_flag()) {
              auto *flag = dynamic_cast<FlagBase *>(option);
              ARG_PARSER_DEBUG("flag: " << name);
              flag->parse();
            } else if (option->is_option()) {
              auto *opt = dynamic_cast<OptionBase *>(option);
              if (j < opts.size() - 1) {
                ARG_PARSER_DEBUG("option: " << name << "="
                                            << opts.substr(j + 1));
                // If not the last character, use the rest as
                // value
                opt->parse(opts.substr(j + 1));
                break;
              } else if (i + 1 < commands.size()) {
                auto val = commands[++i];
                ARG_PARSER_DEBUG("option: " << name << "=" << val);
                opt->parse(val);
              } else {
                throw std::runtime_error("Missing value for option: " + name);
              }
            }
          } else {
            throw std::runtime_error("Unknown option: " + name);
          }
        }
      }
      // Handle -- option
      else if (arg == "--") {
        i++;
        break;
      }
      // Handle positional arguments
      else {
        if (pos_index < positionals.size()) {
          if (remaining_are_positional()) {
            break;
          }
          auto *pos = dynamic_cast<OptionBase *>(positionals[pos_index]);
          ARG_PARSER_DEBUG("positional: " << pos_index << ": " << arg);
          pos->parse(arg);
          if (!pos->is_multiple()) {
            pos_index++;
          }
        } else {
          if (!subcommands_.empty()) {
            auto subcmd_ptr_it =
                std::find_if(begin(subcommands_), end(subcommands_),
                             [&arg](auto sub) { return sub->command_ == arg; });
            if (subcmd_ptr_it != end(subcommands_)) {
              ARG_PARSER_DEBUG(
                  "subcmd: " << detail::join(
                      std::vector<std::string>{argv + i, argv + argc}, ' '));
              return (*subcmd_ptr_it)->parse(argc - i, argv + i);
            } else {
              throw std::runtime_error("unkonwn subcommand: " + arg);
            }
          } else {
            throw std::runtime_error("Too many positional arguments");
          }
        }
      }
      ++i;
    }

    while (i < commands.size()) {
      if (pos_index < positionals.size()) {
        auto *pos = dynamic_cast<OptionBase *>(positionals[pos_index]);
        ARG_PARSER_DEBUG("positional: " << pos_index << ": " << commands[i]);
        pos->parse(commands[i]);
        if (!pos->is_multiple()) {
          pos_index++;
        }
      } else {
        throw std::runtime_error("Too many positional arguments");
      }
      ++i;
    }

    // Handle environment
    for (const auto &arg : args_) {
      if ((arg->is_option() || arg->is_positional()) && arg->count() == 0) {
        dynamic_cast<OptionBase *>(arg.get())->use_env_if_needed();
      }
    }
    // Handle options that were not provided but have default values
    for (const auto &arg : args_) {
      if ((arg->is_option() || arg->is_positional()) && arg->count() == 0) {
        dynamic_cast<OptionBase *>(arg.get())->use_default_if_needed();
      }
    }
    if (callback_) {
      callback_();
    }
  }

  virtual std::string usage() const {
    std::stringstream usage_str;

    if (parent_ != nullptr) {
      std::vector<std::string> parent_cmds;
      Command *p = parent_;
      while (p != nullptr) {
        parent_cmds.push_back(p->command());
        p = p->parent_;
      }
      std::copy(parent_cmds.rbegin(), parent_cmds.rend(),
                std::ostream_iterator<std::string>(usage_str, " "));
    }

    usage_str << command_;
    if (std::ranges::find_if(args_, [](const auto &arg) {
          return arg->is_option() || arg->is_flag();
        }) != args_.end()) {
      usage_str << " [options]...";
    }
    if (!subcommands_.empty()) {
      usage_str << " [cmd] [options]...";
    }
    auto positionals = args_ | std::views::filter([](const auto &arg) {
                         return arg->is_positional();
                       });
    for (const auto &arg : positionals) {
      if (dynamic_cast<OptionBase *>(arg.get())->is_multiple()) {
        usage_str << " <" << arg->long_opt_names_.front() << ">...";
      } else {
        usage_str << " <" << arg->long_opt_names_.front() << ">";
      }
    }
    if (std::ranges::find_if(args_, [](const auto &arg) {
          return arg->is_option() || arg->is_flag();
        }) != args_.end()) {
      usage_str << "\n\nOptions:";
    }
    for (const auto &arg : args_) {
      if ((arg->is_option() || arg->is_flag()) && !arg->hidden_) {
        usage_str << "\n " << arg->usage();
      }
    }

    if (std::ranges::find_if(args_, [](const auto &arg) {
          return arg->is_positional();
        }) != args_.end()) {
      usage_str << "\n\nPositionals:";
    }
    for (const auto &arg : args_) {
      if (arg->is_positional() && !arg->hidden_) {
        usage_str << "\n " << arg->usage();
      }
    }
    return usage_str.str();
  }
  std::string oneline_usage() {
    std::stringstream usage_str;
    usage_str << detail::format(command_, option_width(), description_);
    return usage_str.str();
  }
  void add_default_help_flag() {
    static bool default_help{false};
    auto *const help = get("help", false);
    if (nullptr == help) {
      auto *const h = get("h", false);
      std::string help_name = "help";
      if (nullptr == h) {
        help_name = "h,help";
      }
      auto &f =
          add_flag(help_name, "Display this help information", default_help);
      f.set_option_width(this->option_width());
      f.callback([this](bool v) {
        if (v) {
          print_usage();
          std::exit(0);
        }
      });
    }
  }

  virtual void print_usage() const { std::cerr << usage() << "\n"; }

  std::string const &command() const { return command_; }
  std::string const &name() const { return command_; }
  Command &hidden(bool v = true) {
    is_hidden_ = v;
    return *this;
  }
  void set_parent(Command *parent) { parent_ = parent; }
  bool has_parent() const { return parent_ != nullptr; }
  bool is_parsed() { return is_parsed_; }
  bool is_hidden() const { return is_hidden_; }
  void set_remaining_are_positional() { remaining_are_positional_ = true; }
  bool remaining_are_positional() { return remaining_are_positional_; }

 protected:
  size_t option_width() { return option_width_; }
  void set_option_width(size_t width) {
    option_width_ = width;
    for (auto &arg : args_) {
      arg->set_option_width(width);
    }
    for (auto &cmd : subcommands_) {
      cmd->set_option_width(width);
    }
  }
  bool flag_or_option_exists(ArgBase &new_arg) const {
    return flag_exists(new_arg) || option_exists(new_arg);
  }
  bool flag_exists(ArgBase &new_arg) const {
    return option_name_exists(new_arg, 0);
  }
  bool option_exists(ArgBase &new_arg) const {
    return option_name_exists(new_arg, 1);
  }
  bool positional_exists(ArgBase &new_arg) const {
    return option_name_exists(new_arg, 2);
  }
  bool option_name_exists(ArgBase &new_arg, int type) const {
    for (const auto &arg : args_) {
      if (type == 0 && !arg->is_flag()) {
        continue;
      }
      if (type == 1 && !arg->is_option()) {
        continue;
      }
      if (type == 2 && !arg->is_positional()) {
        continue;
      }
      for (auto &name : arg->long_opt_names_) {
        if (std::find(new_arg.long_opt_names_.begin(),
                      new_arg.long_opt_names_.end(),
                      name) != new_arg.long_opt_names_.end()) {
          return true;
        }
      }
      for (auto &name : arg->short_opt_names_) {
        if (std::find(new_arg.short_opt_names_.begin(),
                      new_arg.short_opt_names_.end(),
                      name) != new_arg.short_opt_names_.end()) {
          return true;
        }
      }
    }
    return false;
  }
  std::vector<std::unique_ptr<ArgBase>> args_;
  std::string command_;
  std::string description_;
  std::vector<std::shared_ptr<Command>> subcommands_;
  Command *parent_{nullptr};
  std::function<void()> callback_{nullptr};
  bool is_parsed_{false};
  bool is_hidden_{false};
  bool remaining_are_positional_{false};
  size_t option_width_{OPTION_NAME_WIDTH};
};

class ArgParser : public Command {
 public:
  ArgParser(std::string prog, std::string description)
      : Command(prog, description) {}
  virtual ~ArgParser() {}
  using Command::add_flag;
  using Command::add_option;
  using Command::add_positional;
  std::string usage() const override {
    std::stringstream usage_str;
    if (!description_.empty()) {
      usage_str << description_ << "\n";
    }
    usage_str << "\nUsage: \n  ";
    usage_str << this->Command::usage();

    if (!subcommands_.empty()) {
      usage_str << "\n\nAvailable Commands:";
      for (auto const &cmd : subcommands_) {
        if (cmd->is_hidden()) {
          continue;
        }
        usage_str << "\n " << cmd->oneline_usage();
      }
    }

    return usage_str.str();
  }
  void print_usage() const override { std::cerr << this->usage() << '\n'; }
  void print_version() const {
    std::cerr << "Version: " << version << std::endl;
  }
  Command &parse(int argc, const char *argv[]) {
    add_default_help_flag();
    for (auto &sc : subcommands_) {
      sc->add_default_help_flag();
    }
    Command::parse(argc, argv);
    auto it = std::find_if(begin(subcommands_), end(subcommands_),
                           [](auto &cmdptr) { return cmdptr->is_parsed(); });
    if (it != end(subcommands_)) {
      return **it;
    }
    return *this;
  }

  Command &add_command(std::string const &cmd, std::string const &description) {
    if (std::any_of(begin(args_), end(args_),
                    [](auto const &a) { return a->is_positional(); })) {
      throw std::runtime_error(
          "Cannot add sub command when there are positionals");
    }
    auto cmd_ptr = std::make_shared<Command>(cmd, description);
    cmd_ptr->set_parent(this);
    subcommands_.push_back(cmd_ptr);
    return *cmd_ptr;
  }
  void set_option_width(int width) { this->Command::set_option_width(width); }

 private:
  std::string version{"0.1"};
};

}  // namespace argparse

#endif  // __ARGPARSE_ARGPARSE_HPP__
