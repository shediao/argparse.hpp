//
// Created by shediao on 24-12-29.
//

#ifndef ARG_PASER_HPP
#define ARG_PASER_HPP

#include <cstddef>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace argparse {

namespace {

template <typename T>
concept is_tuple_like = requires(T t) {
    typename std::tuple_element<0, T>::type;
    requires std::tuple_size_v<T> > 0;
    std::get<0>(std::declval<T &>());
};

template <typename T>
concept can_parse_from_string_without_split =
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
concept can_parse_from_string_split_once = requires(T t) {
    parse_from_string<T>(std::declval<std::string>(), std::declval<char>());
};
template <typename T>
concept can_parse_from_string = can_parse_from_string_without_split<T> ||
                                can_parse_from_string_split_once<T>;

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
    requires can_parse_from_string_without_split<T>
inline void replace_value(T &value, const std::string &opt_value) {
    value = parse_from_string<T>(opt_value);
}

template <typename T>
    requires can_parse_from_string_split_once<T>
inline void replace_value(T &value, const std::string &opt_value, char delim) {
    value = parse_from_string<T>(opt_value, delim);
}

template <typename T>
    requires is_container<T> &&
             can_parse_from_string_without_split<typename T::value_type>
inline void append_value(T &value, const std::string &opt_value) {
    value.insert(value.end(),
                 parse_from_string<typename T::value_type>(opt_value));
}

template <typename T>
    requires is_container<T> &&
             can_parse_from_string_split_once<typename T::value_type>
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

class FlagBase : public ArgBase {
    friend class ArgParser;

   public:
    FlagBase(const std::string &name, const std::string &description)
        : ArgBase(name, description) {}
    bool is_flag() const override { return true; }
    bool is_option() const override { return false; }
    bool is_positional() const override { return false; }
    virtual void parse() = 0;

