//
// Created by shediao on 24-12-29.
//

#ifndef ARG_PASER_HPP
#define ARG_PASER_HPP

#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
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

template <>
struct is_string<std::string> : std::true_type {};
template <>
struct is_string<std::wstring> : std::true_type {};
template <>
struct is_string<std::u8string> : std::true_type {};
template <>
struct is_string<std::u16string> : std::true_type {};
template <>
struct is_string<std::u32string> : std::true_type {};

template <typename T>
constexpr bool is_string_v = is_string<T>::value;

template <typename T>
concept is_tuple_like = requires(T t) {
    typename std::tuple_element<0, T>::type;
    requires std::tuple_size_v<T> > 0;
    std::get<0>(std::declval<T &>());
};

template <typename T>
concept is_parse_from_string_basic_type =
    std::same_as<T, std::string> || std::same_as<T, bool> ||
    std::same_as<T, int> || std::same_as<T, long> ||
    std::same_as<T, unsigned long> || std::same_as<T, long long> ||
    std::same_as<T, unsigned long long> || std::same_as<T, float> ||
    std::same_as<T, double> || std::same_as<T, long double> ||
    requires(std::string const &s) { T(s); };

template <typename T>
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

template <typename T>
    requires is_tuple_like<T>
T make_tuple_from_container(std::vector<std::string> const &v) {
    return make_tuple_from_container_impl<T>(
        v, std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>());
}

// for tuple like types
template <typename T>
    requires is_tuple_like<T>
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

template <typename T>
concept is_tuple_like_parse_from_split_string = requires(T t) {
    parse_from_string<T>(std::declval<std::string>(), std::declval<char>());
};
template <typename T>
concept can_parse_from_string = is_parse_from_string_basic_type<T> ||
                                is_tuple_like_parse_from_split_string<T>;

template <typename T>
concept is_container = requires(T t) {
    t.insert(t.end(), std::declval<typename T::value_type>());
    requires can_parse_from_string<typename T::value_type>;
};

template <typename T>
    requires(!is_container<T>)
void replace_or_append_new_value(T &value, T new_value) {
    value = std::move(new_value);
}

template <typename T>
    requires is_container<T>
void replace_or_append_new_value(T &value, T new_value) {
    value.insert(value.end(), std::make_move_iterator(new_value.begin()),
                 std::make_move_iterator(new_value.end()));
}

template <typename T>
struct action_value_type {
    using type = T;
};
template <typename T>
struct action_value_type<std::optional<T>> {
    using type = T;
};
template <typename T>
using action_value_type_t = typename action_value_type<T>::type;

template <typename T>
struct flag_action_function {
    using type = std::function<void(T &)>;
};
template <typename T>
struct flag_action_function<std::optional<T>> {
    using type = std::function<void(T &)>;
};

template <typename T>
using flag_action_function_t = typename flag_action_function<T>::type;

template <typename T>
struct option_action_function {
    using type = std::function<void(T &, std::string const &)>;
};
template <typename T>
struct option_action_function<std::optional<T>> {
    using type = std::function<void(T &, std::string const &)>;
};

template <typename T>
using option_action_function_t = typename option_action_function<T>::type;
template <typename T>
using positional_action_function_t = typename option_action_function<T>::type;
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

template <typename T>
    requires is_parse_from_string_basic_type<T>
inline void replace_value(T &value, const std::string &opt_value) {
    value = parse_from_string<T>(opt_value);
}

template <typename T>
    requires is_tuple_like_parse_from_split_string<T>
inline void replace_value(T &value, const std::string &opt_value, char delim) {
    value = parse_from_string<T>(opt_value, delim);
}

template <typename T>
    requires is_container<T> &&
             is_parse_from_string_basic_type<typename T::value_type>
inline void append_value(T &value, const std::string &opt_value) {
    value.insert(value.end(),
                 parse_from_string<typename T::value_type>(opt_value));
}

template <typename T>
    requires is_container<T> &&
             is_tuple_like_parse_from_split_string<typename T::value_type>
inline void append_value(T &value, const std::string &opt_value, char delim) {
    value.insert(value.end(),
                 parse_from_string<typename T::value_type>(opt_value, delim));
}

class ArgBase {
    friend class ArgParser;

