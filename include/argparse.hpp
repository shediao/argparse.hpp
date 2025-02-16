//
// Created by shediao on 24-12-29.
//

#ifndef ARG_PASER_HPP
#define ARG_PASER_HPP

#include <algorithm>
#include <cctype>
#include <concepts>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace argparse {

namespace {

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_optional_v = is_optional<T>::value;

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

template <typename T>
concept ParseFromStringBasicType =
    is_string_v<std::remove_cv_t<T>> ||
    std::same_as<std::remove_cv_t<T>, bool> ||
    std::same_as<std::remove_cv_t<T>, char> ||
    std::same_as<std::remove_cv_t<T>, int> ||
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
    is_optional_v<T> && ParseFromStringSingleType<typename T::value_type>;

template <typename T>
concept ParseFromStringOptionalTupleLikeType =
    is_optional_v<T> && ParseFromStringTupleLikeType<typename T::value_type>;

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
            throw std::invalid_argument("Invalid string for unsigned long: " +
                                        s);
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
            throw std::invalid_argument(
                "Invalid string for unsigned long long: " + s);
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
    auto v = split(s, delim, std::tuple_size_v<std::decay_t<T>>);
    if (v.size() != std::tuple_size_v<std::decay_t<T>>) {
        throw std::invalid_argument(
            "Invalid string for split " +
            std::to_string(std::tuple_size_v<std::decay_t<T>>) +
            "th element:" + s);
    }
    return make_tuple_from_container<T>(v);
}

