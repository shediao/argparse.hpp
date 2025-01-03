//
// Created by shediao on 24-12-29.
//

#ifndef ARG_PASER_HPP
#define ARG_PASER_HPP

#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace arg::parser {

namespace detail {

template <typename T>
concept IsTupleLike = requires(T t) {
    typename std::tuple_element<0, T>::type;
    requires std::tuple_size<T>::value > 0;
    std::get<0>(std::declval<T &>());
};

template <typename T>
T ParseFromString(std::string const &s);

template <>
char ParseFromString(std::string const &s) = delete;
template <>
wchar_t ParseFromString(std::string const &s) = delete;

template <typename T>
    requires std::constructible_from<T, std::string>
T ParseFromString(std::string const &s) {
    return T(s);
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

inline std::vector<std::string> Split(std::string const &s, char delim,
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

template <>
inline std::string ParseFromString<std::string>(std::string const &s) {
    return s;
}
// bool 特化
template <>
inline bool ParseFromString<bool>(std::string const &s) {
    if (s == "true" || s == "on" || s == "1") {
        return true;
    }
    if (s == "false" || s == "off" || s == "0") {
        return false;
    }
    throw std::invalid_argument("Invalid string for bool: " + s);
}

#define ARGPARSER_TYPE_FROM_STRING(type, std_stox)                             \
    template <>                                                                \
    inline type ParseFromString<type>(std::string const &s) {                  \
        try {                                                                  \
            size_t pos = 0;                                                    \
            const type result{std_stox(s, &pos)};                              \
            if (pos != s.size()) {                                             \
                throw std::invalid_argument("Invalid string for " #type ": " + \
                                            s);                                \
            }                                                                  \
            return result;                                                     \
        } catch (const std::invalid_argument &e) {                             \
            throw std::invalid_argument("Invalid string for " #type ": " + s); \
        } catch (const std::out_of_range &e) {                                 \
            throw std::out_of_range("Out of range for " #type ": " + s);       \
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
T MakeTupleFromContainerImpl(std::vector<std::string> const &v,
                             std::integer_sequence<std::size_t, I...>) {
    return T{
        ParseFromString<std::decay_t<std::tuple_element_t<I, T>>>(v[I])...};
}

template <typename T>
    requires IsTupleLike<T>
T MakeTupleFromContainer(std::vector<std::string> const &v) {
    return MakeTupleFromContainerImpl<T>(
        v, std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>());
}

// for tuple like types
template <typename T>
    requires IsTupleLike<T>
T ParseFromString(std::string const &s, const char delim) {
    auto v = detail::Split(s, delim, std::tuple_size_v<std::decay_t<T>>);
    if (v.size() != std::tuple_size_v<std::decay_t<T>>) {
        throw std::invalid_argument(
            "Invalid string for Split " +
            std::to_string(std::tuple_size_v<std::decay_t<T>>) +
            "th element:" + s);
    }
    return MakeTupleFromContainer<T>(v);
}

template <typename T>
concept CanParseFromStringWithoutSplit =
    requires(T t) { ParseFromString<T>(std::declval<std::string>()); };

template <typename T>
concept CanParseFromStringSplitOnece = requires(T t) {
    ParseFromString<T>(std::declval<std::string>(), std::declval<char>());
};
template <typename T>
concept CanParseFromString =
    CanParseFromStringWithoutSplit<T> || CanParseFromStringSplitOnece<T>;

template <typename T>
concept IsContainer = requires(T t) {
    t.insert(t.end(), std::declval<typename T::value_type>());
    requires CanParseFromString<typename T::value_type>;
    requires std::same_as<std::basic_string<typename T::value_type>, T>;
};

inline std::pair<std::string, std::string> ParseOptionNames(
    const std::string &name) {
    auto names = detail::Split(name, ',', -1);
    if (names.size() == 2) {
        const auto long_name = names[0].length() > 1
                                   ? (names[0])
                                   : (names[1].length() > 1 ? names[1] : "");
        const auto short_name = names[0].length() == 1
                                    ? (names[0])
                                    : (names[1].length() == 1 ? names[1] : "");
        if (long_name.empty() || short_name.empty()) {
            throw std::invalid_argument("Invalid string for " + name);
        }
        return {short_name, long_name};
    }
    if (names.size() == 1) {
        if (names[0].length() > 1) {
            return std::make_pair("", names[0]);
        } else {
            return std::make_pair(names[0], "");
        }
    }
    throw std::invalid_argument("Invalid option name: " + name);
}

template <typename T>
    requires(!IsContainer<T>)
void replace_or_append_new_value(T &value, T new_value) {
    value = std::move(new_value);
}

template <typename T>
    requires IsContainer<T>
void replace_or_append_new_value(T &value, T new_value) {
    value.insert(value.end(), std::make_move_iterator(new_value.begin()),
                 std::make_move_iterator(new_value.end()));
}
}  // namespace detail

namespace action {
void store_true(bool &value) { value = true; }

void store_false(bool &value) { value = false; }

template <typename T>
    requires std::integral<T>
void increment(T &value) {
    value++;
}

template <typename T>
    requires std::integral<T>
void decrement(T &value) {
    value--;
}

template <typename T>
    requires detail::CanParseFromString<T>
void replace_value(T &value, const std::string &opt_value) {
    value = detail::ParseFromString<T>(opt_value);
}

template <typename T>
    requires detail::IsContainer<T>
void append_value(T &value, const std::string &opt_value) {
    value.insert(value.end(), detail::ParseFromString<T>(opt_value));
}
}  // namespace action

class ArgBase {
    friend class ArgParser;

   public:
    ArgBase(const std::string &name, const std::string &description)
        : description_(description) {
        for (auto &&opt_name : detail::Split(name, ',', -1)) {
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
    void require(bool required = true) { this->required_ = required; }
    const std::string &get_description() const { return description_; }
    virtual std::string usage(int option_width, char padding_char) const = 0;
    virtual ~ArgBase() = default;
    virtual bool is_flag() const = 0;
    virtual bool is_option() const = 0;
    virtual bool is_positional() const = 0;

   protected:
    size_t count_{0};
    bool required_{false};
    std::vector<std::string> short_opt_names_;
    std::vector<std::string> long_opt_names_;
    std::string description_;
};

template <typename... T>
struct overload : T... {
    using T::operator()...;
};

template <typename... T>
overload(T...) -> overload<T...>;

template <typename T = bool>
    requires(std::integral<T> && detail::CanParseFromString<T>) ||
            std::is_same_v<T, bool>
class Flag : public ArgBase {
    friend class ArgParser;

   public:
    Flag(const std::string &name, const std::string &description)
        : ArgBase(name, description), action_{action::store_true} {}
    Flag(const std::string &name, const std::string &description,
         std::function<void(T &)> action)
        : ArgBase(name, description), action_(action) {}
    bool is_flag() const override { return true; }
    bool is_option() const override { return false; }
    bool is_positional() const override { return false; }
    void parse() {
        std::visit(overload{
                       [this](T &value) { action_(value); },
                       [this](std::reference_wrapper<T> &value) {
                           action_(value.get());
                       },
                   },
                   value_);
        count_++;
    }
    void set_default(T default_value) { value_ = default_value; }
    T value() const {
        return std::visit(
            overload{
                [](T &value) { return value; },
                [](std::reference_wrapper<T> &value) { return value.get(); },
            },
            value_);
    }

   protected:
    std::string usage(int option_width, char padding_char) const override {
        std::stringstream usage_str;
        std::string options_str{};
        std::vector<std::string> short_opts;
        std::vector<std::string> long_opts;
        std::transform(begin(short_opt_names_), end(short_opt_names_),
                       back_inserter(short_opts),
                       [](auto const &s) { return "-" + s; });
        std::transform(begin(long_opt_names_), end(long_opt_names_),
                       back_inserter(long_opts),
                       [](auto const &s) { return "--" + s; });
        if (!short_opts.empty()) {
            options_str += detail::join(short_opts, ',');
            if (!long_opts.empty()) {
                options_str += ", ";
            }
        } else {
            options_str += "    ";
        }
        if (!long_opts.empty()) {
            options_str += detail::join(long_opts, ',');
        }
        usage_str << std::format("{0:<{1}}{2}", options_str, option_width,
                                 get_description());
        return usage_str.str();
    }

   private:
    std::variant<T, std::reference_wrapper<T>> value_;
    std::function<void(T &)> action_;
};

class OptionBase : public ArgBase {
    friend class ArgParser;

   public:
    OptionBase(const std::string &name, const std::string &description)
        : ArgBase(name, description) {}
    bool is_flag() const override { return false; }
    void set_default(const std::string &default_value_str) {
        this->default_value_str = default_value_str;
    }
    void set_value_help(const std::string &value_help) {
        this->value_help = value_help;
    }
    template <typename T>
        requires detail::IsContainer<T>
    T as() const {
        T result;
        if (opt_values.empty()) {
            result.insert(result.end(),
                          detail::ParseFromString<T>(default_value_str));
        } else {
            std::transform(opt_values.begin(), opt_values.end(),
                           std::back_inserter(result),
                           [](const std::string &opt_value) {
                               return detail::ParseFromString<T>(opt_value);
                           });
        }
        return result;
    }
    template <typename T>
        requires detail::IsContainer<T> &&
                 detail::CanParseFromStringSplitOnece<T>
    T as(char delim) const {
        T result;
        if (opt_values.empty()) {
            result.insert(result.end(),
                          detail::ParseFromString<T>(default_value_str, delim));
        } else {
            std::transform(opt_values.begin(), opt_values.end(),
                           std::back_inserter(result),
                           [delim](const std::string &opt_value) {
                               return detail::ParseFromString<T>(opt_value,
                                                                 delim);
                           });
        }
        return result;
    }
    template <typename T>
        requires(!detail::IsContainer<T>)
    T as() const {
        if (opt_values.empty()) {
            return detail::ParseFromString<T>(default_value_str);
        } else {
            return detail::ParseFromString<T>(opt_values.back());
        }
    }
    template <typename T>
        requires(!detail::IsContainer<T>) &&
                detail::CanParseFromStringSplitOnece<T>
    T as(char delim) const {
        if (opt_values.empty()) {
            return detail::ParseFromString<T>(default_value_str, delim);
        } else {
            return detail::ParseFromString<T>(opt_values.back(), delim);
        }
    }
    virtual void parse(const std::string &opt_value) {
        this->opt_values.push_back(opt_value);
        count_++;
    }

   protected:
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
    std::string usage(int option_width, char padding_char) const override {
        std::stringstream usage_str;
        if (is_option()) {
            std::string options_str{};
            std::vector<std::string> short_opts;
            std::vector<std::string> long_opts;
            std::transform(begin(short_opt_names_), end(short_opt_names_),
                           back_inserter(short_opts),
                           [](auto const &s) { return "-" + s; });
            std::transform(begin(long_opt_names_), end(long_opt_names_),
                           back_inserter(long_opts),
                           [](auto const &s) { return "--" + s; });
            if (!short_opts.empty()) {
                options_str += detail::join(short_opts, ',');
                if (!long_opts.empty()) {
                    options_str += ", ";
                }
            } else {
                options_str += "    ";
            }
            if (!long_opts.empty()) {
                options_str += detail::join(long_opts, ',');
            }
            options_str += (" " + value_help);
            usage_str << std::format("{0:<{1}}{2}", options_str, option_width,
                                     get_description());
        } else {
            usage_str << std::format("{0:<{1}}{2}", long_opt_names_[0],
                                     option_width, get_description());
        }
        return usage_str.str();
    }
    std::string value_help;
    std::string default_value_str;
    std::vector<std::string> opt_values;
};

template <typename T>
    requires detail::CanParseFromString<T> || detail::IsContainer<T>
class Option : public OptionBase {
    friend class ArgParser;

   public:
    Option(const std::string &name, const std::string &description,
           T &bind_value)
        requires detail::CanParseFromString<T>
        : OptionBase(name, description),
          value_(bind_value),
          action_([](T &value, const std::string &opt_value) {
              action::replace_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value)
        requires detail::IsContainer<T>
        : OptionBase(name, description),
          value_(bind_value),
          action_([](T &value, const std::string &opt_value) {
              action::append_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    bool is_option() const override { return true; }
    bool is_positional() const override { return false; }
    void parse(const std::string &opt_value) override {
        OptionBase::parse(opt_value);
        std::visit(
            overload{
                [this, &opt_value](T &value) { action_(value, opt_value); },
                [this, &opt_value](std::reference_wrapper<T> &value) {
                    action_(value.get(), opt_value);
                },
            },
            value_);
    }

    T const &value() const {
        return std::visit(
            overload{
                [](T &value) { return value; },
                [](std::reference_wrapper<T> &value) { return value.get(); },
            },
            value_);
    }

   private:
    std::variant<T, std::reference_wrapper<T>> value_;
    std::function<void(T &, const std::string &)> action_;
};

template <typename T>
    requires detail::CanParseFromString<T> || detail::IsContainer<T>
class Positional : public OptionBase {
    friend class ArgParser;

   public:
    Positional(
        const std::string &name, const std::string &description, T &bind_value,
        std::function<void(T &, const std::string &)> action =
            [](T &value, const std::string &opt_value) {
                value = detail::ParseFromString<T>(opt_value);
            })
        requires detail::CanParseFromString<T>
        : OptionBase(name, description),
          bind_value_(bind_value),
          action_(action) {
        set_default_value_help<T>();
    }
    Positional(
        const std::string &name, const std::string &description, T &bind_value,
        std::function<void(T &, const std::string &)> action =
            [](T &value, const std::string &opt_value) {
                action::append_value<T>(value, opt_value);
            })
        requires(detail::IsContainer<T>)
        : OptionBase(name, description),
          bind_value_(bind_value),
          action_(action) {
        set_default_value_help<T>();
    }
    bool is_option() const override { return false; }
    bool is_positional() const override { return true; }
    void parse(const std::string &opt_value) override {
        OptionBase::parse(opt_value);
        std::visit(
            overload{
                [this, &opt_value](T &value) { action_(value, opt_value); },
                [this, &opt_value](std::reference_wrapper<T> &value) {
                    action_(value.get(), opt_value);
                },
            },
            bind_value_);
    }

    T const &value() const {
        return std::visit(
            overload{
                [](T &value) { return value; },
                [](std::reference_wrapper<T> &value) { return value.get(); },
            },
            bind_value_);
    }

   private:
    std::variant<T, std::reference_wrapper<T>> bind_value_;
    std::function<void(T &, const std::string &)> action_;
};

class ArgParser {
   public:
    ArgParser(int argc, const char *argv[])
        : commands(argc > 0 ? argv : nullptr,
                   argc > 0 ? argv + argc : nullptr) {}
    template <typename T = bool>
        requires std::integral<T> || std::is_same_v<T, bool>
    void add_flag(const std::string &name, const std::string &description) {
        args.push_back(std::make_unique<Flag<T>>(name, description));
    }

    template <typename T>
        requires detail::CanParseFromStringWithoutSplit<T>
    void add_option(const std::string &name, const std::string &description,
                    T &bind_value) {
        args.push_back(
            std::make_unique<Option<T>>(name, description, bind_value));
    }

    template <typename T>
        requires detail::CanParseFromStringSplitOnece<T>
    void add_option(const std::string &name, const std::string &description,
                    T &bind_value, char delim) {
        args.push_back(
            std::make_unique<Option<T>>(name, description, bind_value, delim));
    }

    template <typename T>
        requires detail::CanParseFromStringWithoutSplit<T>
    void add_positional(const std::string &name, const std::string &description,
                        T &bind_value) {
        args.push_back(
            std::make_unique<Positional<T>>(name, description, bind_value));
    }

    template <typename T>
        requires detail::CanParseFromStringSplitOnece<T>
    void add_positional(const std::string &name, const std::string &description,
                        T &bind_value, char delim) {
        args.push_back(std::make_unique<Positional<T>>(name, description,
                                                       bind_value, delim));
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
                usage_str << " " << arg->usage(32, '.') << '\n';
            }
        }

        if (std::ranges::find_if(args, [](const auto &arg) {
                return arg->is_positional();
            }) != args.end()) {
            usage_str << "\n\nPositionals:\n";
        }
        for (const auto &arg : args) {
            if (arg->is_positional()) {
                usage_str << arg->usage(32, '.') << '\n';
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