   public:
    ArgBase(const std::string &name, const std::string &description)
        : description_(description) {
        for (auto &&opt_name : split(name, ',', -1)) {
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
    virtual std::string usage(int option_width) const = 0;
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

class FlagBase : public ArgBase {
    friend class ArgParser;

   public:
    FlagBase(const std::string &name, const std::string &description)
        : ArgBase(name, description) {}
    bool is_flag() const override final { return true; }
    bool is_option() const override final { return false; }
    bool is_positional() const override final { return false; }
    virtual void parse() = 0;

   protected:
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
        usage_str << std::format("{0:<{1}}{2}", options_str, option_width,
                                 get_description());
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
          action_{store_true} {}
    Flag(const std::string &name, const std::string &description, T &bind_value,
         flag_action_function_t<T> action)
        : FlagBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_(std::move(action)) {}
    void parse() override {
        if constexpr (is_optional_v<T>) {
            auto &optional_value = bind_value_.get();
            if (!optional_value.has_value()) {
                optional_value = typename T::value_type{};
            }
            action_(optional_value.value());
        } else {
            action_(bind_value_.get());
        }
        count_++;
    }
    T const &value() const { return bind_value_; }

   private:
    std::reference_wrapper<T> bind_value_;
    flag_action_function_t<T> action_;
};

class OptionBase : public ArgBase {
    friend class ArgParser;

   public:
    OptionBase(const std::string &name, const std::string &description)
        : ArgBase(name, description) {}
    bool is_flag() const override final { return false; }
    void set_value_help(const std::string &value_help) {
        this->value_help = value_help;
    }
    virtual void parse(const std::string &opt_value) {
        this->opt_values.push_back(opt_value);
        count_++;
    }

    virtual bool is_multiple() const = 0;
    virtual void use_default_if_needed() = 0;

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
    std::string usage(int option_width) const override {
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
                options_str += " ...";
            }
            usage_str << std::format("{0:<{1}}{2}", options_str, option_width,
                                     get_description());
        } else {
            std::string options_str{long_opt_names_[0]};
            if (is_multiple()) {
                options_str += " ...";
            }
            usage_str << std::format("{0:<{1}}{2}", options_str, option_width,
                                     get_description());
        }
        return usage_str.str();
    }
    std::string value_help;
    std::vector<std::string> opt_values;
};

template <typename T>
    requires can_parse_from_string<T> ||
             (is_optional_v<T> &&
              can_parse_from_string<typename T::value_type>) ||
             is_container<T>
class Option final : public OptionBase {
    friend class ArgParser;

   public:
    Option(const std::string &name, const std::string &description,
           T &bind_value)
        requires is_parse_from_string_basic_type<T> ||
                 (is_optional_v<T> &&
                  is_parse_from_string_basic_type<typename T::value_type>)
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_(replace_value<action_value_type_t<T>>) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value, char delim)
        requires is_tuple_like_parse_from_split_string<T> ||
                 (is_optional_v<T> &&
                  is_tuple_like_parse_from_split_string<typename T::value_type>)
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([delim](action_value_type_t<T> &value,
                          const std::string &opt_value) {
              replace_value<action_value_type_t<T>>(value, opt_value, delim);
          }) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value)
        requires is_container<T> &&
                     is_parse_from_string_basic_type<typename T::value_type>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value, char delim)
        requires is_container<T> && is_tuple_like_parse_from_split_string<
                                        typename T::value_type>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([delim](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value, delim);
          }) {
        set_default_value_help<T>();
    }
    bool is_option() const override final { return true; }
    bool is_positional() const override final { return false; }
    void parse(const std::string &opt_value) override {
        OptionBase::parse(opt_value);
        if constexpr (is_optional_v<T>) {
            auto &optional_value = bind_value_.get();
            if (!optional_value.has_value()) {
                optional_value = typename T::value_type{};
            }
            action_(optional_value.value(), opt_value);
        } else {
            action_(bind_value_.get(), opt_value);
        }
    }
    bool is_multiple() const override {
        if constexpr (is_container<T>) {
            return true;
        } else {
            return false;
        }
    }
    void use_default_if_needed() override {
        if (count() != 0) {
            return;
        }
        if constexpr (is_container<T>) {
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
    }

    void set_default(const std::string &default_value)
        requires(!is_container<T>)
    {
        this->default_value_ = default_value;
    }
    void set_default(std::initializer_list<std::string> default_value)
        requires is_container<T>
    {
        this->default_value_ = default_value;
    }

    T const &value() const { return bind_value_; }

   private:
    std::reference_wrapper<T> bind_value_;
    option_action_function_t<T> action_;
    std::conditional_t<is_container<T>, std::optional<std::vector<std::string>>,
                       std::optional<std::string>>
        default_value_;
};

template <typename T>
    requires can_parse_from_string<T> ||
             (is_optional_v<T> &&
              can_parse_from_string<typename T::value_type>) ||
             is_container<T>
class Positional final : public OptionBase {
    friend class ArgParser;