std::string format(std::string const &option_name, int width,
                   std::string const &description) {
    std::string ret;
    if (option_name.length() > static_cast<size_t>(width)) {
        ret = option_name + " " + description;
    } else {
        ret = option_name + std::string(width - option_name.length(), ' ') +
              description;
    }
    return ret;
}

}  // namespace

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
    friend class ArgParser;

   public:
    ArgBase(const std::string &name, const std::string &description)
        : description_(description) {
        for (auto &&opt_name : split(name, ',', -1)) {
            if (opt_name.empty()) {
                continue;
            }
            if (opt_name[0] == '-') {
                throw std::invalid_argument("Invalid option name: " + name +
                                            ", " + opt_name +
                                            " starts with '-'");
            }
            if (std::find_if(opt_name.begin(), opt_name.end(),
                             [](unsigned char c) { return std::isblank(c); }) !=
                opt_name.end()) {
                throw std::invalid_argument("Invalid option name: " + name +
                                            ", " + opt_name +
                                            " contains blank");
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

   protected:
    virtual bool is_flag() const = 0;
    virtual bool is_option() const = 0;
    virtual bool is_positional() const = 0;
    virtual std::string usage(int option_width) const = 0;
    const std::string &description() const { return description_; }
    size_t count_{0};
    std::vector<std::string> short_opt_names_;
    std::vector<std::string> long_opt_names_;
    std::string description_;
};

class FlagBase : public ArgBase {
    friend class ArgParser;

   public:
    FlagBase(const std::string &name, const std::string &description)
        : ArgBase(name, description) {}

   protected:
    bool is_flag() const override final { return true; }
    bool is_option() const override final { return false; }
    bool is_positional() const override final { return false; }
    virtual void parse() = 0;
    std::string usage(int option_width) const override {
        std::stringstream usage_str;
        std::string options_str{};
        std::vector<std::string> short_opts;
        std::vector<std::string> long_opts;
        std::ranges::transform(short_opt_names_, back_inserter(short_opts),
                               [](auto const &s) { return "-" + s; });
        std::ranges::transform(long_opt_names_, back_inserter(long_opts),
                               [](auto const &s) { return "--" + s; });
        if (!short_opts.empty()) {
            options_str += join(short_opts, ',');
            if (!long_opts.empty()) {
                options_str += ", ";
            }
        } else {
            options_str += "    ";
        }
        if (!long_opts.empty()) {
            options_str += join(long_opts, ',');
        }
        usage_str << format(options_str, option_width, description());
        return usage_str.str();
    }
};

template <typename T = bool>
    requires std::integral<T> || std::is_same_v<T, bool> ||
             (is_optional_v<T> &&
              (std::integral<typename T::value_type> ||
               std::is_same_v<typename T::value_type, bool>))
class Flag final : public FlagBase {
    friend class ArgParser;

   public:
    Flag(const std::string &name, const std::string &description, T &bind_value)
        : FlagBase(name, description),
          bind_value_(std::ref(bind_value)),
          parse_function_{store_true} {}
    Flag(const std::string &name, const std::string &description, T &bind_value,
         std::function<void(extract_value_type_t<T> &)> action)
        : FlagBase(name, description),
          bind_value_(std::ref(bind_value)),
          parse_function_(std::move(action)) {}

    T const &value() const { return bind_value_; }

   protected:
    void parse() override {
        if constexpr (is_optional_v<T>) {
            auto &flag_value = bind_value_.get();
            if (!flag_value.has_value()) {
                flag_value = typename T::value_type{};
            }
            parse_function_(flag_value.value());
        } else {
            parse_function_(bind_value_.get());
        }
        count_++;
    }

   private:
    std::reference_wrapper<T> bind_value_;
    std::function<void(extract_value_type_t<T> &)> parse_function_;
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
    friend class ArgParser;

   public:
    OptionBase(const std::string &name, const std::string &description)
        : ArgBase(name, description) {}

    OptionBase &choices(std::initializer_list<std::string> choices) {
        choices_ = std::vector<std::string>(choices);
        value_checker_.push_back(OptionValueChecker<std::string>(
            [this](const std::string &value) {
                return std::ranges::find(choices_, value) !=
                       std::ranges::end(choices_);
            },
            "not in choices: " + join(choices, ',')));
        return *this;
    }

   protected:
    bool is_flag() const override final { return false; }
    virtual void parse(const std::string &opt_value) {
        for (const auto &checker : value_checker_) {
            if (!checker(opt_value)) {
                throw std::invalid_argument("check failed: (" + opt_value +
                                            "): " + checker.error_message());
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
        if constexpr (std::is_integral_v<T>) {
            value_help = "<N>";
        } else if constexpr (std::is_floating_point_v<T>) {
            value_help = "<0.0>";
        } else {
            value_help = "<arg>";
        }
    }
    std::string usage(int option_width) const override {
        constexpr int max_width = 80;
        std::stringstream usage_str;
        if (is_option()) {
            std::string options_str{};
            std::vector<std::string> short_opts;
            std::vector<std::string> long_opts;
            std::ranges::transform(short_opt_names_, back_inserter(short_opts),
                                   [](auto const &s) { return "-" + s; });
            std::ranges::transform(long_opt_names_, back_inserter(long_opts),
                                   [](auto const &s) { return "--" + s; });
            if (!short_opts.empty()) {
                options_str += join(short_opts, ',');
                if (!long_opts.empty()) {
                    options_str += ", ";
                }
            } else {
                options_str += "    ";
            }
            if (!long_opts.empty()) {
                options_str += join(long_opts, ',');
            }
            options_str += (" " + value_help);
            if (is_multiple()) {
                options_str += "...";
            }
            usage_str << format(options_str, option_width, description());
            if (auto default_value = get_default_value();
                default_value.has_value()) {
                auto default_value_string =
                    " (default: " + *default_value + ")";
                if (option_width + description().length() +
                        default_value_string.length() >
                    max_width) {
                    usage_str << "\n"
                              << format("", option_width, default_value_string);
                } else {
                    usage_str << default_value_string;
                }
            }
            if (!choices_.empty()) {
                auto choices_string = " (choices: " + join(choices_, ',') + ")";
                usage_str << "\n" << format("", option_width, choices_string);
            }
        } else {
            std::string options_str{long_opt_names_[0]};
            if (is_multiple()) {
                options_str += "...";
            }
            usage_str << format(options_str, option_width, description());
            if (auto default_value = get_default_value();
                default_value.has_value()) {
                auto default_value_string =
                    " (default: " + *default_value + ")";
                if (option_width + description().length() +
                        default_value_string.length() >
                    max_width) {
                    usage_str << "\n"
                              << format("", option_width, default_value_string);
                } else {
                    usage_str << default_value_string;
                }
            }
        }
        return usage_str.str();
    }
    std::string value_help;
    std::vector<std::string> opt_values;
    std::vector<OptionValueChecker<std::string>> value_checker_;
    std::vector<std::string> choices_;
};

template <BindableType T>
class Option final : public OptionBase {
    friend class ArgParser;

   public:
    Option(const std::string &name, const std::string &description,
           T &bind_value)
        requires BindableWithoutDelimiterType<T>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          parse_function_(parse_from_string<extract_value_type_t<T>>) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value, char delim)
        requires BindableWithDelimiterType<T>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          parse_function_([delim](std::string const &opt_value) {
              return parse_from_string<extract_value_type_t<T>>(opt_value,
                                                                delim);
          }) {
        set_default_value_help<T>();
    }
    Option<T> &default_value(const std::string &default_value)
        requires(!ParseFromStringContainerType<T>)
    {
        this->default_value_ = default_value;
        return *this;
    }
    Option<T> &default_value(std::initializer_list<std::string> default_value)
        requires ParseFromStringContainerType<T>
    {
        this->default_value_ = default_value;
        return *this;
    }

    T const &value() const { return bind_value_; }

   protected:
    bool is_option() const override final { return true; }
    bool is_positional() const override final { return false; }
    void parse(const std::string &opt_value) override {
        OptionBase::parse(opt_value);
        if constexpr (ParseFromStringContainerType<T>) {
            bind_value_.get().insert(bind_value_.get().end(),
                                     parse_function_(opt_value));
        } else {
            bind_value_.get() = parse_function_(opt_value);
        }
    }
    bool is_multiple() const override {
        if constexpr (ParseFromStringContainerType<T>) {
            return true;
        } else {
            return false;
        }
    }
    void use_default_if_needed() override {
        if (count() != 0) {
            return;
        }
        if constexpr (ParseFromStringContainerType<T>) {
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
        count_ = 0;
    }
    std::optional<std::string> get_default_value() const override {
        if constexpr (ParseFromStringContainerType<T>) {
            if (default_value_.has_value()) {
                return "{" + join(default_value_.value(), ',') + "}";
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
    std::function<extract_value_type_t<T>(std::string const &)> parse_function_;
    std::conditional_t<ParseFromStringContainerType<T>,
                       std::optional<std::vector<std::string>>,
                       std::optional<std::string>>
        default_value_;
};

template <BindableType T>
class Positional final : public OptionBase {
    friend class ArgParser;

   public:
    Positional(const std::string &name, const std::string &description,
               T &bind_value)
        requires BindableWithoutDelimiterType<T>
        : OptionBase(name, description), bind_value_(std::ref(bind_value)) {
        set_default_value_help<T>();
        if constexpr (ParseFromStringContainerType<T>) {
            parse_function_ = [](std::string const &opt_value) {
                return parse_from_string<extract_value_type_t<T>>(opt_value);
            };
        } else if constexpr (is_optional_v<T>) {
            parse_function_ = [](std::string const &opt_value) {
                return parse_from_string<extract_value_type_t<T>>(opt_value);
            };
        } else {
            parse_function_ = [](std::string const &opt_value) {
                return parse_from_string<T>(opt_value);
            };
        }
    }
    Positional(const std::string &name, const std::string &description,
               T &bind_value, char delim)
        requires BindableWithDelimiterType<T>
        : OptionBase(name, description), bind_value_(std::ref(bind_value)) {
        set_default_value_help<T>();
        if constexpr (ParseFromStringContainerType<T>) {
            parse_function_ = [delim](std::string const &opt_value) {
                return parse_from_string<extract_value_type_t<T>>(opt_value,
                                                                  delim);
            };
        } else if constexpr (is_optional_v<T>) {
            parse_function_ = [delim](std::string const &opt_value) {
                return parse_from_string<extract_value_type_t<T>>(opt_value,
                                                                  delim);
            };
        } else {
            parse_function_ = [delim](std::string const &opt_value) {
                return parse_from_string<T>(opt_value, delim);
            };
        }
    }
    Positional<T> &default_value(const std::string &default_value)
        requires(!ParseFromStringContainerType<T>)
    {
        this->default_value_ = default_value;
        return *this;
    }
    Positional<T> &default_value(
        std::initializer_list<std::string> default_value)
        requires ParseFromStringContainerType<T>
    {
        this->default_value_ = default_value;
        return *this;
    }

    T const &value() const { return bind_value_; }

   protected:
    bool is_option() const override final { return false; }
    bool is_positional() const override final { return true; }
    void parse(const std::string &opt_value) override {
        OptionBase::parse(opt_value);
        if constexpr (ParseFromStringContainerType<T>) {
            bind_value_.get().insert(bind_value_.get().end(),
                                     parse_function_(opt_value));
        } else {
            bind_value_.get() = parse_function_(opt_value);
        }
    }
    bool is_multiple() const override {
        if constexpr (ParseFromStringContainerType<T>) {
            return true;
        } else {
            return false;
        }
    }
    void use_default_if_needed() override {
        if (count() != 0) {
            return;
        }
        if constexpr (ParseFromStringContainerType<T>) {
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
        count_ = 0;
    }
    std::optional<std::string> get_default_value() const override {
        if constexpr (ParseFromStringContainerType<T>) {
            if (default_value_.has_value()) {
                return "{" + join(default_value_.value(), ',') + "}";
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
    std::function<extract_value_type_t<T>(std::string const &)> parse_function_;
    std::conditional_t<ParseFromStringContainerType<T>,
                       std::optional<std::vector<std::string>>,
                       std::optional<std::string>>
        default_value_;
};

class ArgParser {
   public:
    ArgParser(std::string prog, std::string description)
        : program{std::move(prog)}, description(std::move(description)) {}
    template <typename T>
        requires std::same_as<T, bool> || std::same_as<std::optional<bool>, T>
    Flag<T> &add_flag(
        const std::string &name, const std::string &description, T &bind_value,
        std::function<void(extract_value_type_t<T> &)> action = store_true) {
        auto flag = std::make_unique<Flag<T>>(name, description, bind_value,
                                              std::move(action));
        auto &ret = *(flag.get());
        if (option_exist(ret)) {
            throw std::runtime_error("Option already exists: " + name);
        }
        args.push_back(std::move(flag));
        return ret;
    }
    template <typename T>
        requires std::same_as<T, int> || std::same_as<std::optional<int>, T>
    Flag<T> &add_flag(const std::string &name, const std::string &description,
                      T &bind_value,
                      std::function<void(extract_value_type_t<T> &)> action =
                          increment<extract_value_type_t<T>>) {
        auto flag = std::make_unique<Flag<T>>(name, description, bind_value,
                                              std::move(action));
        auto &ret = *(flag.get());
        if (option_exist(ret)) {
            throw std::runtime_error("Option already exists: " + name);
        }
        args.push_back(std::move(flag));
        return ret;
    }

    template <BindableWithoutDelimiterType T>
    Option<T> &add_option(const std::string &name,
                          const std::string &description, T &bind_value) {
        auto option =
            std::make_unique<Option<T>>(name, description, bind_value);
        auto &ret = *(option.get());
        if (option_exist(ret)) {
            throw std::runtime_error("Option already exists: " + name);
        }
        args.push_back(std::move(option));
        return ret;
    }

    template <BindableWithDelimiterType T>
    Option<T> &add_option(const std::string &name,
                          const std::string &description, T &bind_value,
                          char delim) {
        auto option =
            std::make_unique<Option<T>>(name, description, bind_value, delim);
        auto &ret = *(option.get());
        if (option_exist(ret)) {
            throw std::runtime_error("Option already exists: " + name);
        }
        args.push_back(std::move(option));
        return ret;
    }

    template <BindableWithoutDelimiterType T>
    Positional<T> &add_positional(const std::string &name,
                                  const std::string &description,
                                  T &bind_value) {
        if (std::ranges::find_if(args, [](const auto &arg) {
                return arg->is_positional() &&
                       dynamic_cast<OptionBase *>(arg.get())->is_multiple();
            }) != args.end()) {
            throw std::runtime_error(
                "Positional argument only support one container and it "
                "must be the last one");
        }
        auto positional =
            std::make_unique<Positional<T>>(name, description, bind_value);
        auto &ret = *(positional.get());
        if (option_exist(ret)) {
            throw std::runtime_error("Option already exists: " + name);
        }
        args.push_back(std::move(positional));
        return ret;
    }

    template <BindableWithDelimiterType T>
    Positional<T> &add_positional(const std::string &name,
                                  const std::string &description, T &bind_value,
                                  char delim) {
        auto positional = std::make_unique<Positional<T>>(name, description,
                                                          bind_value, delim);
        auto &ret = *(positional.get());
        if (option_exist(ret)) {
            throw std::runtime_error("Option already exists: " + name);
        }
        args.push_back(std::move(positional));
        return ret;
    }

    void print_usage() const {
        std::stringstream usage_str;
        if (!description.empty()) {
            usage_str << "\n";
            usage_str << description;
            usage_str << "\n\n";
        }
        usage_str << "Usage: \n  " << program;
        if (std::ranges::find_if(args, [](const auto &arg) {
                return arg->is_option() || arg->is_flag();
            }) != args.end()) {
            usage_str << " [options]...";
        }
        auto positionals = args | std::views::filter([](const auto &arg) {
                               return arg->is_positional();
                           });
        for (const auto &arg : positionals) {
            if (dynamic_cast<OptionBase *>(arg.get())->is_multiple()) {
                usage_str << " <" << arg->long_opt_names_.front() << ">...";
            } else {
                usage_str << " <" << arg->long_opt_names_.front() << ">";
            }
        }
        if (std::ranges::find_if(args, [](const auto &arg) {
                return arg->is_option() || arg->is_flag();
            }) != args.end()) {
            usage_str << "\n\nOptions:\n";
        }
        for (const auto &arg : args) {
            if (arg->is_option() || arg->is_flag()) {
                usage_str << " " << arg->usage(32) << '\n';
            }
        }

        if (std::ranges::find_if(args, [](const auto &arg) {
                return arg->is_positional();
            }) != args.end()) {
            usage_str << "\n\nPositionals:\n";
        }
        for (const auto &arg : args) {
            if (arg->is_positional()) {
                usage_str << " " << arg->usage(32) << '\n';
            }
        }
        std::cout << usage_str.str() << std::endl;
    }
    void print_version() const {
        std::cout << "Version: " << version << std::endl;
    }
    ArgBase *get(const std::string &name) {
        if (name.length() == 1) {
            auto it = std::ranges::find_if(args, [name](const auto &arg) {
                return std::find(arg->short_opt_names_.begin(),
                                 arg->short_opt_names_.end(),
                                 name) != arg->short_opt_names_.end();
            });
            return it != args.end() ? it->get() : nullptr;
        } else {
            auto it = std::ranges::find_if(args, [name](const auto &arg) {
                return std::find(arg->long_opt_names_.begin(),
                                 arg->long_opt_names_.end(),
                                 name) != arg->long_opt_names_.end();
            });
            return it != args.end() ? it->get() : nullptr;
        }
    }
    ArgBase &operator[](const std::string &name) {
        if (auto *arg = get(name); arg != nullptr) {
            return *arg;
        } else {
            throw std::runtime_error("Unknown option: " + name);
        }
    }
    void parse(int argc, const char *argv[]) {
        std::vector<const char *> commands{argv, argv + argc};
        size_t i = 1;  // Skip program name
        if (!commands.empty() && commands[0] != nullptr &&
            commands[0][0] == '-') {
            i = 0;
        }
        std::vector<ArgBase *> positionals;

        // Collect all positional arguments
        for (const auto &arg : args) {
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

                if (auto *option = get(name)) {
                    if (option->is_flag()) {
                        auto *flag = dynamic_cast<FlagBase *>(option);
                        flag->parse();
                    } else if (option->is_option()) {
                        auto *opt = dynamic_cast<OptionBase *>(option);
                        if (eq_pos != std::string::npos) {
                            opt->parse(value);
                        } else if (i + 1 < commands.size()) {
                            opt->parse(commands[++i]);
                        } else {
                            throw std::runtime_error(
                                "Missing value for option: " + name);
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
                            flag->parse();
                        } else if (option->is_option()) {
                            auto *opt = dynamic_cast<OptionBase *>(option);
                            if (j < opts.size() - 1) {
                                // If not the last character, use the rest as
                                // value
                                opt->parse(opts.substr(j + 1));
                                break;
                            } else if (i + 1 < commands.size()) {
                                opt->parse(commands[++i]);
                            } else {
                                throw std::runtime_error(
                                    "Missing value for option: " + name);
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
                    auto *pos =
                        dynamic_cast<OptionBase *>(positionals[pos_index]);
                    pos->parse(arg);
                    if (!pos->is_multiple()) {
                        pos_index++;
                    }
                } else {
                    throw std::runtime_error("Too many positional arguments");
                }
            }
            ++i;
        }

        while (i < commands.size()) {
            if (pos_index < positionals.size()) {
                auto *pos = dynamic_cast<OptionBase *>(positionals[pos_index]);
                pos->parse(commands[i]);
                if (!pos->is_multiple()) {
                    pos_index++;
                }
            } else {
                throw std::runtime_error("Too many positional arguments");
            }
            ++i;
        }

        // Handle options that were not provided but have default values
        for (const auto &arg : args) {
            if ((arg->is_option() || arg->is_positional()) &&
                arg->count() == 0) {
                dynamic_cast<OptionBase *>(arg.get())->use_default_if_needed();
            }
        }
    }

   private:
    bool option_exist(ArgBase &new_arg) const {
        for (const auto &arg : args) {
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
    std::vector<std::unique_ptr<ArgBase>> args;
    std::string version{"0.1"};
    std::string program;
    std::string description;
};

}  // namespace argparse

#endif  // ARG_PASER_HPP