   protected:
    std::string usage(int option_width, char padding_char) const override {
        std::stringstream usage_str;
        std::string options_str{};
        std::vector<std::string> short_opts;
        std::vector<std::string> long_opts;
        std::ranges::transform(short_opt_names_,
                       back_inserter(short_opts),
                       [](auto const &s) { return "-" + s; });
        std::ranges::transform(long_opt_names_,
                       back_inserter(long_opts),
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

template <typename... T>
overload(T...) -> overload<T...>;

template <typename T = bool>
    requires std::integral<T> || std::is_same_v<T, bool>
class Flag : public FlagBase {
    friend class ArgParser;

   public:
    Flag(const std::string &name, const std::string &description, T &bind_value)
        : FlagBase(name, description),
          value_(std::ref(bind_value)),
          action_{store_true} {}
    Flag(const std::string &name, const std::string &description, T &bind_value,
         std::function<void(T &)> action)
        : FlagBase(name, description),
          value_(std::ref(bind_value)),
          action_(std::move(action)) {}
    void parse() override {
        std::visit(overload{
                       [this](T &value) { action_(value); },
                       [this](std::reference_wrapper<T> &value) {
                           action_(value.get());
                       },
                   },
                   value_);
        count_++;
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
    virtual void parse(const std::string &opt_value) {
        this->opt_values.push_back(opt_value);
        count_++;
    }

    virtual bool is_multiple() const = 0;

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
            std::ranges::transform(short_opt_names_,
                           back_inserter(short_opts),
                           [](auto const &s) { return "-" + s; });
            std::ranges::transform(long_opt_names_,
                           back_inserter(long_opts),
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
    std::string default_value_str;
    std::vector<std::string> opt_values;
};

template <typename T>
    requires can_parse_from_string<T> || is_container<T>
class Option : public OptionBase {
    friend class ArgParser;

   public:
    Option(const std::string &name, const std::string &description,
           T &bind_value)
        requires can_parse_from_string_without_split<T>
        : OptionBase(name, description),
          value_(std::ref(bind_value)),
          action_([](T &value, const std::string &opt_value) {
              replace_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value, char delim)
        requires can_parse_from_string_split_once<T>
        : OptionBase(name, description),
          value_(std::ref(bind_value)),
          action_([delim](T &value, const std::string &opt_value) {
              replace_value<T>(value, opt_value, delim);
          }) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value)
        requires is_container<T> &&
                     can_parse_from_string_without_split<typename T::value_type>
        : OptionBase(name, description),
          value_(std::ref(bind_value)),
          action_([](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    Option(const std::string &name, const std::string &description,
           T &bind_value, char delim)
        requires is_container<T> &&
                     can_parse_from_string_split_once<typename T::value_type>
        : OptionBase(name, description),
          value_(std::ref(bind_value)),
          action_([delim](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value, delim);
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
    bool is_multiple() const override {
        if constexpr (is_container<T>) {
            return true;
        } else {
            return false;
        }
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
    requires can_parse_from_string<T> || is_container<T>
class Positional : public OptionBase {
    friend class ArgParser;

   public:
    Positional(const std::string &name, const std::string &description,
               T &bind_value)
        requires can_parse_from_string_without_split<T>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([](T &value, const std::string &opt_value) {
              replace_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    Positional(const std::string &name, const std::string &description,
               T &bind_value, char delim)
        requires can_parse_from_string_split_once<T>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([delim](T &value, const std::string &opt_value) {
              replace_value<T>(value, opt_value, delim);
          }) {
        set_default_value_help<T>();
    }
    Positional(const std::string &name, const std::string &description,
               T &bind_value)
        requires is_container<T> &&
                     can_parse_from_string_without_split<typename T::value_type>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value);
          }) {
        set_default_value_help<T>();
    }
    Positional(const std::string &name, const std::string &description,
               T &bind_value, char delim)
        requires is_container<T> &&
                     can_parse_from_string_split_once<typename T::value_type>
        : OptionBase(name, description),
          bind_value_(std::ref(bind_value)),
          action_([delim](T &value, const std::string &opt_value) {
              append_value<T>(value, opt_value, delim);
          }) {
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
    bool is_multiple() const override {
        if constexpr (is_container<T>) {
            return true;
        } else {
            return false;
        }
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
    ArgParser(std::string prog, std::string description)
        : program{std::move(prog)}, description(std::move(description)) {}
    void add_flag(const std::string &name, const std::string &description,
                  bool &bind_value,
                  std::function<void(bool &)> action = store_true) {
        args.push_back(std::make_unique<Flag<bool>>(name, description,
                                                    bind_value, std::move(action)));
    }
    void add_flag(const std::string &name, const std::string &description,
                  int &bind_value,
                  std::function<void(int &)> action = increment<int>) {
        args.push_back(
            std::make_unique<Flag<int>>(name, description, bind_value, std::move(action)));
    }

    template <typename T>
        requires can_parse_from_string<T> ||
                 (is_container<T> &&
                  can_parse_from_string<typename T::value_type>)
    void add_option(const std::string &name, const std::string &description,
                    T &bind_value) {
        args.push_back(
            std::make_unique<Option<T>>(name, description, bind_value));
    }

    template <typename T>
        requires can_parse_from_string_split_once<T> ||
                 (is_container<T> &&
                  can_parse_from_string_split_once<typename T::value_type>)
    void add_option(const std::string &name, const std::string &description,
                    T &bind_value, char delim) {
        args.push_back(
            std::make_unique<Option<T>>(name, description, bind_value, delim));
    }

    template <typename T>
        requires can_parse_from_string<T> ||
                 (is_container<T> &&
                  can_parse_from_string<typename T::value_type>)
    void add_positional(const std::string &name, const std::string &description,
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
    }

    template <typename T>
        requires can_parse_from_string_split_once<T> ||
                 (is_container<T> &&
                  can_parse_from_string_split_once<typename T::value_type>)
    void add_positional(const std::string &name, const std::string &description,
                        T &bind_value, char delim) {
        args.push_back(std::make_unique<Positional<T>>(name, description,
                                                       bind_value, delim));
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
                usage_str << " " << arg->usage(32, '.') << '\n';
            }
        }
        std::cout << usage_str.str() << std::endl;
    }
    void print_version() const {
        std::cout << "Version: " << version << std::endl;
    }
    ArgBase *get_arg(const std::string &name) const {
        if (name.length() == 1) {
            auto it =
                std::ranges::find_if(args, [name](const auto &arg) {
                    return std::find(arg->short_opt_names_.begin(),
                                     arg->short_opt_names_.end(),
                                     name) != arg->short_opt_names_.end();
                });
            return it != args.end() ? it->get() : nullptr;
        } else {
            auto it =
                std::ranges::find_if(args, [name](const auto &arg) {
                    return std::find(arg->long_opt_names_.begin(),
                                     arg->long_opt_names_.end(),
                                     name) != arg->long_opt_names_.end();
                });
            return it != args.end() ? it->get() : nullptr;
        }
    }
    void parse(int argc, const char *argv[]) const {
        std::vector<const char *> commands{argv, argv + argc};
        size_t i = 1;  // 跳过程序名
        if (!commands.empty() && commands[0] != nullptr &&
            commands[0][0] == '-') {
            i = 0;
        }
        std::vector<ArgBase *> positionals;

        // 收集所有的位置参数
        for (const auto &arg : args) {
            if (arg->is_positional()) {
                positionals.push_back(arg.get());
            }
        }

        size_t pos_index = 0;

        while (i < commands.size()) {
            const std::string &arg = commands[i];

            // 处理长选项 (--option)
            if (arg.size() > 2 && arg.substr(0, 2) == "--") {
                std::string name = arg.substr(2);
                auto eq_pos = name.find('=');
                std::string value;

                if (eq_pos != std::string::npos) {
                    value = name.substr(eq_pos + 1);
                    name = name.substr(0, eq_pos);
                }

                if (auto *option = get_arg(name)) {
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
            // 处理短选项 (-o)
            else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
                std::string opts = arg.substr(1);

                // 处理组合的短选项
                for (size_t j = 0; j < opts.size(); ++j) {
                    std::string name(1, opts[j]);
                    if (auto *option = get_arg(name)) {
                        if (option->is_flag()) {
                            auto *flag = dynamic_cast<FlagBase *>(option);
                            flag->parse();
                        } else if (option->is_option()) {
                            auto *opt = dynamic_cast<OptionBase *>(option);
                            if (j < opts.size() - 1) {
                                // 如果不是最后一个字符，剩余的部分作为值
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
            // 处理 -- 选项
            else if (arg == "--") {
                i++;
                break;
            }
            // 处理位置参数
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

        // 检查必需的参数是否都提供了
        for (const auto &arg : args) {
            if (arg->required_ && arg->count() == 0) {
                throw std::runtime_error(
                    "Required argument missing: " +
                    (!arg->long_opt_names_.empty()    ? arg->long_opt_names_[0]
                     : !arg->short_opt_names_.empty() ? arg->short_opt_names_[0]
                                                      : ""));
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