   public:
    Positional(const std::string &name, const std::string &description,
               T &bind_value)
        requires is_parse_from_string_basic_type<T> ||
                 (is_optional_v<T> &&
                  is_parse_from_string_basic_type<typename T::value_type>)
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_(replace_value<action_value_type_t<T>>) {
        set_default_value_help<T>();
    }
    Positional(const std::string &name, const std::string &description,
               T &bind_value, char delim)
        requires is_tuple_like_parse_from_split_string<T> ||
                 (is_optional_v<T> &&
                  is_tuple_like_parse_from_split_string<typename T::value_type>)
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([delim](action_value_type_t<T> &value,
                          const std::string &opt_value) {
              replace_value<action_value_type_t<T>>(value, opt_value, delim);
          }) {
        set_default_value_help<T>();
    }
    Positional(const std::string &name, const std::string &description,
               T &bind_value)
        requires is_container<T> &&
                     is_parse_from_string_basic_type<typename T::value_type>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    Positional(const std::string &name, const std::string &description,
               T &bind_value, char delim)
        requires is_container<T> && is_tuple_like_parse_from_split_string<
                                        typename T::value_type>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([delim](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value, delim);
          }) {
        set_default_value_help<T>();
    }
    bool is_option() const override final { return false; }
    bool is_positional() const override final { return true; }
    void parse(const std::string &opt_value) override {
        OptionBase::parse(opt_value);
        if constexpr (is_optional_v<T>) {
            auto &optional_value = bind_value_.get();
            if (!optional_value.has_value()) {
                optional_value = typename T::value_type{};
            }
            action_(optional_value.value(), opt_value);
        } else {
            action_(bind_value_.get(), opt_value);
        }
    }
    bool is_multiple() const override {
        if constexpr (is_container<T>) {
            return true;
        } else {
            return false;
        }
    }
    void use_default_if_needed() override {
        if (count() != 0) {
            return;
        }
        if constexpr (is_container<T>) {
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
    }
    void set_default(const std::string &default_value)
        requires(!is_container<T>)
    {
        this->default_value_ = default_value;
    }
    void set_default(std::initializer_list<std::string> default_value)
        requires is_container<T>
    {
        this->default_value_ = default_value;
    }

    T const &value() const { return bind_value_; }

   private:
    std::reference_wrapper<T> bind_value_;
    positional_action_function_t<T> action_;
    std::conditional_t<is_container<T>, std::optional<std::vector<std::string>>,
                       std::optional<std::string>>
        default_value_;
};

class ArgParser {
   public:
    ArgParser(std::string prog, std::string description)
        : program{std::move(prog)}, description(std::move(description)) {}
    Flag<bool> &add_flag(const std::string &name,
                         const std::string &description, bool &bind_value,
                         flag_action_function_t<bool> action = store_true) {
        args.push_back(std::make_unique<Flag<bool>>(
            name, description, bind_value, std::move(action)));
        return *dynamic_cast<Flag<bool> *>(args.back().get());
    }
    Flag<int> &add_flag(const std::string &name, const std::string &description,
                        int &bind_value,
                        flag_action_function_t<int> action = increment<int>) {
        args.push_back(std::make_unique<Flag<int>>(
            name, description, bind_value, std::move(action)));
        return *dynamic_cast<Flag<int> *>(args.back().get());
    }

    template <typename T>
        requires can_parse_from_string<T> ||
                 (is_optional_v<T> &&
                  can_parse_from_string<typename T::value_type>) ||
                 (is_container<T> &&
                  can_parse_from_string<typename T::value_type>)
    Option<T> &add_option(const std::string &name,
                          const std::string &description, T &bind_value) {
        args.push_back(
            std::make_unique<Option<T>>(name, description, bind_value));
        return *dynamic_cast<Option<T> *>(args.back().get());
    }

    template <typename T>
        requires is_tuple_like_parse_from_split_string<T> ||
                 (is_optional_v<T> &&
                  is_tuple_like_parse_from_split_string<typename T::value_type>) ||
                 (is_container<T> &&
                  is_tuple_like_parse_from_split_string<typename T::value_type>)
    Option<T> &add_option(const std::string &name,
                          const std::string &description, T &bind_value,
                          char delim) {
        args.push_back(
            std::make_unique<Option<T>>(name, description, bind_value, delim));
        return *dynamic_cast<Option<T> *>(args.back().get());
    }

    template <typename T>
        requires can_parse_from_string<T> ||
                 (is_optional_v<T> &&
                  can_parse_from_string<typename T::value_type>) ||
                 (is_container<T> &&
                  can_parse_from_string<typename T::value_type>)
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
        args.push_back(
            std::make_unique<Positional<T>>(name, description, bind_value));
        return *dynamic_cast<Positional<T> *>(args.back().get());
    }

    template <typename T>
        requires is_tuple_like_parse_from_split_string<T> ||
                 (is_optional_v<T> &&
                  is_tuple_like_parse_from_split_string<typename T::value_type>) ||
                 (is_container<T> &&
                  is_tuple_like_parse_from_split_string<typename T::value_type>)
    Positional<T> &add_positional(const std::string &name,
                                  const std::string &description, T &bind_value,
                                  char delim) {
        args.push_back(std::make_unique<Positional<T>>(name, description,
                                                       bind_value, delim));
        return *dynamic_cast<Positional<T> *>(args.back().get());
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

        // Check if all required arguments are provided
        for (const auto &arg : args) {
            if (arg->required_ && arg->count() == 0) {
                throw std::runtime_error(
                    "Required argument missing: " +
                    (!arg->long_opt_names_.empty()    ? arg->long_opt_names_[0]
                     : !arg->short_opt_names_.empty() ? arg->short_opt_names_[0]
                                                      : ""));
            }
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
    std::vector<std::unique_ptr<ArgBase>> args;
    std::string version{"0.1"};
    std::string program;
    std::string description;
};

}  // namespace argparse

#endif  // ARG_PASER_HPP
