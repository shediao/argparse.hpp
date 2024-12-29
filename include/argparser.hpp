//
// Created by shediao on 24-12-29.
//

#ifndef ARGPASER_HPP
#define ARGPASER_HPP

#include <string>
#include <vector>
#include <stdexcept>

namespace arg::parser {

namespace detail {
// 前置声明
template <typename T>
T from_string(std::string const& s);

inline std::vector<std::string> split(std::string const& s, char delim, int max) {
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
template<typename T>
concept is_container = requires(T t) {
    t.insert(t.end(), std::declval<typename T::value_type>());
};

template <typename T>
concept is_tuple = requires(T t) {
    std::get<0>(std::declval<T&>());
};

template <> inline std::string from_string<std::string>(std::string const& s) {
    return s;
}
// bool 特化
template <>
inline bool from_string<bool>(std::string const& s) {
    if (s == "true" || s == "on" || s == "1") {
        return true;
    }
    if (s == "false" || s == "off" || s == "0") {
        return false;
    }
    throw std::invalid_argument("Invalid string for bool: " + s);
}

#define ARGPARSER_TYPE_FROM_STRING(type, std_stox) \
template <> \
inline type from_string<type>(std::string const& s) { \
    try { \
        size_t pos = 0; \
        const type result{std_stox(s, &pos)}; \
        if (pos != s.size()) { \
            throw std::invalid_argument("Invalid string for " #type ": " + s); \
        } \
        return result; \
    } catch (const std::invalid_argument& e) { \
        throw std::invalid_argument("Invalid string for " #type ": " + s); \
    } catch (const std::out_of_range& e) { \
        throw std::out_of_range("Out of range for " #type ": " + s); \
    } \
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

// for tuple & pair
template<typename T>
requires is_tuple<T> && (!is_container<T>)
T from_string(std::string const &s, const char delim) {
    auto splited = detail::split(s, delim, std::tuple_size_v<std::decay_t<T>>);
    if (std::size(splited) != std::tuple_size_v<std::decay_t<T>>) {
        throw std::invalid_argument("Invalid string for split " + std::to_string(std::tuple_size_v<std::decay_t<T>>) + "th element:" + s);
    }
    T result;
    std::apply(
        [&splited]<typename... T0>(T0&&... args){
        int i = 0;
        ((args = from_string<std::decay_t<T0>>(splited[i++])), ...);
    }, result);
    return result;
}
template<typename T>
concept can_from_string_without_delim = requires(T t) {
    from_string<T>(std::declval<std::string>());
};

template<typename T>
requires is_container<T> && can_from_string_without_delim<typename T::value_type> && (!is_tuple<T>)
T from_string(std::string const &s, const char delim) {
    auto splited = detail::split(s, delim, -1);
    T result;
    for (auto const &item : splited) {
        result.insert(result.end(), from_string<typename T::value_type>(item));
    }
    return result;
}

template<typename T>
concept can_from_string_with_delim = requires(T t) {
    from_string<T>(std::declval<std::string>(), std::declval<char>());
};

template<typename T>
requires is_container<T> && can_from_string_with_delim<typename T::value_type> && (!is_tuple<T>)
T from_string(std::string const &s, const char delim, const char delim2) {
    auto splited = detail::split(s, delim, -1);
    T result;
    for (auto const &item : splited) {
        result.insert(result.end(), from_string<typename T::value_type>(item, delim2));
    }
    return result;
}

}



}

#endif //ARGPASER_HPP