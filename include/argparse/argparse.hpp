/*
 *  __ _ _ __ __ _ _ __   __ _ _ __ ___  ___  | |__  _ __  _ __
 * / _` | '__/ _` | '_ \ / _` | '__/ __|/ _ \ | '_ \| '_ \| '_ \
 *| (_| | | | (_| | |_) | (_| | |  \__ \  __/_| | | | |_) | |_) |
 * \__,_|_|  \__, | .__/ \__,_|_|  |___/\___(_)_| |_| .__/| .__/
 *           |___/|_|                               |_|   |_|
 *
 *  argparse.hpp — A lightweight, header-only, type-safe C++20 command-line
 *                 argument parser supporting GNU-style syntax.
 *
 *  Author  : shediao.xsd <xushediao1987@163.com>
 *  Repo    : https://github.com/shediao/argparse.hpp
 *  License : MIT
 *
 *  Copyright (c) 2024-2026 shediao.xsd
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *  SPDX-License-Identifier: MIT
 */

#ifndef __ARGPARSE_ARGPARSE_HPP__
#define __ARGPARSE_ARGPARSE_HPP__

#include <algorithm>
#include <cassert>
#include <cctype>
#include <concepts>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif  // _WIN32

#include <stdexcept>

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
using std::to_string;
using std::to_wstring;

inline constexpr size_t TOTAL_WIDTH = 80;

template <typename E>
class unexpected {
 public:
  using error_type = E;

  constexpr explicit unexpected(error_type const& e) : error_(e) {}
  constexpr explicit unexpected(error_type&& e) : error_(std::move(e)) {}

  constexpr error_type& error() & noexcept { return error_; }
  constexpr error_type const& error() const& noexcept { return error_; }
  constexpr error_type&& error() && noexcept { return std::move(error_); }

 private:
  E error_;
};

template <typename E>
constexpr unexpected<std::decay_t<E>> make_unexpected(E&& e) {
  return unexpected<std::decay_t<E>>(std::forward<E>(e));
}

template <typename E>
class bad_expected_access : public std::exception {
 public:
  explicit bad_expected_access(E e) : error_(std::move(e)) {}
  const char* what() const noexcept override { return "bad expected access"; }
  E const& error() const& noexcept { return error_; }

 private:
  E error_;
};

template <typename T, typename E>
union expected_union {
  constexpr expected_union() {}
  constexpr ~expected_union() {}
  T value;
  E error;
};

template <typename T, typename E,
          bool Trivial = std::is_trivially_destructible<T>::value &&
                         std::is_trivially_destructible<E>::value>
struct expected_storage;

template <typename T, typename E>
struct expected_storage<T, E, true> {
  constexpr expected_storage() : has_value(false) {}
  expected_union<T, E> storage;
  bool has_value;
};

template <typename T, typename E>
struct expected_storage<T, E, false> {
  constexpr expected_storage() : has_value(false) {}
  ~expected_storage() = default;
  void destroy() {
    if (has_value) {
      storage.value.~T();
    } else {
      storage.error.~E();
    }
  }
  expected_union<T, E> storage;
  bool has_value;
};
template <typename T, typename E>
class expected;

template <typename T>
struct is_expected : std::false_type {};

template <typename T, typename E>
struct is_expected<expected<T, E>> : std::true_type {};

template <typename T>
constexpr bool is_expected_v = is_expected<T>::value;

template <typename T, typename E>
class expected : private expected_storage<T, E> {
  using base = expected_storage<T, E>;

 public:
  using value_type = T;
  using error_type = E;

  // Constructors
  constexpr expected(T const& value) { construct_value(value); }
  constexpr expected(T&& value) { construct_value(std::move(value)); }
  constexpr expected(unexpected<E> const& e) { construct_error(e.error()); }
  constexpr expected(unexpected<E>&& e) {
    construct_error(std::move(e).error());
  }
  template <typename U>
    requires std::is_convertible_v<U, E>
  constexpr expected(unexpected<U> const& e) {
    construct_error(e.error());
  }
  template <typename U>
    requires std::is_convertible_v<U, E>
  constexpr expected(unexpected<U>&& e) {
    construct_error(std::move(e).error());
  }
  constexpr expected(expected const& other) { copy_from(other); }
  constexpr expected(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_constructible_v<E>) {
    move_from(std::move(other));
  }

  // Assignment
  constexpr expected& operator=(expected const& other) {
    if (this == &other) {
      return *this;
    }
    reset();
    copy_from(other);
    return *this;
  }

  constexpr expected& operator=(expected&& other) noexcept(
      std::is_nothrow_move_assignable_v<T> &&
      std::is_nothrow_move_assignable_v<E>) {
    if (this == &other) {
      return *this;
    }
    reset();
    move_from(std::move(other));
    return *this;
  }

  // Observer
  constexpr bool has_value() const noexcept { return this->base::has_value; }

  constexpr explicit operator bool() const noexcept { return has_value(); }

  constexpr T& value() & { return check_value(), this->storage.value; }
  constexpr T const& value() const& {
    return check_value(), this->storage.value;
  }
  constexpr T&& value() && {
    return check_value(), std::move(this->storage.value);
  }
  constexpr const T&& value() const&& {
    return check_value(), std::move(this->storage.value);
  }

  constexpr E& error() & {
    assert(!has_value());
    return this->storage.error;
  }
  constexpr E const& error() const& {
    assert(!has_value());
    return this->storage.error;
  }
  constexpr E&& error() && {
    assert(!has_value());
    return std::move(this->storage.error);
  }
  constexpr const E&& error() const&& {
    assert(!has_value());
    return std::move(this->storage.error);
  }

  // Operators
  constexpr T& operator*() & { return value(); }
  constexpr T const& operator*() const& { return value(); }
  constexpr T&& operator*() && { return std::move(value()); }
  constexpr T* operator->() { return &value(); }
  constexpr T const* operator->() const { return &value(); }

  // Modifiers
  template <typename... Args>
  constexpr T& emplace(Args&&... args) {
    reset();
    new (&this->storage.value) T(std::forward<Args>(args)...);
    this->base::has_value = true;
    return this->storage.value;
  }

  // value_or
  template <typename U>
  constexpr T value_or(U&& default_value) const& {
    if (has_value()) {
      return this->storage.value;
    }
    return static_cast<T>(std::forward<U>(default_value));
  }

  // monadic api
  template <typename F>
  constexpr auto and_then(F&& f) & {
    using result_type = std::invoke_result_t<F, T&>;
    static_assert(is_expected_v<result_type>, "and_then must return expected");
    if (has_value()) {
      return std::invoke(std::forward<F>(f), this->storage.value);
    }
    return result_type(make_unexpected(error()));
  }

  template <typename F>
  constexpr auto and_then(F&& f) const& {
    using result_type = std::invoke_result_t<F, const T&>;
    static_assert(is_expected_v<result_type>, "and_then must return expected");
    if (has_value()) {
      return std::invoke(std::forward<F>(f), this->storage.value);
    }
    return result_type(make_unexpected(error()));
  }

  template <typename F>
  constexpr auto and_then(F&& f) && {
    using result_type = std::invoke_result_t<F, T&&>;
    static_assert(is_expected_v<result_type>, "and_then must return expected");
    if (has_value()) {
      return std::invoke(std::forward<F>(f), std::move(this->storage.value));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&& {
    using result_type = std::invoke_result_t<F, const T&&>;
    static_assert(is_expected_v<result_type>, "and_then must return expected");
    if (has_value()) {
      return std::invoke(std::forward<F>(f), std::move(this->storage.value));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto transform(
      F&& f) & -> expected<std::invoke_result_t<F, T&>, E> {
    using result_type = expected<std::invoke_result_t<F, T&>, E>;
    if (has_value()) {
      return result_type(std::invoke(std::forward<F>(f), this->storage.value));
    }
    return result_type(make_unexpected(error()));
  }

  template <typename F>
  constexpr auto transform(
      F&& f) const& -> expected<std::invoke_result_t<F, const T&>, E> {
    using result_type = expected<std::invoke_result_t<F, const T&>, E>;
    if (has_value()) {
      return result_type(std::invoke(std::forward<F>(f), this->storage.value));
    }
    return result_type(make_unexpected(error()));
  }

  template <typename F>
  constexpr auto transform(
      F&& f) && -> expected<std::invoke_result_t<F, T&&>, E> {
    using result_type = expected<std::invoke_result_t<F, T&&>, E>;
    if (has_value()) {
      return result_type(
          std::invoke(std::forward<F>(f), std::move(this->storage.value)));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto transform(
      F&& f) const&& -> expected<std::invoke_result_t<F, const T&&>, E> {
    using result_type = expected<std::invoke_result_t<F, const T&&>, E>;
    if (has_value()) {
      return result_type(
          std::invoke(std::forward<F>(f), std::move(this->storage.value)));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) & -> expected<T, std::invoke_result_t<F, E&>> {
    using result_type = expected<T, std::invoke_result_t<F, E&>>;
    if (has_value()) {
      return result_type(this->storage.value);
    }
    return result_type(
        make_unexpected(std::invoke(std::forward<F>(f), error())));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) const& -> expected<T, std::invoke_result_t<F, const E&>> {
    using result_type = expected<T, std::invoke_result_t<F, const E&>>;
    if (has_value()) {
      return result_type(this->storage.value);
    }
    return result_type(
        make_unexpected(std::invoke(std::forward<F>(f), error())));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) && -> expected<T, std::invoke_result_t<F, E&&>> {
    using result_type = expected<T, std::invoke_result_t<F, E&&>>;
    if (has_value()) {
      return result_type(std::move(this->storage.value));
    }
    return result_type(make_unexpected(
        std::invoke(std::forward<F>(f), std::move(*this).error())));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) const&& -> expected<T, std::invoke_result_t<F, const E&&>> {
    using result_type = expected<T, std::invoke_result_t<F, const E&&>>;
    if (has_value()) {
      return result_type(this->storage.value);
    }
    return result_type(make_unexpected(
        std::invoke(std::forward<F>(f), std::move(*this).error())));
  }

  template <typename F>
  constexpr expected or_else(F&& f) & {
    if (has_value()) {
      return *this;
    }
    return std::invoke(std::forward<F>(f), error());
  }

  template <typename F>
  constexpr expected or_else(F&& f) const& {
    if (has_value()) {
      return *this;
    }
    return std::invoke(std::forward<F>(f), error());
  }

  template <typename F>
  constexpr expected or_else(F&& f) && {
    if (has_value()) {
      return std::move(*this);
    }
    return std::invoke(std::forward<F>(f), std::move(*this).error());
  }

  template <typename F>
  constexpr expected or_else(F&& f) const&& {
    if (has_value()) {
      return std::move(*this);
    }
    return std::invoke(std::forward<F>(f), std::move(*this).error());
  }

  // swap
  constexpr void swap(expected& other) {
    using std::swap;
    if (has_value() && other.has_value()) {
      swap(this->storage.value, other.storage.value);
      return;
    }
    if (!has_value() && !other.has_value()) {
      swap(this->storage.error, other.storage.error);
      return;
    }
    expected tmp(std::move(other));
    other = std::move(*this);
    *this = std::move(tmp);
  }

  // Destructor
  constexpr ~expected() { reset(); }

  bool operator==(const expected& other) const {
    if (has_value() && other.has_value()) {
      return this->storage.value == other.storage.value;
    }
    if (!has_value() && !other.has_value()) {
      return this->storage.error == other.storage.error;
    }
    return false;
  }

  bool operator==(const T& value) const {
    return has_value() && this->storage.value == value;
  }

 private:
  template <typename U>
  constexpr void construct_value(U&& v) {
    new (&this->storage.value) T(std::forward<U>(v));
    this->base::has_value = true;
  }

  template <typename G>
  constexpr void construct_error(G&& g) {
    new (&this->storage.error) E(std::forward<G>(g));
    this->base::has_value = false;
  }

  constexpr void reset() {
    if constexpr (!std::is_trivially_destructible<T>::value ||
                  !std::is_trivially_destructible<E>::value) {
      this->base::destroy();
    }
  }

  constexpr void check_value() const {
#ifdef __cpp_exceptions
    if (!has_value()) {
      throw bad_expected_access<E>(this->storage.error);
    }
#else
    assert(has_value());
#endif
  }

  constexpr void copy_from(const expected& other) {
    if (other.has_value()) {
      construct_value(other.storage.value);
    } else {
      construct_error(other.storage.error);
    }
  }

  constexpr void move_from(expected&& other) {
    if (other.has_value()) {
      construct_value(std::move(other.storage.value));
    } else {
      construct_error(std::move(other.storage.error));
    }
  }
};

template <typename E>
class expected<void, E> {
 public:
  using error_type = E;

  constexpr expected() noexcept : has_value_(true) {}

  constexpr expected(unexpected<E> const& e) : has_value_(false) {
    new (&error_) E(e.error());
  }

  constexpr expected(unexpected<E>&& e) : has_value_(false) {
    new (&error_) E(std::move(e.error()));
  }

  constexpr expected(expected const& other) : has_value_(other.has_value_) {
    if (!has_value_) {
      new (&error_) E(other.error_);
    }
  }

  constexpr expected(expected&& other) : has_value_(other.has_value_) {
    if (!has_value_) {
      new (&error_) E(std::move(other.error_));
    }
  }

  constexpr ~expected() {
    if (!has_value_) {
      error_.~E();
    }
  }

  constexpr bool has_value() const noexcept { return has_value_; }

  constexpr explicit operator bool() const noexcept { return has_value_; }

  constexpr void value() const { check_value(); }

  constexpr E& error() & { return error_; }

  constexpr E const& error() const& { return error_; }

  constexpr E&& error() && { return std::move(error_); }
  constexpr const E&& error() const&& { return std::move(error_); }

  // Assignment
  constexpr expected& operator=(expected const& other) {
    if (this == &other) {
      return *this;
    }
    if (!has_value_) {
      error_.~E();
    }
    has_value_ = other.has_value_;
    if (!has_value_) {
      new (&error_) E(other.error_);
    }
    return *this;
  }

  constexpr expected& operator=(expected&& other) {
    if (this == &other) {
      return *this;
    }
    if (!has_value_) {
      error_.~E();
    }
    has_value_ = other.has_value_;
    if (!has_value_) {
      new (&error_) E(std::move(other.error_));
    }
    return *this;
  }

  template <typename F>
  constexpr auto and_then(F&& f) & {
    using result_type = std::invoke_result_t<F>;
    if (has_value_) {
      return std::invoke(std::forward<F>(f));
    }
    return result_type(make_unexpected(error_));
  }

  template <typename F>
  constexpr auto and_then(F&& f) const& {
    using result_type = std::invoke_result_t<F>;
    if (has_value_) {
      return std::invoke(std::forward<F>(f));
    }
    return result_type(make_unexpected(error_));
  }

  template <typename F>
  constexpr auto and_then(F&& f) && {
    using result_type = std::invoke_result_t<F>;
    if (has_value_) {
      return std::invoke(std::forward<F>(f));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&& {
    using result_type = std::invoke_result_t<F>;
    if (has_value_) {
      return std::invoke(std::forward<F>(f));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto transform(F&& f) & -> expected<std::invoke_result_t<F>, E> {
    using result_type = expected<std::invoke_result_t<F>, E>;
    if (has_value_) {
      return result_type(std::invoke(std::forward<F>(f)));
    }
    return result_type(make_unexpected(error_));
  }

  template <typename F>
  constexpr auto transform(
      F&& f) const& -> expected<std::invoke_result_t<F>, E> {
    using result_type = expected<std::invoke_result_t<F>, E>;
    if (has_value_) {
      return result_type(std::invoke(std::forward<F>(f)));
    }
    return result_type(make_unexpected(error_));
  }

  template <typename F>
  constexpr auto transform(F&& f) && -> expected<std::invoke_result_t<F>, E> {
    using result_type = expected<std::invoke_result_t<F>, E>;
    if (has_value_) {
      return result_type(std::invoke(std::forward<F>(f)));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto transform(
      F&& f) const&& -> expected<std::invoke_result_t<F>, E> {
    using result_type = expected<std::invoke_result_t<F>, E>;
    if (has_value_) {
      return result_type(std::invoke(std::forward<F>(f)));
    }
    return result_type(make_unexpected(std::move(*this).error()));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) & -> expected<void, std::invoke_result_t<F, E&>> {
    using result_type = expected<void, std::invoke_result_t<F, E&>>;
    if (has_value_) {
      return result_type();
    }
    return result_type(
        make_unexpected(std::invoke(std::forward<F>(f), error_)));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) const& -> expected<void, std::invoke_result_t<F, const E&>> {
    using result_type = expected<void, std::invoke_result_t<F, const E&>>;
    if (has_value_) {
      return result_type();
    }
    return result_type(
        make_unexpected(std::invoke(std::forward<F>(f), error_)));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) && -> expected<void, std::invoke_result_t<F, E&&>> {
    using result_type = expected<void, std::invoke_result_t<F, E&&>>;
    if (has_value_) {
      return result_type();
    }
    return result_type(make_unexpected(
        std::invoke(std::forward<F>(f), std::move(*this).error())));
  }

  template <typename F>
  constexpr auto transform_error(
      F&& f) const&& -> expected<void, std::invoke_result_t<F, const E&&>> {
    using result_type = expected<void, std::invoke_result_t<F, const E&&>>;
    if (has_value_) {
      return result_type();
    }
    return result_type(make_unexpected(
        std::invoke(std::forward<F>(f), std::move(*this).error())));
  }

  // swap
  constexpr void swap(expected& other) {
    using std::swap;
    if (has_value_ && other.has_value_) {
      return;  // both void, nothing to swap
    }
    if (!has_value_ && !other.has_value_) {
      swap(error_, other.error_);
      return;
    }
    expected tmp(std::move(other));
    other = std::move(*this);
    *this = std::move(tmp);
  }

 private:
  constexpr void check_value() const {
#ifdef __cpp_exceptions
    if (!has_value_) {
      throw bad_expected_access<E>(error_);
    }
#else
    assert(has_value_);
#endif
  }

  union {
    E error_;
  };

  bool has_value_;
};

template <typename T, typename E>
constexpr bool operator==(expected<T, E> const& lhs,
                          expected<T, E> const& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }

  if (lhs.has_value()) {
    return lhs.value() == rhs.value();
  }

  return lhs.error() == rhs.error();
}

template <typename E>
constexpr bool operator==(expected<void, E> const& lhs,
                          expected<void, E> const& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }

  if (lhs.has_value()) {
    return true;  // both void, both have value
  }

  return lhs.error() == rhs.error();
}

namespace detail {
inline size_t& option_name_width_impl() {
  static size_t width = 24;
  return width;
}
template <typename T>
std::string join(const std::vector<std::string>& v, const T& delim) {
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

template <typename C, typename CharT, typename F>
  requires requires(F f, CharT c) {
    { f(c) } -> std::convertible_to<bool>;
  } &&
           std::is_constructible_v<
               C, typename std::vector<typename C::value_type>::iterator,
               typename std::vector<typename C::value_type>::iterator>
C split_to_if(const std::basic_string<CharT>& str, F f, int split_count = -1,
              bool compress_tokens = false) {
  auto begin = str.begin();
  auto delimiter = begin;
  int count = 0;

  std::vector<typename C::value_type> to;
  while ((split_count < 0 || count++ < split_count) &&
         (delimiter = std::find_if(begin, str.end(), f)) != str.end()) {
    to.emplace_back(begin, delimiter);
    if (compress_tokens) {
      begin = std::find_if_not(delimiter, str.end(), f);
      if (begin == str.end()) {
        return C{to.begin(), to.end()};
      }
    } else {
      begin = std::next(delimiter);
    }
  }

  to.emplace_back(begin, str.end());
  return C{to.begin(), to.end()};
}

inline std::vector<std::string> split(std::string const& s, char delim,
                                      int split_count) {
  return split_to_if<std::vector<std::string>>(
      s, [delim](char c) { return c == delim; }, split_count, false);
}

using std::to_string;
using std::to_wstring;
inline std::string to_string(std::string const& s) { return s; }
inline std::wstring to_wstring(std::wstring const& s) { return s; }

inline void die(std::string const& msg) { throw std::runtime_error(msg); }

inline void report_invalid_argument(std::string const& msg) {
  throw std::invalid_argument(msg);
}

}  // namespace detail

/// Get the current option name width (default: 24).
inline size_t option_name_width() { return detail::option_name_width_impl(); }

/// Set the option name width globally for help/usage formatting.
inline void set_option_name_width(size_t width) {
  detail::option_name_width_impl() = width;
}

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
struct is_string : std::false_type {};

template <class _CharT, class _Traits, class _Allocator>
struct is_string<std::basic_string<_CharT, _Traits, _Allocator>>
    : std::true_type {};

template <typename T>
constexpr bool is_string_v = is_string<T>::value;

template <typename T>
struct is_string_view : std::false_type {};

template <class _CharT, class _Traits>
struct is_string_view<std::basic_string_view<_CharT, _Traits>>
    : std::true_type {};

template <typename T>
constexpr bool is_string_view_v = is_string_view<T>::value;

template <typename T>
  requires(!is_optional_v<T> && !is_string_view_v<T> &&
           ((std::is_integral_v<T> && (sizeof(T) > sizeof(char)) &&
             (sizeof(T) <= sizeof(long long int)) &&
             !std::is_same_v<T, wchar_t> && !std::is_same_v<T, char16_t> &&
             !std::is_same_v<T, char32_t>) ||
            std::is_same_v<bool, T> || std::is_same_v<char, T> ||
            std::is_floating_point_v<T> ||
            std::is_constructible_v<T, std::string> ||
            std::is_convertible_v<std::string, T>))
expected<T, std::string> from_string(std::string const& s) {
  try {
    size_t pos = 0;
    if constexpr (std::is_same_v<T, bool>) {
      if (s == "true" || s == "on" || s == "yes" || s == "1") {
        return true;
      }
      if (s == "false" || s == "off" || s == "no" || s == "0") {
        return false;
      }
      return make_unexpected("Invalid boolean value: " + s);
    } else if constexpr (std::is_same_v<T, char>) {
      if (s.size() != 1) {
        return make_unexpected("Invalid character: " + s);
      }
      return s[0];
    } else if constexpr (std::is_floating_point_v<T>) {
      auto result = std::stold(s, &pos);
      if (pos != s.size()) {
        return make_unexpected("Invalid floating-point number: " + s);
      }
      if (result > (std::numeric_limits<T>::max)() ||
          result < (std::numeric_limits<T>::lowest)()) {
        return make_unexpected("Overflow: " + s);
      }
      return static_cast<T>(result);
    } else if constexpr (std::is_unsigned_v<T> && std::is_integral_v<T>) {
      auto result = std::stoull(s, &pos);
      if (pos != s.size()) {
        return make_unexpected("Invalid unsigned integral: " + s);
      }
      if (result == (std::numeric_limits<unsigned long long>::max)()) {
        return (std::numeric_limits<T>::max)();
      }
      if (result > (std::numeric_limits<T>::max)()) {
        return make_unexpected("Overflow: " + s);
      }
      return static_cast<T>(result);
    } else if constexpr (std::is_signed_v<T> && std::is_integral_v<T>) {
      auto result = std::stoll(s, &pos);
      if (pos != s.size()) {
        return make_unexpected("Invalid signed integral: " + s);
      }
      if (result > (std::numeric_limits<T>::max)() ||
          result < (std::numeric_limits<T>::min)()) {
        return make_unexpected("Overflow: " + s);
      }
      return static_cast<T>(result);
    } else if constexpr (std::is_constructible_v<T, std::string>) {
      return T{s};
    } else if constexpr (std::is_convertible_v<std::string, T>) {
      return static_cast<T>(s);
    } else {
      return make_unexpected("Unsupported type for from_string");
    }
  } catch (const std::out_of_range& e) {
    return make_unexpected("Out of range: " + s + ", " + e.what());
  } catch (const std::invalid_argument& e) {
    return make_unexpected("Invalid number: " + s + ", " + e.what());
  }
  return make_unexpected("Unsupported type for from_string");
}

template <typename T>
concept can_from_string_without_delim = requires {
  {
    from_string<T>(std::declval<std::string>())
  } -> std::same_as<expected<T, std::string>>;
};

template <typename T>
concept can_from_string_all_tuple_like_items = requires {
  typename std::tuple_size<T>::type;
  requires std::tuple_size<T>::value > 0;
} && []<std::size_t... I>(std::integer_sequence<std::size_t, I...>) constexpr {
  return (
      can_from_string_without_delim<std::decay_t<std::tuple_element_t<I, T>>> &&
      ...);
}(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T>
  requires can_from_string_all_tuple_like_items<T>
expected<T, std::string> from_string(const std::string& s, char delim) {
  auto values = split(s, delim, std::tuple_size_v<T> - 1);
  if (values.size() != std::tuple_size_v<T>) {
    return make_unexpected("Invalid tuple value: " + s + " expected " +
                           std::to_string(std::tuple_size_v<T>) + " " +
                           (std::tuple_size_v<T> == 1 ? "value" : "values") +
                           ".");
  }
  try {
    return []<size_t... I>(std::vector<std::string> const& values,
                           std::integer_sequence<std::size_t, I...>) {
      return T{from_string<std::decay_t<std::tuple_element_t<I, T>>>(values[I])
                   .value()...};
    }(values, std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>());
  } catch (bad_expected_access<std::string> const& e) {
    return make_unexpected(e.error());
  }
}

template <typename T>
concept can_from_string_with_delim = requires {
  {
    from_string<T>(std::declval<std::string>(), std::declval<char>())
  } -> std::same_as<expected<T, std::string>>;
};

template <typename T>
concept can_from_string =
    can_from_string_without_delim<T> || can_from_string_with_delim<T>;

template <typename T>
concept is_container = requires(T t) {
  typename T::value_type;
  std::is_constructible_v<
      T, typename std::vector<typename T::value_type>::iterator,
      typename std::vector<typename T::value_type>::iterator>;
} && (
    requires(T t){ t.push_back(std::declval<typename T::value_type>()); }
    ||
    requires(T t){ t.insert(t.end(), std::declval<typename T::value_type>()); }
    ||
    requires(T t){ t.push(std::declval<typename T::value_type>()); }
    );

template <typename T>
concept is_non_string_container = is_container<T> && !is_string_v<T>;

template <typename T>
concept from_string_container =
    is_non_string_container<T> &&
    (can_from_string_without_delim<typename T::value_type> ||
     can_from_string_with_delim<typename T::value_type>);

template <typename T>
concept bindable_without_delim =
    (is_optional_v<T> &&
     can_from_string_without_delim<typename T::value_type>) ||
    (!is_optional_v<T> && can_from_string_without_delim<T>) ||
    (is_non_string_container<T> &&
     can_from_string_without_delim<typename T::value_type>);

template <typename T>
concept bindable_with_delim =
    (is_optional_v<T> && can_from_string_with_delim<typename T::value_type>) ||
    (!is_optional_v<T> && can_from_string_with_delim<T>) ||
    (is_non_string_container<T> &&
     can_from_string_with_delim<typename T::value_type>);

template <typename T>
concept bindable = bindable_without_delim<T> || bindable_with_delim<T>;

template <typename T>
concept has_to_string = requires(T t) {
  { to_string(t) } -> std::convertible_to<std::string>;
};
template <typename T>
concept has_to_wstring = requires(T t) {
  { to_wstring(t) } -> std::convertible_to<std::wstring>;
};

template <typename T>
concept has_to_string_memfunc = requires(T t) {
  { t.to_string() } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_to_wstring_memfunc = requires(T t) {
  { t.to_wstring() } -> std::convertible_to<std::wstring>;
};

template <typename T>
concept has_string_memfunc = requires(T t) {
  { t.string() } -> std::convertible_to<std::string>;
};

template <typename T>
concept has_wstring_memfunc = requires(T t) {
  { t.wstring() } -> std::convertible_to<std::wstring>;
};

template <typename T, typename CharT = char>
concept has_c_str_memfunc = requires(T t) {
  { t.c_str() } -> std::same_as<const CharT*>;
};

template <typename T>
struct extract_value_type {
  using type = T;
};

template <typename T>
struct extract_value_type<std::optional<T>> {
  using type = T;
};

template <from_string_container T>
struct extract_value_type<T> {
  using type = typename T::value_type;
};

template <typename T>
using extract_value_type_t = typename extract_value_type<T>::type;

// Word-wrap a single segment of text (no embedded \n) to fit within
// the given width.  Break on spaces when possible; force-break only
// when a single word is wider than the requested width.
inline std::string word_wrap(const std::string& text, size_t width) {
  if (text.empty() || width == 0) {
    return text;
  }
  std::string result;
  result.reserve(text.size() + text.size() / (width ? width : 1));
  size_t pos = 0;
  while (pos < text.size()) {
    // Skip a single leading space on continuation lines.
    if (!result.empty() && pos < text.size() && text[pos] == ' ') {
      ++pos;
    }
    size_t remaining = text.size() - pos;
    if (remaining <= width) {
      if (!result.empty()) {
        result += '\n';
      }
      result += text.substr(pos);
      break;
    }
    // Look backwards for a space.
    size_t break_at = pos + width;
    while (break_at > pos && text[break_at] != ' ') {
      --break_at;
    }
    if (break_at == pos) {
      break_at = pos + width;  // force break
    }
    if (!result.empty()) {
      result += '\n';
    }
    result += text.substr(pos, break_at - pos);
    pos = break_at;
  }
  return result;
}

// Format a help entry in two columns:
//   column 1 : option_name (width characters)
//   column 2 : description (word-wrapped to fit within 80 - width chars)
//
// If the option_name already contains embedded '\\n' (because the caller
// placed multiple names on separate lines), only the *last* line is used
// to decide where the description begins.
//
// The 'prefix' parameter is prepended to the first line and to every
// continuation line, ensuring consistent indentation when the caller
// places the formatted block inside a larger layout.
inline std::string format(std::string const& option_name,
                          std::string const& description,
                          std::string const& prefix = "") {
  constexpr size_t MINIMUM_DESCRIPTION_WIDTH = 40;
  size_t desc_width =
      (TOTAL_WIDTH > option_name_width() + MINIMUM_DESCRIPTION_WIDTH)
          ? (TOTAL_WIDTH - option_name_width())
          : MINIMUM_DESCRIPTION_WIDTH;

  // Length of the last line of option_name (may be multi-line).
  auto nl_pos = option_name.rfind('\n');
  size_t last_len = (nl_pos == std::string::npos)
                        ? option_name.size()
                        : (option_name.size() - nl_pos - 1);

  std::string result = prefix + option_name;

  // Split the description on existing \n (intentional hard breaks),
  // word-wrap each segment, then emit every line with proper indentation.
  size_t seg_start = 0;
  bool first_out_line = true;

  while (seg_start <= description.size()) {
    auto hard_nl = description.find('\n', seg_start);
    auto segment = (hard_nl == std::string::npos)
                       ? description.substr(seg_start)
                       : description.substr(seg_start, hard_nl - seg_start);

    // Word-wrap this segment.
    auto wrapped = word_wrap(segment, desc_width);

    // Emit the wrapped lines.
    size_t wpos = 0;
    while (wpos < wrapped.size()) {
      auto wnl = wrapped.find('\n', wpos);
      auto line = (wnl == std::string::npos) ? wrapped.substr(wpos)
                                             : wrapped.substr(wpos, wnl - wpos);

      if (first_out_line && last_len < option_name_width()) {
        // First description line goes on the same line as the option.
        result.append(option_name_width() - last_len, ' ');
      } else {
        result += '\n';
        result += prefix;
        result.append(option_name_width(), ' ');
      }
      result += line;
      first_out_line = false;

      if (wnl == std::string::npos) {
        break;
      }
      wpos = wnl + 1;
    }

    if (hard_nl == std::string::npos) {
      break;
    }
    seg_start = hard_nl + 1;
  }

  return result;
}

#if defined(_WIN32)
// Helper function to convert a UTF-8 std::string to a UTF-16 std::wstring
inline std::wstring to_wstring(const std::string_view& str) {
  if (str.empty()) {
    return {};
  }
  DWORD size_needed = MultiByteToWideChar(
      CP_UTF8, 0, str.data(), static_cast<DWORD>(str.size()), NULL, 0);
  if (size_needed <= 0) {
    // TODO: throw an exception on conversion errors
    return {};
  }
  std::wstring wstr(size_needed, L'\0');
  DWORD size = MultiByteToWideChar(CP_UTF8, 0, str.data(),
                                   static_cast<DWORD>(str.size()), wstr.data(),
                                   size_needed);
  wstr.resize(size);
  return wstr;
}

// Helper function to convert a UTF-16 std::wstring to a UTF-8 std::string
inline std::string to_string(const std::wstring_view& wstr) {
  if (wstr.empty()) {
    return {};
  }
  DWORD size_needed =
      WideCharToMultiByte(CP_UTF8, 0, wstr.data(),
                          static_cast<DWORD>(wstr.size()), NULL, 0, NULL, NULL);
  if (size_needed <= 0) {
    // TODO: throw an exception on conversion errors
    return {};
  }
  std::string str(size_needed, '\0');
  DWORD size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(),
                                   static_cast<DWORD>(wstr.size()), str.data(),
                                   size_needed, NULL, NULL);
  str.resize(size);
  return str;
}
#endif  // _WIN32

inline std::optional<std::string> getenv(std::string const& name) {
#if defined(_WIN32)
  auto wname = to_wstring(name);
  auto size = GetEnvironmentVariableW(wname.c_str(), nullptr, 0);
  if (size == 0) {
    return GetLastError() == ERROR_ENVVAR_NOT_FOUND
               ? std::nullopt
               : std::optional<std::string>{""};
  }
  std::wstring ret(size, L'\0');
  size = GetEnvironmentVariableW(wname.data(), ret.data(), size);
  ret.resize(size);
  return to_string(ret);
#else
  auto* env = ::getenv(name.c_str());
  if (env) {
    return std::string(env);
  }
  return std::nullopt;
#endif
}

template <typename T>
inline std::string to_string(T const& value)
  requires(has_to_string_memfunc<T> || has_string_memfunc<T> ||
           has_c_str_memfunc<T, char> || std::is_convertible_v<T, std::string>)
{
  if constexpr (has_to_string_memfunc<T>) {
    return value.to_string();
  } else if constexpr (has_string_memfunc<T>) {
    return value.string();
  } else if constexpr (has_c_str_memfunc<T, char>) {
    return value.c_str();
  } else {
    return static_cast<std::string>(value);
  }
}

template <typename T>
inline std::wstring to_wstring(T const& value)
  requires(has_to_wstring_memfunc<T> || has_wstring_memfunc<T> ||
           has_c_str_memfunc<T, wchar_t> ||
           std::is_convertible_v<T, std::wstring>)
{
  if constexpr (has_to_wstring_memfunc<T>) {
    return value.to_wstring();
  } else if constexpr (has_wstring_memfunc<T>) {
    return value.wstring();
  } else if constexpr (has_c_str_memfunc<T, wchar_t>) {
    return value.c_str();
  } else {
    return static_cast<std::wstring>(value);
  }
}

template <is_container T>
void push_back(T& t, typename T::value_type&& value) {
  if constexpr (requires(T t) {
                  t.emplace_back(std::declval<typename T::value_type&&>());
                }) {
    t.emplace_back(std::move(value));
  } else if constexpr (requires(T t) {
                         t.push_back(std::declval<typename T::value_type&&>());
                       }) {
    t.push_back(std::move(value));
  } else if constexpr (requires(T t) {
                         t.insert(t.end(),
                                  std::declval<typename T::value_type&&>());
                       }) {
    t.insert(t.end(), std::move(value));
  } else if constexpr (requires(T t) {
                         t.push(std::declval<typename T::value_type&&>());
                       }) {
    t.push(std::move(value));
  } else {
    throw std::runtime_error("Unsupported container type");
  }
}
template <is_container T>
void push_back(T& t, typename T::value_type const& value) {
  if constexpr (requires(T t) {
                  t.emplace_back(std::declval<typename T::value_type>());
                }) {
    t.emplace_back(value);
  } else if constexpr (requires(T t) {
                         t.push_back(std::declval<typename T::value_type>());
                       }) {
    t.push_back(value);
  } else if constexpr (requires(T t) {
                         t.insert(t.end(),
                                  std::declval<typename T::value_type>());
                       }) {
    t.insert(t.end(), value);
  } else if constexpr (requires(T t) {
                         t.push(std::declval<typename T::value_type>());
                       }) {
    t.push(value);
  } else {
    throw std::runtime_error("Unsupported container type");
  }
}

}  // namespace detail

using argparse::detail::to_string;
using argparse::detail::to_wstring;
using detail::from_string;

inline void store_true(bool& value) { value = true; }

inline void store_false(bool& value) { value = false; }

template <typename T>
  requires std::integral<T>
inline void increment(T& value) {
  ++value;
}

template <typename T>
  requires std::integral<T>
inline void decrement(T& value) {
  --value;
}
struct token;
class argv_stream;
class Command;
class ArgParser;

inline expected<std::tuple<std::vector<char>, std::vector<std::string>>,
                std::string>
parse_option_name(std::string const& name) {
  std::tuple<std::vector<char>, std::vector<std::string>> ret;
  for (auto&& opt_name : detail::split(name, ',', -1)) {
    if (opt_name.empty()) {
      continue;
    }
    if (opt_name[0] == '-') {
      return make_unexpected("Invalid option name: " + name + ", " + opt_name +
                             " starts with '-'");
    }
    if (std::find_if(opt_name.begin(), opt_name.end(), [](unsigned char c) {
          return std::isblank(c);
        }) != opt_name.end()) {
      return make_unexpected("Invalid option name: " + name + ", " + opt_name +
                             " contains whitespace");
    }
    if (opt_name.find_first_of("<>[]{}()|;!`&$\\ \a\b\f\n\r\t\v") !=
        std::string::npos) {
      return make_unexpected("Invalid option name: " + name + ", " + opt_name +
                             " contains special characters (<>[]{}()|;!`&$\\ "
                             "\\a\\b\\f\\n\\r\\t\\v) ");
    }
    auto& [short_names, long_names] = ret;
    if (opt_name.length() == 1 &&
        std::find(short_names.begin(), short_names.end(), opt_name[0]) ==
            short_names.end()) {
      short_names.push_back(opt_name[0]);
    }
    if (opt_name.length() > 1 && std::find(long_names.begin(), long_names.end(),
                                           opt_name) == long_names.end()) {
      long_names.emplace_back(std::move(opt_name));
    }
  }
  return ret;
}

inline std::pair<std::string, std::string> help_row(
    std::vector<char> short_names, std::vector<std::string> long_names,
    std::string value_placeholder, std::string description) {
  std::pair<std::string, std::string> ret;
  std::string& left = get<0>(ret);
  std::string& right = get<1>(ret);
  right = description;

  if (!short_names.empty()) {
    left.append(1, '-');
    left.append(1, short_names[0]);
  }

  if (!long_names.empty()) {
    if (!left.empty()) {
      left.append(", ");
    }
    left.append(2, '-');
    left.append(long_names[0]);
  }

  if (!value_placeholder.empty()) {
    left.append(1, ' ');
    left.append(value_placeholder);
  }

  return ret;
}

class FlagSchema {
 public:
  FlagSchema(std::vector<char> short_names, std::vector<std::string> long_names,
             std::string description)
      : short_names_(std::move(short_names)),
        long_names_(std::move(long_names)),
        description_(std::move(description)) {}
  void negatable(bool v) { negatable_ = v; }
  bool is_negatable() { return negatable_; }

  bool has_short_name(char name) const {
    return std::find(short_names_.begin(), short_names_.end(), name) !=
           short_names_.end();
  }
  bool has_long_name(std::string const& name) const {
    return std::find(long_names_.begin(), long_names_.end(), name) !=
           long_names_.end();
  }

  std::pair<std::string, std::string> help_row() const {
    return argparse::help_row(short_names_, long_names_, "", description_);
  }

 private:
  std::vector<char> short_names_;
  std::vector<std::string> long_names_;
  std::string description_;
  bool negatable_{false};
  [[maybe_unused]] bool hidden_{false};
};

class OptionSchema {
 public:
  OptionSchema(std::vector<char> short_names,
               std::vector<std::string> long_names, std::string description)
      : short_names_(std::move(short_names)),
        long_names_(std::move(long_names)),
        description_(std::move(description)) {}
  void negatable(bool v) { negatable_ = v; }
  bool is_negatable() const { return negatable_; }

  bool has_short_name(char name) const {
    return std::find(short_names_.begin(), short_names_.end(), name) !=
           short_names_.end();
  }
  bool has_long_name(std::string const& name) const {
    return std::find(long_names_.begin(), long_names_.end(), name) !=
           long_names_.end();
  }

  std::pair<std::string, std::string> help_row() const {
    return argparse::help_row(short_names_, long_names_, value_placeholder_,
                              description_);
  }

 private:
  std::vector<char> short_names_;
  std::vector<std::string> long_names_;
  std::string description_;
  std::string value_placeholder_;
  bool negatable_{false};
  [[maybe_unused]] bool hidden_{false};
};

class PositionalSchema {
 public:
  PositionalSchema(std::string name, std::size_t size, std::string description)
      : name_(std::move(name)),
        size_(size),
        description_(std::move(description)) {}
  std::string const& name() const { return name_; }

  std::string usage() const {
    std::stringstream ret;
    ret << name_ << "\t" << description_;
    return ret.str();
  }
  std::pair<std::string, std::string> help_row() const {
    return std::pair<std::string, std::string>{name_, description_};
  }

 private:
  std::string name_;
  [[maybe_unused]] std::size_t size_;
  std::string description_;
  [[maybe_unused]] bool hidden_{false};
};

class CommandSchema {
 public:
  CommandSchema(std::string name, std::string description,
                CommandSchema* parent)
      : name_(std::move(name)),
        description_(std::move(description)),
        parent_(parent) {}

  std::string const& name() const { return name_; }

  FlagSchema& add_flag(std::vector<char> short_names,
                       std::vector<std::string> long_names,
                       std::string description) {
    flags_.emplace_back(std::make_shared<FlagSchema>(
        std::move(short_names), std::move(long_names), std::move(description)));
    return *flags_.back();
  }

  OptionSchema& add_option(std::vector<char> short_names,
                           std::vector<std::string> long_names,
                           std::string description) {
    options_.emplace_back(std::make_shared<OptionSchema>(
        std::move(short_names), std::move(long_names), std::move(description)));
    return *options_.back();
  }

  PositionalSchema& add_positional(std::string name, std::size_t size,
                                   std::string description) {
    positionals_.emplace_back(std::make_shared<PositionalSchema>(
        std::move(name), size, std::move(description)));
    return *positionals_.back();
  }

  CommandSchema& add_subcommand(std::string name, std::string description) {
    subcommands_.emplace_back(std::make_shared<CommandSchema>(
        std::move(name), std::move(description), this));
    return *subcommands_.back();
  }
  CommandSchema& add_subcommand(std::shared_ptr<CommandSchema> subcommand) {
    subcommands_.emplace_back(std::move(subcommand));
    return *subcommands_.back();
  }

  bool has_flag(std::string const& name) const {
    for (auto&& flag : flags_) {
      if (flag->has_long_name(name)) {
        return true;
      }
    }
    return false;
  }
  bool has_flag(char name) const {
    for (auto&& flag : flags_) {
      if (flag->has_short_name(name)) {
        return true;
      }
    }
    return false;
  }

  bool has_option(std::string const& name) const {
    for (auto&& option : options_) {
      if (option->has_long_name(name)) {
        return true;
      }
    }
    return false;
  }
  bool has_option(char name) const {
    for (auto&& option : options_) {
      if (option->has_short_name(name)) {
        return true;
      }
    }
    return false;
  }

  bool has_positional(std::string const& name) const {
    for (auto&& positional : positionals_) {
      if (positional->name() == name) {
        return true;
      }
    }
    return false;
  }

  bool has_subcommand(std::string const& name) const {
    for (auto&& subcommand : subcommands_) {
      if (subcommand->name_ == name) {
        return true;
      }
    }
    return false;
  }

  std::string usage(size_t max_left_width = 32, size_t max_right_width = 80) {
    {
      size_t max_width = 0;
      for (auto&& flag : flags_) {
        max_width = std::max(max_width, flag->help_row().first.size());
      }
      for (auto&& option : options_) {
        max_width = std::max(max_width, option->help_row().first.size());
      }
      for (auto&& positional : positionals_) {
        max_width = std::max(max_width, positional->help_row().first.size());
      }
      max_left_width = std::min(max_width, max_left_width);
    }

    CommandSchema* parent = parent_;
    std::string command_str;
    while (parent != nullptr) {
      command_str.insert(0, parent->name_ + " ");
      parent = parent->parent_;
    }
    std::stringstream out;
    out << "Usage:\n";
    out << "  " << command_str << name_ << "\n";
    out << "\nDescription:\n";
    out << description_ << "\n";
    out << "\nArguments:\n";
    for (auto&& positional : positionals_) {
      auto [left, right] = positional->help_row();
      format_print(out, left, max_left_width, right, max_right_width);
    }
    out << "\nOptions:\n";
    for (auto&& flag : flags_) {
      auto [left, right] = flag->help_row();
      format_print(out, left, max_left_width, right, max_right_width);
    }
    for (auto&& option : options_) {
      auto [left, right] = option->help_row();
      format_print(out, left, max_left_width, right, max_right_width);
    }
    out << "\nCommands:\n";
    for (auto&& subcommand : subcommands_) {
      format_print(out, subcommand->name_, max_left_width,
                   subcommand->description_, max_right_width);
    }
    return out.str();
  }

  void format_print(std::stringstream& out, std::string left, size_t left_width,
                    std::string right, size_t right_width) {
    right = detail::word_wrap(right, right_width);
    auto pos = right.find('\n');
    while (pos != std::string::npos) {
      right.replace(pos, 1, "\n" + std::string(left_width + 2, ' '));
      pos = right.find('\n', pos + left_width + 2);
    }
    out << "  " << std::left << std::setfill(' ') << std::setw(left_width)
        << left << "  " << right << "\n";
  }

 private:
  std::string name_;
  std::string description_;
  [[maybe_unused]] bool hidden_{false};
  [[maybe_unused]] bool treat_remaining_as_positional_{false};
  std::vector<std::shared_ptr<FlagSchema>> flags_;
  std::vector<std::shared_ptr<OptionSchema>> options_;
  std::vector<std::shared_ptr<PositionalSchema>> positionals_;
  CommandSchema* parent_{nullptr};
  std::vector<std::shared_ptr<CommandSchema>> subcommands_;
};

class ArgBase {
  friend class Command;
  friend class ArgParser;

 public:
  ArgBase(const std::string& name, const std::string& description)
      : description_(description) {
    auto ret = parse_option_name(name);

    if (ret) {
      std::transform(std::get<0>(*ret).begin(), std::get<0>(*ret).end(),
                     std::back_inserter(short_opt_names_),
                     [](char c) { return std::string(1, c); });
      std::copy(std::get<1>(*ret).begin(), std::get<1>(*ret).end(),
                std::back_inserter(long_opt_names_));
    } else {
      detail::report_invalid_argument(ret.error());
    }
  }
  size_t count() const { return count_; }
  virtual ~ArgBase() = default;

  ArgBase& hidden(bool v = true) {
    hidden_ = v;
    return *this;
  }

  ArgBase& env(std::string const& env) {
    env_key_ = env;
    return *this;
  }

 protected:
  virtual bool is_flag() const = 0;
  virtual bool is_option() const = 0;
  virtual bool is_positional() const = 0;
  virtual std::string usage() const = 0;
  const std::string& description() const { return description_; }
  size_t count_{0};
  std::vector<std::string> short_opt_names_;
  std::vector<std::string> long_opt_names_;
  std::string description_;
  std::string env_key_;
  bool hidden_{false};
};

class FlagBase : public ArgBase {
  friend class Command;
  friend class ArgParser;

 public:
  FlagBase(const std::string& name, const std::string& description)
      : ArgBase(name, description) {}

  void negatable(bool v = true) { this->negatable_ = v; }
  bool is_negatable() { return this->negatable_; }

 protected:
  bool is_flag() const override final { return true; }
  bool is_option() const override final { return false; }
  bool is_positional() const override final { return false; }
  virtual void parse() = 0;
  virtual void parse_negated() = 0;
  bool negatable_ = false;
  std::string usage() const override {
    std::stringstream opt_str;
    if (short_opt_names_.empty()) {
      if (!long_opt_names_.empty()) {
        opt_str << "  --" << (negatable_ ? "[no-]" : "") << long_opt_names_[0];
      } else {
        opt_str << "  ";
      }
    } else {
      if (negatable_ && long_opt_names_.empty()) {
        opt_str << "[--no]-" << short_opt_names_[0];
      } else {
        opt_str << "-" << short_opt_names_[0];
      }
      if (!long_opt_names_.empty()) {
        opt_str << ", --" << (negatable_ ? "[no-]" : "") << long_opt_names_[0];
      }
    }
    std::string extra_desc;
    {
      std::vector<std::string> aliases;
      for (size_t i = 1; i < short_opt_names_.size(); ++i) {
        aliases.push_back(short_opt_names_[i]);
      }
      for (size_t i = 1; i < long_opt_names_.size(); ++i) {
        aliases.push_back(long_opt_names_[i]);
      }
      if (!aliases.empty()) {
        extra_desc += " (aliases: " + detail::join(aliases, ", ") + ")";
      }
    }
    return detail::format(opt_str.str(), description() + extra_desc, " ");
  }
};

template <typename T>
concept bindable_flag =
    std::is_same_v<T, bool> || std::is_same_v<T, std::optional<bool>> ||
    std::is_same_v<T, int> || std::is_same_v<T, std::optional<int>>;
template <typename T>
concept bindable_int_flag =
    std::is_same_v<T, int> || std::is_same_v<T, std::optional<int>>;
template <typename T>
concept bindable_bool_flag =
    std::is_same_v<T, bool> || std::is_same_v<T, std::optional<bool>>;

template <bindable_flag T = bool>
class Flag final : public FlagBase {
  friend class Command;
  friend class ArgParser;

 public:
  using value_type = std::conditional_t<detail::is_optional_v<T>,
                                        detail::extract_value_type_t<T>, T>;
  using parsed_value_type = detail::extract_value_type_t<T>;

 public:
  Flag(const std::string& name, const std::string& description, T& bind_value)
      : FlagBase(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_{store_true},
        negated_parse_action_{store_false} {}
  Flag(const std::string& name, const std::string& description, T& bind_value,
       std::function<void(parsed_value_type&)> action,
       std::function<void(parsed_value_type&)> negated_action)
      : FlagBase(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_(std::move(action)),
        negated_parse_action_(std::move(negated_action)) {}

  Flag<T>& callback(std::function<void(parsed_value_type)> cb) {
    callback_ = std::move(cb);
    return *this;
  }

  T const& value() const { return bind_value_; }

 protected:
  void parse() override {
    if constexpr (detail::is_optional_v<T>) {
      auto& flag_value = bind_value_.get();
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
  void parse_negated() override {
    if constexpr (detail::is_optional_v<T>) {
      auto& flag_value = bind_value_.get();
      if (!flag_value.has_value()) {
        flag_value = typename T::value_type{};
      }
      negated_parse_action_(flag_value.value());
      if (callback_) {
        callback_(flag_value.value());
      }
    } else {
      negated_parse_action_(bind_value_.get());
      if (callback_) {
        callback_(bind_value_.get());
      }
    }
    count_++;
  }

 private:
  std::reference_wrapper<T> bind_value_;
  std::function<void(parsed_value_type&)> parse_function_;
  std::function<void(parsed_value_type&)> negated_parse_action_;
  std::function<void(parsed_value_type)> callback_;
};

class OptionBase : public ArgBase {
  friend class Command;
  friend class ArgParser;
  friend class OptionAlias;
  friend std::vector<token> tokenize(argv_stream& args, Command& cmd);

 public:
  OptionBase(const std::string& name, const std::string& description)
      : ArgBase(name, description) {}

  void required() { is_required_ = true; }
  bool is_required() const { return is_required_; }

 protected:
  bool is_flag() const override final { return false; }
  virtual void parse(const std::string& opt_value) {
    for (const auto& validator : pre_parse_validators_) {
      if (auto [ok, err_msg] = validator(opt_value); !ok) {
        std::string msg = "Validation failed: ";
        msg += detail::join(long_opt_names_, ',');
        if (!long_opt_names_.empty() && !short_opt_names_.empty()) {
          msg += ", ";
        }
        msg += detail::join(short_opt_names_, ',');
        msg += ": `";
        msg += opt_value;
        msg += "` is an invalid value. ";
        detail::report_invalid_argument(msg + err_msg);
      }
    }
    if (!choices_.empty()) {
      if (choices_.find(opt_value) == choices_.end()) {
        std::string msg = "Invalid choice: ";
        msg += detail::join(long_opt_names_, ',');
        if (!long_opt_names_.empty() && !short_opt_names_.empty()) {
          msg += ", ";
        }
        msg += detail::join(short_opt_names_, ',');
        msg += ": `";
        msg += opt_value;
        msg += "` is an invalid value. Valid choices are: ";
        std::vector<std::string> keys;
        std::transform(choices_.begin(), choices_.end(), back_inserter(keys),
                       [](auto const& pair) { return pair.first; });
        msg += detail::join(keys, ',');
        detail::report_invalid_argument(msg);
      }
    }
    this->opt_values.push_back(opt_value);
    count_++;
  }

  virtual bool is_multiple() const = 0;
  virtual void use_default_if_needed() = 0;
  virtual std::optional<std::string> get_default_value() const = 0;

  template <typename T>
  void set_value_placeholder_for_type() {
    if constexpr (detail::is_optional_v<T>) {
      set_value_placeholder_for_type<typename T::value_type>();
    } else {
      if constexpr (std::is_integral_v<T>) {
        value_placeholder_ = "<N>";
      } else if constexpr (std::is_floating_point_v<T>) {
        value_placeholder_ = "<x.y>";
      } else {
        value_placeholder_ = "<arg>";
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
    std::string extra_desc;
    if (!this->choices_.empty()) {
      std::vector<std::string> choice_strs;
      for (auto const& [value, help] : this->choices_) {
        choice_strs.push_back("[" + value + "] " + help);
      }
      extra_desc += " (";
      extra_desc += detail::join(choice_strs, ", ");
      extra_desc += ")";
    }
    if (auto default_value = get_default_value(); default_value.has_value()) {
      extra_desc += " (default: " + *default_value;
      if (!env_key_.empty()) {
        extra_desc += ", ENV:" + env_key_;
      }
      extra_desc += ")";
    }
    if (is_option()) {
      std::vector<std::string> aliases;
      for (size_t i = 1; i < short_opt_names_.size(); ++i) {
        aliases.push_back(short_opt_names_[i]);
      }
      for (size_t i = 1; i < long_opt_names_.size(); ++i) {
        aliases.push_back(long_opt_names_[i]);
      }
      if (!aliases.empty()) {
        extra_desc += " (aliases: " + detail::join(aliases, ", ") + ")";
      }
    }

    std::stringstream opt_str;
    if (is_option()) {
      if (short_opt_names_.empty()) {
        opt_str << "  --" << long_opt_names_[0] << " " << value_placeholder_;
      } else {
        opt_str << "-" << short_opt_names_[0];
        if (!long_opt_names_.empty()) {
          opt_str << ", --" << long_opt_names_[0];
        }
        opt_str << " " << value_placeholder_;
      }
    } else {
      opt_str << long_opt_names_[0];
    }
    return detail::format(opt_str.str(), description() + extra_desc, " ");
  }
  bool is_required_{false};
  std::string value_placeholder_;
  std::vector<std::string> opt_values;
  std::vector<std::function<std::pair<bool, std::string>(std::string const&)>>
      pre_parse_validators_;
  std::map<std::string, std::string> choices_;
};

template <typename Derived>
class OptionBaseCRTP : public OptionBase {
 public:
  using OptionBase::OptionBase;
  Derived& validator(
      std::function<std::pair<bool, std::string>(std::string const&)> f) {
    pre_parse_validators_.push_back(std::move(f));

    return static_cast<Derived&>(*this);
  }

  Derived& choices(
      std::map<std::string, std::string> const& choices_description) {
    this->choices_ = choices_description;
    return static_cast<Derived&>(*this);
  }

  template <typename T>
  Derived& choices(std::initializer_list<T> const& choices)
    requires(std::is_same_v<T, std::string> ||
             std::is_same_v<T, std::string_view> ||
             std::is_same_v<T, const char*>)
  {
    std::map<std::string, std::string> choices_description;
    for (auto const& choice : choices) {
      choices_description[choice] = "";
    }
    this->choices_ = choices_description;
    return static_cast<Derived&>(*this);
  }

  Derived& value_placeholder(std::string const& value_placeholder) {
    bool add_parentheses = true;
    if (!value_placeholder.empty() && (value_placeholder.starts_with('<') ||
                                       value_placeholder.starts_with('['))) {
      add_parentheses = false;
    }
    if (add_parentheses) {
      this->value_placeholder_ = "<" + value_placeholder + ">";
    } else {
      this->value_placeholder_ = value_placeholder;
    }
    return static_cast<Derived&>(*this);
  }

  Derived& hidden(bool v = true) {
    ArgBase::hidden(v);
    return static_cast<Derived&>(*this);
  }
  Derived& env(std::string const& env) {
    ArgBase::env(env);
    return static_cast<Derived&>(*this);
  }
  Derived& required() {
    OptionBase::required();
    return static_cast<Derived&>(*this);
  }
};

template <detail::bindable T>
class Option final : public OptionBaseCRTP<Option<T>> {
  friend class Command;
  friend class ArgParser;

 public:
  using value_type = std::conditional_t<detail::is_optional_v<T>,
                                        detail::extract_value_type_t<T>, T>;
  using parsed_value_type = detail::extract_value_type_t<T>;

 public:
  Option(const std::string& name, const std::string& description, T& bind_value)
    requires requires {
      {
        from_string<parsed_value_type>(std::declval<std::string>())
      } -> std::same_as<expected<parsed_value_type, std::string>>;
    }
      : OptionBaseCRTP<Option<T>>(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_([](std::string const& opt_value) {
          return from_string<parsed_value_type>(opt_value).value();
        }) {
    OptionBase::set_value_placeholder_for_type<T>();
  }
  Option(const std::string& name, const std::string& description, T& bind_value,
         char delim)
    requires requires {
      {
        from_string<parsed_value_type>(std::declval<std::string>(),
                                       std::declval<char>())
      } -> std::same_as<expected<parsed_value_type, std::string>>;
    }
      : OptionBaseCRTP<Option<T>>(name, description),
        bind_value_(std::ref(bind_value)),
        parse_function_([delim](std::string const& opt_value) {
          return from_string<parsed_value_type>(opt_value, delim).value();
        }) {
    OptionBase::set_value_placeholder_for_type<T>();
  }
  Option<T>& default_value(const std::string& default_value)
    requires(!detail::from_string_container<T>)
  {
    this->default_value_ = default_value;
    return *this;
  }

  Option<T>& default_value(std::vector<std::string> const& default_value)
    requires detail::from_string_container<T>
  {
    this->default_value_ = default_value;
    return *this;
  }

  Option<T>& callback(std::function<void(value_type const&)> cb) {
    callback_ = std::move(cb);
    return *this;
  }

  using OptionBaseCRTP<Option<T>>::validator;
  Option<T>& validator(
      std::function<std::pair<bool, std::string>(const parsed_value_type&)>
          validator_fn) {
    value_validators_.push_back(std::move(validator_fn));
    return *this;
  }

  Option<T>& range(parsed_value_type r_min, parsed_value_type r_max)
    requires std::is_arithmetic_v<parsed_value_type>
  {
    value_validators_.push_back([r_min, r_max](const parsed_value_type& val) {
      using return_type = std::pair<bool, std::string>;
      auto ok = r_min <= val && val <= r_max;
      if (!ok) {
        std::string err_msg = "not in range: [" + argparse::to_string(r_min) +
                              " - " + argparse::to_string(r_max) + "]";
        return return_type{ok, err_msg};
      } else {
        return return_type{ok, ""};
      }
    });
    return *this;
  }

  using OptionBaseCRTP<Option<T>>::choices;
  Option<T>& choices(
      std::map<parsed_value_type, std::string> const& value_and_helpers)
    requires(!std::is_same_v<parsed_value_type, std::string>)
  {
    value_choices_ = value_and_helpers;
    return *this;
  }

  T const& value() const { return bind_value_; }

 protected:
  bool is_option() const override final { return true; }
  bool is_positional() const override final { return false; }
  void parse(const std::string& opt_value) override {
    OptionBaseCRTP<Option<T>>::parse(opt_value);
    auto parsed_value = parse_function_(opt_value);
    for (const auto& validator : value_validators_) {
      if (auto [ok, err_msg] = validator(parsed_value); !ok) {
        std::string msg = "Validation failed: ";
        msg += detail::join(ArgBase::long_opt_names_, ',');
        if (!ArgBase::long_opt_names_.empty() &&
            !ArgBase::short_opt_names_.empty()) {
          msg += ", ";
        }
        msg += detail::join(ArgBase::short_opt_names_, ',');
        msg += ": `";
        msg += opt_value;
        msg += "` is an invalid value. ";
        detail::report_invalid_argument(msg + err_msg);
      }
    }
    if constexpr (requires(parsed_value_type a, parsed_value_type b) {
                    a < b;
                  }) {
      if (!value_choices_.empty() &&
          (value_choices_.find(parsed_value) == value_choices_.end())) {
        std::string msg = "Invalid choice: ";
        msg += detail::join(ArgBase::long_opt_names_, ',');
        if (!ArgBase::long_opt_names_.empty() &&
            !ArgBase::short_opt_names_.empty()) {
          msg += ", ";
        }
        msg += detail::join(ArgBase::short_opt_names_, ',');
        msg += ": `";
        msg += opt_value;
        msg += "` is an invalid value. Valid choices are: ";

        if constexpr (detail::has_to_string<parsed_value_type> ||
                      std::is_constructible_v<std::string, parsed_value_type>) {
          std::vector<std::string> keys;
          std::transform(
              value_choices_.begin(), value_choices_.end(),
              std::back_inserter(keys), [](auto const& pair) {
                if constexpr (detail::has_to_string<parsed_value_type>) {
                  return argparse::to_string(pair.first);
                } else {
                  return pair.first;
                }
              });
          msg += detail::join(keys, ',');
        }
        detail::report_invalid_argument(msg);
      }
    }
    if constexpr (detail::from_string_container<T>) {
      detail::push_back(bind_value_.get(), std::move(parsed_value));
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
    if constexpr (detail::from_string_container<T>) {
      return true;
    } else {
      return false;
    }
  }
  void use_default_if_needed() override {
    if (ArgBase::count() != 0) {
      return;
    }
    if constexpr (detail::from_string_container<T>) {
      if (default_value_.has_value()) {
        for (const auto& value : default_value_.value()) {
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
    if constexpr (detail::from_string_container<T>) {
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
  std::function<parsed_value_type(std::string const&)> parse_function_;
  std::conditional_t<detail::from_string_container<T>,
                     std::optional<std::vector<std::string>>,
                     std::optional<std::string>>
      default_value_;
  std::function<void(value_type const&)> callback_;
  std::vector<
      std::function<std::pair<bool, std::string>(parsed_value_type const&)>>
      value_validators_;
  std::map<parsed_value_type, std::string> value_choices_;
};

class OptionAlias : public FlagBase {
 public:
  OptionAlias(std::string const& name, OptionBase* option,
              std::string const& opt_value)
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

  void parse_negated() override {}

 private:
  OptionBase* option_{nullptr};
  std::string opt_value_{};
};

template <detail::bindable T>
class Positional final : public OptionBaseCRTP<Positional<T>> {
  friend class Command;
  friend class ArgParser;

 public:
  using value_type = std::conditional_t<detail::is_optional_v<T>,
                                        detail::extract_value_type_t<T>, T>;
  using parsed_value_type = detail::extract_value_type_t<T>;

 public:
  Positional(const std::string& name, const std::string& description,
             T& bind_value)
    requires requires {
      {
        from_string<parsed_value_type>(std::declval<std::string>())
      } -> std::same_as<expected<parsed_value_type, std::string>>;
    }
      : OptionBaseCRTP<Positional<T>>(name, description),
        bind_value_(std::ref(bind_value)) {
    if constexpr (detail::from_string_container<T>) {
      parse_function_ = [](std::string const& opt_value) {
        return from_string<parsed_value_type>(opt_value).value();
      };
    } else if constexpr (detail::is_optional_v<T>) {
      parse_function_ = [](std::string const& opt_value) {
        return from_string<parsed_value_type>(opt_value).value();
      };
    } else {
      parse_function_ = [](std::string const& opt_value) {
        return from_string<T>(opt_value).value();
      };
    }
    (void)OptionBaseCRTP<Positional<T>>::value_placeholder(name);
  }
  Positional(const std::string& name, const std::string& description,
             T& bind_value, char delim)
    requires requires {
      {
        from_string<parsed_value_type>(std::declval<std::string>(),
                                       std::declval<char>())

      } -> std::same_as<expected<parsed_value_type, std::string>>;
    }
      : OptionBaseCRTP<Positional<T>>(name, description),
        bind_value_(std::ref(bind_value)) {
    if constexpr (detail::from_string_container<T>) {
      parse_function_ = [delim](std::string const& opt_value) {
        return from_string<parsed_value_type>(opt_value, delim).value();
      };
    } else if constexpr (detail::is_optional_v<T>) {
      parse_function_ = [delim](std::string const& opt_value) {
        return from_string<parsed_value_type>(opt_value, delim).value();
      };
    } else {
      parse_function_ = [delim](std::string const& opt_value) {
        return from_string<T>(opt_value, delim).value();
      };
    }
    (void)OptionBaseCRTP<Positional<T>>::value_placeholder(name);
  }
  Positional<T>& default_value(const std::string& default_value)
    requires(!detail::from_string_container<T>)
  {
    this->default_value_ = default_value;
    return *this;
  }
  Positional<T>& default_value(std::vector<std::string> const& default_value)
    requires detail::from_string_container<T>
  {
    this->default_value_ = default_value;
    return *this;
  }

  Positional<T>& callback(std::function<void(value_type const&)> cb) {
    callback_ = std::move(cb);
    return *this;
  }

  T const& value() const { return bind_value_; }

  using OptionBaseCRTP<Positional<T>>::validator;
  Positional<T>& validator(
      std::function<std::pair<bool, std::string>(const parsed_value_type&)>
          validator_fn) {
    value_validators_.push_back(std::move(validator_fn));
    return *this;
  }

  Positional<T>& range(parsed_value_type r_min, parsed_value_type r_max)
    requires std::is_arithmetic_v<parsed_value_type>
  {
    value_validators_.push_back([r_min, r_max](const parsed_value_type& val) {
      using return_type = std::pair<bool, std::string>;
      auto ok = r_min <= val && val <= r_max;
      if (!ok) {
        std::string err_msg = "not in range: [" + argparse::to_string(r_min) +
                              " - " + argparse::to_string(r_max) + "]";
        return return_type{ok, err_msg};
      } else {
        return return_type{ok, ""};
      }
    });
    return *this;
  }

  using OptionBaseCRTP<Positional<T>>::choices;
  Positional<T>& choices(
      std::map<parsed_value_type, std::string> const& value_and_helpers)
    requires(!std::is_same_v<parsed_value_type, std::string>)
  {
    value_choices_ = value_and_helpers;
    return *this;
  }

 protected:
  bool is_option() const override final { return false; }
  bool is_positional() const override final { return true; }
  void parse(const std::string& opt_value) override {
    OptionBaseCRTP<Positional<T>>::parse(opt_value);
    auto parsed_value = parse_function_(opt_value);
    for (const auto& validator : value_validators_) {
      if (auto [ok, err_msg] = validator(parsed_value); !ok) {
        std::string msg = "Validation failed: ";
        msg += detail::join(ArgBase::long_opt_names_, ',');
        if (!ArgBase::long_opt_names_.empty() &&
            !ArgBase::short_opt_names_.empty()) {
          msg += ", ";
        }
        msg += detail::join(ArgBase::short_opt_names_, ',');
        msg += ": `";
        msg += opt_value;
        msg += "` is an invalid value. ";
        detail::report_invalid_argument(msg + err_msg);
      }
    }
    if constexpr (requires(parsed_value_type a, parsed_value_type b) {
                    a < b;
                  }) {
      if (!value_choices_.empty() &&
          (value_choices_.find(parsed_value) == value_choices_.end())) {
        std::string msg = "Invalid choice: ";
        msg += detail::join(ArgBase::long_opt_names_, ',');
        if (!ArgBase::long_opt_names_.empty() &&
            !ArgBase::short_opt_names_.empty()) {
          msg += ", ";
        }
        msg += detail::join(ArgBase::short_opt_names_, ',');
        msg += ": `";
        msg += opt_value;
        msg += "` is an invalid value. Valid choices are: ";

        if constexpr (std::is_same_v<parsed_value_type, std::string> ||
                      detail::has_to_string<parsed_value_type> ||
                      std::is_constructible_v<std::string, parsed_value_type>) {
          std::vector<std::string> keys;
          std::transform(
              value_choices_.begin(), value_choices_.end(),
              std::back_inserter(keys), [](auto const& pair) {
                if constexpr (detail::has_to_string<parsed_value_type>) {
                  return argparse::to_string(pair.first);
                } else {
                  return pair.first;
                }
              });
          msg += detail::join(keys, ',');
        }
        detail::report_invalid_argument(msg);
      }
    }
    if constexpr (detail::from_string_container<T>) {
      detail::push_back(bind_value_.get(), std::move(parsed_value));
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
    if constexpr (detail::from_string_container<T>) {
      return true;
    } else {
      return false;
    }
  }
  void use_default_if_needed() override {
    if (ArgBase::count() != 0) {
      return;
    }
    if constexpr (detail::from_string_container<T>) {
      if (default_value_.has_value()) {
        for (const auto& value : default_value_.value()) {
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
    if constexpr (detail::from_string_container<T>) {
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
  std::function<parsed_value_type(std::string const&)> parse_function_;
  std::conditional_t<detail::from_string_container<T>,
                     std::optional<std::vector<std::string>>,
                     std::optional<std::string>>
      default_value_;
  std::function<void(value_type const&)> callback_;
  std::vector<
      std::function<std::pair<bool, std::string>(parsed_value_type const&)>>
      value_validators_;
  std::map<parsed_value_type, std::string> value_choices_;
};

struct token;
class argv_stream;
class Command {
  friend class ArgParser;

  friend inline std::vector<token> tokenize(argv_stream& args, Command& cmd);

  template <typename T>
    requires std::same_as<T, bool> || std::same_as<std::optional<bool>, T>
  Flag<T>& add_flag_bool(
      const std::string& name, const std::string& description, T& bind_value,
      std::function<void(detail::extract_value_type_t<T>&)> action = store_true,
      std::function<void(detail::extract_value_type_t<T>&)> negated_action =
          store_false) {
    auto flag =
        std::make_unique<Flag<T>>(name, description, bind_value,
                                  std::move(action), std::move(negated_action));
    auto& ret = *(flag.get());
    if (flag_or_option_exists(ret)) {
      detail::die("Flag or option already exists: " + name);
    }

    auto option_names = parse_option_name(name);
    if (option_names) {
      command_schema_.add_flag(get<0>(*option_names), get<1>(*option_names),
                               ret.description_);
    }

    args_.push_back(std::move(flag));
    return ret;
  }
  template <bindable_int_flag T>
  Flag<T>& add_flag_int(
      const std::string& name, const std::string& description, T& bind_value,
      std::function<void(detail::extract_value_type_t<T>&)> action =
          increment<detail::extract_value_type_t<T>>,
      std::function<void(detail::extract_value_type_t<T>&)> negated_action =
          decrement<detail::extract_value_type_t<T>>) {
    auto flag =
        std::make_unique<Flag<T>>(name, description, bind_value,
                                  std::move(action), std::move(negated_action));
    auto& ret = *(flag.get());
    if (flag_or_option_exists(ret)) {
      detail::die("Flag or option already exists: " + name);
    }
    auto option_names = parse_option_name(name);
    if (option_names) {
      command_schema_.add_flag(get<0>(*option_names), get<1>(*option_names),
                               ret.description_);
    }
    args_.push_back(std::move(flag));
    return ret;
  }

 public:
  Command(std::string cmd, std::string description)
      : command_{std::move(cmd)},
        description_(std::move(description)),
        command_schema_{command_, description_, nullptr} {}
  virtual ~Command() {}

 protected:
  Command(Command&&) = default;
  Command& operator=(Command&&) = default;

 public:
  template <bindable_bool_flag T>
  Flag<T>& add_flag(const std::string& name, const std::string& description,
                    T& bind_value) {
    std::function<void(detail::extract_value_type_t<T>&)> action = store_true;
    std::function<void(detail::extract_value_type_t<T>&)> negated_action =
        store_false;
    return add_flag_bool(name, description, bind_value, std::move(action),
                         std::move(negated_action));
  }
  template <bindable_int_flag T>
  Flag<T>& add_flag(const std::string& name, const std::string& description,
                    T& bind_value) {
    std::function<void(detail::extract_value_type_t<T>&)> action =
        increment<detail::extract_value_type_t<T>>;
    std::function<void(detail::extract_value_type_t<T>&)> negated_action =
        decrement<detail::extract_value_type_t<T>>;
    return add_flag_int(name, description, bind_value, std::move(action),
                        std::move(negated_action));
  }

  template <bindable_bool_flag T>
  Flag<T>& add_negative_flag(const std::string& name,
                             const std::string& description, T& bind_value) {
    std::function<void(detail::extract_value_type_t<T>&)> action = store_false;
    std::function<void(detail::extract_value_type_t<T>&)> negated_action =
        store_true;
    return add_flag_bool(name, description, bind_value, std::move(action),
                         std::move(negated_action));
  }
  template <bindable_int_flag T>
  Flag<T>& add_negative_flag(const std::string& name,
                             const std::string& description, T& bind_value) {
    std::function<void(detail::extract_value_type_t<T>&)> action =
        decrement<detail::extract_value_type_t<T>>;
    std::function<void(detail::extract_value_type_t<T>&)> negated_action =
        increment<detail::extract_value_type_t<T>>;
    return add_flag_int(name, description, bind_value, std::move(action),
                        std::move(negated_action));
  }

  FlagBase& add_alias(const std::string& name, const std::string& opt_name,
                      const std::string& opt_value) {
    auto* opt = get(opt_name);
    if (!opt) {
      detail::die("Option not found: " + opt_name);
    }
    if (!opt->is_option()) {
      detail::die("Not an option: " + opt_name);
    }

    auto alias =
        std::make_unique<OptionAlias>(name, (OptionBase*)opt, opt_value);

    auto& ret = *(alias.get());
    if (flag_or_option_exists(ret)) {
      detail::die("Flag or option already exists: " + name);
    }
    auto option_names = parse_option_name(name);
    if (option_names) {
      command_schema_.add_flag(get<0>(*option_names), get<1>(*option_names),
                               ret.description_);
    }
    args_.push_back(std::move(alias));
    return ret;
  }

  template <detail::bindable_without_delim T>
  Option<T>& add_option(const std::string& name, const std::string& description,
                        T& bind_value) {
    auto option = std::make_unique<Option<T>>(name, description, bind_value);
    auto& ret = *(option.get());
    if (flag_or_option_exists(ret)) {
      detail::die("Flag or option already exists: " + name);
    }
    auto option_names = parse_option_name(name);
    if (option_names) {
      command_schema_.add_option(get<0>(*option_names), get<1>(*option_names),
                                 ret.description_);
    }
    args_.push_back(std::move(option));
    return ret;
  }

  template <detail::bindable_with_delim T>
  Option<T>& add_option(const std::string& name, const std::string& description,
                        T& bind_value, char delim) {
    auto option =
        std::make_unique<Option<T>>(name, description, bind_value, delim);
    auto& ret = *(option.get());
    if (flag_or_option_exists(ret)) {
      detail::die("Flag or option already exists: " + name);
    }
    auto option_names = parse_option_name(name);
    if (option_names) {
      command_schema_.add_option(get<0>(*option_names), get<1>(*option_names),
                                 ret.description_);
    }
    args_.push_back(std::move(option));
    return ret;
  }

  template <detail::bindable_without_delim T>
  Positional<T>& add_positional(const std::string& name,
                                const std::string& description, T& bind_value) {
    if (!subcommands_.empty()) {
      detail::die("Cannot add positional argument when subcommands exist");
    }
    if (std::ranges::find_if(args_, [](const auto& arg) {
          return arg->is_positional() &&
                 dynamic_cast<OptionBase*>(arg.get())->is_multiple();
        }) != args_.end()) {
      detail::die(
          "Only one container positional argument is supported, "
          "and it must be the last one.");
    }
    auto positional =
        std::make_unique<Positional<T>>(name, description, bind_value);
    auto& ret = *(positional.get());
    if (positional_exists(ret)) {
      detail::die("Positional already exists: " + name);
    }
    size_t size = 1;
    if constexpr (detail::from_string_container<T>) {
      size = 0;
    }
    command_schema_.add_positional(name, size, description);
    args_.push_back(std::move(positional));
    return ret;
  }

  template <detail::bindable_with_delim T>
  Positional<T>& add_positional(const std::string& name,
                                const std::string& description, T& bind_value,
                                char delim) {
    if (!subcommands_.empty()) {
      detail::die("Cannot add positional argument when subcommands exist");
    }
    auto positional =
        std::make_unique<Positional<T>>(name, description, bind_value, delim);
    auto& ret = *(positional.get());
    if (positional_exists(ret)) {
      detail::die("Positional already exists: " + name);
    }
    size_t size = 1;
    if constexpr (detail::from_string_container<T>) {
      size = 0;
    } else {
      size = std::tuple_size_v<T>;
    }
    command_schema_.add_positional(name, size, description);
    args_.push_back(std::move(positional));
    return ret;
  }

  ArgBase* get(const std::string& name, bool find_from_parent = true) {
    if (name.length() == 1) {
      auto it = std::ranges::find_if(args_, [name](const auto& arg) {
        return std::find(arg->short_opt_names_.begin(),
                         arg->short_opt_names_.end(),
                         name) != arg->short_opt_names_.end();
      });
      return it != args_.end()
                 ? it->get()
                 : ((find_from_parent && parent_) ? parent_->get(name)
                                                  : nullptr);
    } else {
      auto it = std::ranges::find_if(args_, [name](const auto& arg) {
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
  ArgBase& operator[](const std::string& name) {
    auto* arg = get(name);
    if (arg == nullptr) {
      detail::die("Unknown option: " + name);
    }
    return *arg;
  }

  Command& callback(std::function<void()> cb) {
    this->callback_ = std::move(cb);
    return *this;
  }
  Command& usage_header(std::string header) {
    this->usage_header_ = std::move(header);
    return *this;
  }
  Command& usage_footer(std::string footer) {
    this->usage_footer_ = std::move(footer);
    return *this;
  }

  void parse(size_t argc, char const* const* argv) {
    this->is_parsed_ = true;
    std::vector<const char*> commands{argv, argv + argc};
    ARG_PARSER_DEBUG(
        detail::join(std::vector<std::string>{argv, argv + argc}, ' '));
    size_t i = 1;  // Skip program name
    if (!commands.empty() && commands[0] != nullptr && commands[0][0] == '-') {
      i = 0;
    }
    std::vector<ArgBase*> positionals;

    // Collect all positional arguments
    for (const auto& arg : args_) {
      if (arg->is_positional()) {
        positionals.push_back(arg.get());
      }
    }

    size_t pos_index = 0;

    while (i < commands.size()) {
      const std::string& arg = commands[i];

      // Handle long options (--option)
      if (arg.size() > 2 && arg.substr(0, 2) == "--") {
        std::string name = arg.substr(2);
        auto eq_pos = name.find('=');
        std::string value;

        if (eq_pos != std::string::npos) {
          value = name.substr(eq_pos + 1);
          name = name.substr(0, eq_pos);
        }

        if (auto* option = get(name); option != nullptr) {
          if (option->is_flag()) {
            auto* flag = dynamic_cast<FlagBase*>(option);
            ARG_PARSER_DEBUG("flag: " << name);
            flag->parse();
          } else if (option->is_option()) {
            auto* opt = dynamic_cast<OptionBase*>(option);
            if (eq_pos != std::string::npos) {
              ARG_PARSER_DEBUG("option: " << name << "=" << value);
              opt->parse(value);
            } else if (i + 1 < commands.size()) {
              auto val = commands[++i];
              ARG_PARSER_DEBUG("option: " << name << "=" << val);
              opt->parse(val);
            } else {
              detail::die("Missing value for option: " + name);
            }
          }
        } else if (name.length() > 3 && name.substr(0, 3) == "no-") {
          if (auto* negatable_flag = get(name.substr(3));
              negatable_flag != nullptr) {
            if (negatable_flag->is_flag() &&
                dynamic_cast<FlagBase*>(negatable_flag)->is_negatable()) {
              dynamic_cast<FlagBase*>(negatable_flag)->parse_negated();
            } else {
              detail::die("Unknown option: " + name);
            }
          }
        } else {
          detail::die("Unknown option: " + name);
        }
      }
      // Handle short options (-o)
      else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
        std::string opts = arg.substr(1);

        // Handle combined short options
        for (size_t j = 0; j < opts.size(); ++j) {
          std::string name(1, opts[j]);
          if (auto* option = get(name)) {
            if (option->is_flag()) {
              auto* flag = dynamic_cast<FlagBase*>(option);
              ARG_PARSER_DEBUG("flag: " << name);
              flag->parse();
            } else if (option->is_option()) {
              auto* opt = dynamic_cast<OptionBase*>(option);
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
                detail::die("Missing value for option: " + name);
              }
            }
          } else {
            detail::die("Unknown option: " + name);
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
          auto* pos = dynamic_cast<OptionBase*>(positionals[pos_index]);
          if (treat_remaining_as_positional() &&
              pos_index == positionals.size() - 1 && pos->is_multiple()) {
            break;
          }
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
              // Dispatch to subcommand first; parent-level arguments that
              // appear after the subcommand name (e.g. --parent-flag in
              // "cmd --flag subcmd --parent-flag") are resolved via parent
              // lookup during subcommand parsing.  Only after the subcommand
              // has consumed those arguments do we run the parent's
              // post-processing (env fallback, defaults, required checks,
              // callback), so that the parent sees the complete set of
              // parsed values.
              (*subcmd_ptr_it)->parse(argc - i, argv + i);
              finish_parse_();
              return;
            } else {
              detail::die("Unknown subcommand: " + arg);
            }
          } else {
            detail::die("Too many positional arguments");
          }
        }
      }
      ++i;
    }

    while (i < commands.size()) {
      if (pos_index < positionals.size()) {
        auto* pos = dynamic_cast<OptionBase*>(positionals[pos_index]);
        ARG_PARSER_DEBUG("positional: " << pos_index << ": " << commands[i]);
        pos->parse(commands[i]);
        if (!pos->is_multiple()) {
          pos_index++;
        }
      } else {
        detail::die("Too many positional arguments");
      }
      ++i;
    }

    finish_parse_();
  }

 private:
  /// Post-processing shared by the no-subcommand and subcommand-dispatch
  /// paths: apply env fallback, default values, required-option checks,
  /// and finally invoke the command callback (if set).
  void finish_parse_() {
    // Handle environment variables
    for (const auto& arg : args_) {
      if ((arg->is_option() || arg->is_positional()) && arg->count() == 0) {
        dynamic_cast<OptionBase*>(arg.get())->use_env_if_needed();
      }
    }
    // Handle options that were not provided but have default values
    for (const auto& arg : args_) {
      if ((arg->is_option() || arg->is_positional()) && arg->count() == 0) {
        dynamic_cast<OptionBase*>(arg.get())->use_default_if_needed();
      }
    }
    // Check required options
    for (const auto& arg : args_) {
      if ((arg->is_option() || arg->is_positional()) && arg->count() == 0 &&
          dynamic_cast<OptionBase*>(arg.get())->is_required()) {
        detail::die("Missing required option or positional: " +
                    (arg->long_opt_names_.empty()
                         ? *(arg->short_opt_names_.begin())
                         : *(arg->long_opt_names_.begin())));
      }
    }
    // Invoke command-level callback
    if (callback_) {
      callback_();
    }
  }

 public:
  virtual std::string usage() const {
    std::stringstream usage_str;

    if (parent_ != nullptr) {
      std::vector<std::string> parent_cmds;
      Command* p = parent_;
      while (p != nullptr) {
        parent_cmds.push_back(p->command());
        p = p->parent_;
      }
      std::copy(parent_cmds.rbegin(), parent_cmds.rend(),
                std::ostream_iterator<std::string>(usage_str, " "));
    }

    if (!usage_header_.empty()) {
      usage_str << usage_header_;
      if (usage_header_.back() != '\n') {
        usage_str << '\n';
      }
    }

    usage_str << command_;
    if (std::ranges::find_if(args_, [](const auto& arg) {
          return arg->is_option() || arg->is_flag();
        }) != args_.end()) {
      usage_str << " [options]...";
    }
    if (!subcommands_.empty()) {
      usage_str << " [cmd] [options]...";
    }
    auto positionals = args_ | std::views::filter([](const auto& arg) {
                         return arg->is_positional();
                       });
    if (!positionals.empty()) {
      usage_str << " [--]";
    }
    for (const auto& arg : positionals) {
      usage_str << " "
                << dynamic_cast<OptionBase*>(arg.get())->value_placeholder_;
      if (dynamic_cast<OptionBase*>(arg.get())->is_multiple()) {
        usage_str << "...";
      }
    }
    if (std::ranges::find_if(args_, [](const auto& arg) {
          return arg->is_option() || arg->is_flag();
        }) != args_.end()) {
      usage_str << "\n\nOptions:";
    }
    for (const auto& arg : args_) {
      if ((arg->is_option() || arg->is_flag()) && !arg->hidden_) {
        usage_str << "\n" << arg->usage();
      }
    }

    if (std::ranges::find_if(args_, [](const auto& arg) {
          return arg->is_positional();
        }) != args_.end()) {
      usage_str << "\n\nPositionals:";
    }
    for (const auto& arg : args_) {
      if (arg->is_positional() && !arg->hidden_) {
        usage_str << "\n" << arg->usage();
      }
    }
    if (!usage_footer_.empty()) {
      usage_str << "\n" << usage_footer_;
    }
    return usage_str.str();
  }
  std::string one_line_usage() {
    std::stringstream usage_str;
    usage_str << detail::format(command_, description_, " ");
    return usage_str.str();
  }
  void add_default_help_flag() {
    static bool default_help{false};
    auto* const help = get("help", false);
    if (nullptr == help) {
      auto* const h = get("h", false);
      std::string help_name = "help";
      if (nullptr == h) {
        help_name = "h,help";
      }
      auto& f = add_flag(help_name, "Print this help message", default_help);
      f.callback([this](bool v) {
        if (v) {
          print_usage();
          std::exit(0);
        }
      });
    }
  }

  virtual void print_usage() const { std::cerr << usage() << "\n"; }

  std::string const& command() const { return command_; }
  std::string const& name() const { return command_; }
  std::string const& description() const { return description_; }
  Command& hidden(bool v = true) {
    is_hidden_ = v;
    return *this;
  }
  void set_parent(Command* parent) { parent_ = parent; }
  bool has_parent() const { return parent_ != nullptr; }
  bool is_parsed() { return is_parsed_; }
  bool is_hidden() const { return is_hidden_; }
  void set_treat_remaining_as_positional(bool v = true) {
    treat_remaining_as_positional_ = v;
  }
  bool treat_remaining_as_positional() const {
    return treat_remaining_as_positional_;
  }

  Command* get_subcommand(std::string const& cmd) {
    for (auto& subcmd : subcommands_) {
      if (subcmd->command() == cmd) {
        return subcmd.get();
      }
    }
    return nullptr;
  }

 protected:
  bool flag_or_option_exists(ArgBase& new_arg) const {
    return flag_exists(new_arg) || option_exists(new_arg);
  }
  bool flag_exists(ArgBase& new_arg) const {
    return option_name_exists(new_arg, 0);
  }
  bool option_exists(ArgBase& new_arg) const {
    return option_name_exists(new_arg, 1);
  }
  bool positional_exists(ArgBase& new_arg) const {
    return option_name_exists(new_arg, 2);
  }
  bool option_name_exists(ArgBase& new_arg, int type) const {
    for (const auto& arg : args_) {
      if (type == 0 && !arg->is_flag()) {
        continue;
      }
      if (type == 1 && !arg->is_option()) {
        continue;
      }
      if (type == 2 && !arg->is_positional()) {
        continue;
      }
      for (auto& name : arg->long_opt_names_) {
        if (std::find(new_arg.long_opt_names_.begin(),
                      new_arg.long_opt_names_.end(),
                      name) != new_arg.long_opt_names_.end()) {
          return true;
        }
      }
      for (auto& name : arg->short_opt_names_) {
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
  std::string usage_header_;
  std::string usage_footer_;
  std::vector<std::shared_ptr<Command>> subcommands_;
  Command* parent_{nullptr};
  std::function<void()> callback_{nullptr};
  bool is_parsed_{false};
  bool is_hidden_{false};
  bool treat_remaining_as_positional_{false};
  CommandSchema command_schema_;
};

enum class token_kind {
  short_option,
  long_option,
  text,
  option_terminator,
  command
};

struct token {
  token_kind kind;
  std::string value;
};

class argv_stream {
 public:
  argv_stream(int argc, const char* argv[])
      : args_{argv + 1, argv + argc}, index_{0} {}

  bool eof() const { return index_ == args_.size(); }
  std::string const& consume() {
    assert(!eof());
    return args_[index_++];
  }

  std::string const* peek(size_t offset = 0) {
    auto pos = index_ + offset;
    if (pos >= args_.size()) {
      return nullptr;
    }
    return &args_[index_ + offset];
  }

 private:
  std::vector<std::string> args_;
  size_t index_{0};
};

inline std::vector<token> tokenize(argv_stream& args, Command& cmd) {
  std::vector<token> tokens;
  Command* current_command = &cmd;
  bool positional_only = false;
  while (!args.eof()) {
    std::string const& arg = args.consume();
    if (positional_only) {
      tokens.emplace_back(token{token_kind::text, arg});
    } else if (arg == "--") {
      positional_only = true;
      tokens.emplace_back(token{token_kind::option_terminator, arg});
    } else if (arg.starts_with("--")) {
      std::string body = arg.substr(2);
      if (auto pos = body.find("="); pos != std::string::npos) {
        tokens.emplace_back(
            token{token_kind::long_option, body.substr(0, pos)});
        tokens.emplace_back(token{token_kind::text, body.substr(pos + 1)});
      } else {
        tokens.emplace_back(token{token_kind::long_option, body});
      }
    } else if (arg.starts_with("-") && arg.length() > 1) {
      std::transform(
          arg.begin() + 1, arg.end(), back_inserter(tokens), [](char c) {
            return token{token_kind::short_option, std::string(1, c)};
          });
    } else if (Command* subcmd = current_command->get_subcommand(arg); subcmd) {
      tokens.emplace_back(token{token_kind::command, arg});
      current_command = subcmd;
    } else {
      tokens.emplace_back(token{token_kind::text, arg});
    }
  }

  return tokens;
}

class ArgParser : public Command {
 public:
  ArgParser(std::string prog, std::string description)
      : Command(prog, description) {}
  virtual ~ArgParser() {}
  ArgParser(ArgParser&& other) = default;
  ArgParser& operator=(ArgParser&& other) = default;
  using Command::add_flag;
  using Command::add_option;
  using Command::add_positional;
  std::string usage() const override {
    std::stringstream usage_str;
    if (!description_.empty()) {
      usage_str << description_;
      if (description_.back() != '\n') {
        usage_str << '\n';
      }
    }
    usage_str << "\nUsage:\n";
    usage_str << this->Command::usage();

    if (!subcommands_.empty()) {
      usage_str << "\nAvailable Commands:";
      for (auto const& cmd : subcommands_) {
        if (cmd->is_hidden()) {
          continue;
        }
        usage_str << "\n" << cmd->one_line_usage();
      }
    }

    return usage_str.str();
  }
  void print_usage() const override { std::cerr << this->usage() << '\n'; }
  Command& parse(size_t argc, const char* const* argv) {
    add_default_help_flag();
    for (auto& sc : subcommands_) {
      sc->add_default_help_flag();
    }
    Command::parse(argc, argv);
    auto it = std::find_if(begin(subcommands_), end(subcommands_),
                           [](auto& cmdptr) { return cmdptr->is_parsed(); });
    if (it != end(subcommands_)) {
      return **it;
    }
    return *this;
  }
  Command& parse(std::vector<std::string> const& args) {
    std::vector<const char*> args_cstr(args.size() + 1, nullptr);  // NOLINT>

    std::transform(args.begin(), args.end(), args_cstr.begin(),
                   [](const std::string& s) { return s.data(); });
    return parse(args.size(), args_cstr.data());
  }
#if defined(_WIN32)
  Command& parse(size_t argc, wchar_t const* const* wargv) {
    std::vector<std::string> args;
    args.reserve(argc);
    std::transform(wargv, wargv + argc, std::back_inserter(args),
                   [](const wchar_t* s) {
                     return detail::to_string(std::wstring_view(s));
                   });
    std::vector<const char*> args_cstr(argc + 1, nullptr);
    std::transform(args.begin(), args.end(), args_cstr.begin(),
                   [](const std::string& s) { return s.data(); });
    return parse(argc, args_cstr.data());
  }
  Command& parse(std::vector<std::wstring> const& args) {
    std::vector<const wchar_t*> args_cstr(args.size() + 1,
                                          nullptr);  // NOLINT>

    std::transform(args.begin(), args.end(), args_cstr.begin(),
                   [](const std::wstring& s) { return s.data(); });
    return parse(args.size(), args_cstr.data());
  }
#endif

  Command& add_command(std::string const& cmd, std::string const& description) {
    if (std::any_of(begin(args_), end(args_),
                    [](auto const& a) { return a->is_positional(); })) {
      detail::die("Cannot add a subcommand when positional arguments exist");
    }
    auto cmd_ptr = std::make_shared<Command>(cmd, description);
    cmd_ptr->set_parent(this);
    subcommands_.push_back(cmd_ptr);
    return *cmd_ptr;
  }

 private:
  /// Build a sanitized shell function name from a command path
  /// (e.g. {"myprog","build"} → "_myprog_build").
  static std::string bash_func_name(std::vector<std::string> const& path) {
    std::string name = "_" + detail::join(path, "_");
    for (char& c : name) {
      if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
        c = '_';
      }
    }
    return name;
  }

  /// Escape a string for double-quoted/single-quote Bash contexts.
  /// Double-Quote Escapes \", $, `, \, ! to prevent premature string
  /// Single-Quote Escapes ' to '\''
  /// termination, variable/command expansion, and history expansion.
  static std::string escape_bash(std::string const& s,
                                 bool is_double_quote_string = true) {
    std::string result;
    if (is_double_quote_string) {
      for (char c : s) {
        if (c == '"' || c == '$' || c == '`' || c == '\\' || c == '!') {
          result += '\\';
          result += c;
        } else {
          result += c;
        }
      }
    } else {
      for (char c : s) {
        if (c == '\'') {
          result += "'\\''";
        } else {
          result += c;
        }
      }
    }
    return result;
  }

  /// Write the "inner" completion logic for a single Command level
  /// to @p os.  The generated function receives two positional
  /// arguments: `$1` = current word, `$2` = previous word.
  /// @param cmd    The Command whose flags / options / subcommands
  ///               should be completed.
  /// @param func   Name of the shell function being generated.
  /// @param path   The path of command names leading to @p cmd
  ///               (used to construct sub-function names for
  ///               nested subcommands).
  void print_bash_complete_impl(std::ostream& os, Command const& cmd,
                                std::string const& func,
                                std::vector<std::string> const& path) const {
    os << func << "() {\n";
    os << "    local cur=\"$1\" prev=\"$2\"\n";
    os << "\n";

    bool has_any_option = std::ranges::any_of(cmd.args_, [](auto const& a) {
      return !a->hidden_ && (a->is_flag() || a->is_option());
    });

    // ---- prev-word dispatch (space-separated option values) ----
    bool has_prev_cases = false;
    for (auto const& arg : cmd.args_) {
      if (arg->hidden_) {
        continue;
      }
      if (!arg->is_option()) {
        continue;
      }
      auto* opt = dynamic_cast<OptionBase*>(arg.get());
      if (!opt || opt->choices_.empty()) {
        continue;
      }

      if (!has_prev_cases) {
        os << "    case \"$prev\" in\n";
        has_prev_cases = true;
      }

      std::vector<std::string> patterns;
      for (auto const& s : arg->short_opt_names_) {
        patterns.push_back("-" + s);
      }
      for (auto const& l : arg->long_opt_names_) {
        patterns.push_back("--" + l);
      }
      os << "        " << detail::join(patterns, '|') << ")\n";

      std::vector<std::string> choice_keys;
      for (auto const& [k, _] : opt->choices_) {
        choice_keys.push_back(k);
      }
      os << "            # shellcheck disable=SC2207\n";
      os << "            COMPREPLY=($(compgen -W \""
         << escape_bash(detail::join(choice_keys, ' ')) << "\" -- \"$cur\"))\n";
      os << "            return 0\n";
      os << "            ;;\n";
    }
    // Also emit prev-case entries for options that take a value but
    // have no explicit choices — default to file completion.
    for (auto const& arg : cmd.args_) {
      if (arg->hidden_) {
        continue;
      }
      if (!arg->is_option()) {
        continue;
      }
      auto* opt = dynamic_cast<OptionBase*>(arg.get());
      if (!opt || !opt->choices_.empty()) {
        continue;
      }

      if (!has_prev_cases) {
        os << "    case \"$prev\" in\n";
        has_prev_cases = true;
      }

      std::vector<std::string> patterns;
      for (auto const& s : arg->short_opt_names_) {
        patterns.push_back("-" + s);
      }
      for (auto const& l : arg->long_opt_names_) {
        patterns.push_back("--" + l);
      }
      os << "        " << detail::join(patterns, '|') << ")\n";
      os << "            # shellcheck disable=SC2207\n";
      os << "            COMPREPLY=($(compgen -f -- \"$cur\"))\n";
      os << "            return 0\n";
      os << "            ;;\n";
    }
    if (has_prev_cases) {
      os << "    esac\n";
    }
    os << "\n";

    // ---- --opt=value dispatch (equals-separated long-option values) ----
    bool has_eq_cases = false;
    for (auto const& arg : cmd.args_) {
      if (arg->hidden_) {
        continue;
      }
      if (!arg->is_option()) {
        continue;
      }
      auto* opt = dynamic_cast<OptionBase*>(arg.get());
      if (!opt || opt->choices_.empty()) {
        continue;
      }

      if (!has_eq_cases) {
        os << "    if [[ \"$cur\" == --*=* ]]; then\n";
        os << "        local opt=\"${cur%%=*}\"\n";
        os << "        local val=\"${cur#*=}\"\n";
        os << "        case \"$opt\" in\n";
        has_eq_cases = true;
      }
      for (auto const& l : arg->long_opt_names_) {
        os << "            --" << l << ")\n";
        std::vector<std::string> choice_keys;
        for (auto const& [k, _] : opt->choices_) {
          choice_keys.push_back(k);
        }
        os << "                # shellcheck disable=SC2207\n";
        os << "                COMPREPLY=($(compgen -W \""
           << escape_bash(detail::join(choice_keys, ' '))
           << "\" -- \"$val\"))\n";
        os << "                return 0\n";
        os << "                ;;\n";
      }
    }
    if (has_eq_cases) {
      os << "        esac\n";
      os << "    fi\n";
      os << "\n";
    }

    // ---- complete option names ----
    if (has_any_option) {
      os << "    if [[ \"$cur\" == -* ]]; then\n";
      os << "        local opts=\"";
      std::vector<std::string> opt_names;
      for (auto const& arg : cmd.args_) {
        if (arg->hidden_) {
          continue;
        }
        if (arg->is_flag() || arg->is_option()) {
          for (auto const& s : arg->short_opt_names_) {
            opt_names.push_back("-" + s);
          }
          for (auto const& l : arg->long_opt_names_) {
            opt_names.push_back("--" + l);
          }
          // negatable long-flag variants
          if (arg->is_flag()) {
            auto* flag = dynamic_cast<FlagBase*>(arg.get());
            if (flag && flag->is_negatable()) {
              for (auto const& l : arg->long_opt_names_) {
                opt_names.push_back("--no-" + l);
              }
            }
          }
        }
      }
      os << escape_bash(detail::join(opt_names, ' ')) << "\"\n";
      os << "        # shellcheck disable=SC2207\n";
      os << "        COMPREPLY=($(compgen -W \"$opts\" -- \"$cur\"))\n";
      os << "        return 0\n";
      os << "    fi\n";
      os << "\n";
    }

    // ---- complete subcommands ----
    if (!cmd.subcommands_.empty()) {
      os << "    local cmds=\"";
      std::vector<std::string> cmd_names;
      for (auto const& sc : cmd.subcommands_) {
        if (sc->is_hidden()) {
          continue;
        }
        cmd_names.push_back(sc->command());
      }
      os << escape_bash(detail::join(cmd_names, ' ')) << "\"\n";
      os << "    # shellcheck disable=SC2207\n";
      os << "    COMPREPLY=($(compgen -W \"$cmds\" -- \"$cur\"))\n";
    }

    // ---- complete positional arguments ----
    // Positionals and subcommands are mutually exclusive.
    if (cmd.subcommands_.empty()) {
      std::vector<OptionBase*> positionals;
      for (auto const& arg : cmd.args_) {
        if (arg->hidden_) {
          continue;
        }
        if (!arg->is_positional()) {
          continue;
        }
        auto* pos = dynamic_cast<OptionBase*>(arg.get());
        if (pos) {
          positionals.push_back(pos);
        }
      }
      if (!positionals.empty()) {
        // Detect which options take a value so we can skip their
        // arguments when counting positional arguments.
        os << "    # Count positional arguments already provided\n";
        os << "    local _pos_count=0 _skip=false\n";
        os << "    for ((_i=1; _i < $COMP_CWORD; _i++)); do\n";
        os << "        if $_skip; then _skip=false; continue; fi\n";
        os << "        local _w=\"${COMP_WORDS[$_i]}\"\n";
        os << "        if [[ \"$_w\" == -* ]]; then\n";
        // Emit a case to skip the value word for each option that
        // takes an argument.
        bool has_value_opts = false;
        for (auto const& arg : cmd.args_) {
          if (arg->hidden_) {
            continue;
          }
          if (!arg->is_option()) {
            continue;
          }
          has_value_opts = true;
        }
        if (has_value_opts) {
          os << "            case \"$_w\" in\n";
          for (auto const& arg : cmd.args_) {
            if (arg->hidden_) {
              continue;
            }
            if (!arg->is_option()) {
              continue;
            }
            std::vector<std::string> patterns;
            for (auto const& s : arg->short_opt_names_) {
              patterns.push_back("-" + s);
            }
            for (auto const& l : arg->long_opt_names_) {
              patterns.push_back("--" + l);
            }
            os << "                " << detail::join(patterns, '|') << ")\n";
            os << "                    _skip=true\n";
            os << "                    ;;\n";
          }
          os << "            esac\n";
        }
        os << "            continue\n";
        os << "        fi\n";
        os << "        _pos_count=$((_pos_count + 1))\n";
        os << "    done\n";
        os << "\n";
        os << "    case $_pos_count in\n";
        for (size_t idx = 0; idx < positionals.size(); ++idx) {
          os << "        " << idx << ")\n";
          if (!positionals[idx]->choices_.empty()) {
            std::vector<std::string> choice_keys;
            for (auto const& [k, _] : positionals[idx]->choices_) {
              choice_keys.push_back(k);
            }
            os << "            # shellcheck disable=SC2207\n";
            os << "            COMPREPLY=($(compgen -W \""
               << escape_bash(detail::join(choice_keys, ' '))
               << "\" -- \"$cur\"))\n";
          } else {
            os << "            # shellcheck disable=SC2207\n";
            os << "            COMPREPLY=($(compgen -f -- \"$cur\"))\n";
          }
          os << "            ;;\n";
        }
        os << "    esac\n";
        os << "    return 0\n";
      }
    }

    // Final return; omit when positionals already provide their own return.
    bool has_visible_positionals = false;
    if (cmd.subcommands_.empty()) {
      for (auto const& arg : cmd.args_) {
        if (!arg->hidden_ && arg->is_positional()) {
          has_visible_positionals = true;
          break;
        }
      }
    }
    if (!has_visible_positionals) {
      os << "    return 0\n";
    }
    os << "}\n";
    os << "\n";

    // ---- recursively generate functions for subcommands ----
    for (auto const& sc : cmd.subcommands_) {
      if (sc->is_hidden()) {
        continue;
      }
      auto sub_path = path;
      sub_path.push_back(sc->command());
      print_bash_complete_impl(os, *sc, bash_func_name(sub_path), sub_path);
    }
  }

 public:
  /// Print a Bash completion script to @p os.
  void print_bash_complete(std::ostream& os = std::cout) const {
    std::vector<std::string> root_path{command_};
    std::string root_func = bash_func_name(root_path);

    // First, recursively generate completion functions for every
    // command level (including the root).  The root implementation
    // gets a "_impl" suffix so that the public dispatcher can use
    // the bare function name.
    print_bash_complete_impl(os, *this, root_func + "_impl", root_path);

    // Emit the main dispatcher function that detects which subcommand
    // the user is inside and delegates accordingly.
    os << root_func << "() {\n";
    os << "    local cur prev cmd i\n";
    os << "    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n";
    os << "    prev=\"${COMP_WORDS[COMP_CWORD-1]}\"\n";
    os << "    cmd=\"\"\n";
    os << "    for ((i=1; i < COMP_CWORD; i++)); do\n";
    os << "        case \"${COMP_WORDS[$i]}\" in\n";

    // Collect all visible subcommand paths.
    std::vector<std::pair<std::vector<std::string>, Command const*>> all_cmds;
    {
      std::function<void(std::vector<std::string> const&, Command const*)>
          collect =
              [&](std::vector<std::string> const& prefix, Command const* cmd) {
                for (auto const& sc : cmd->subcommands_) {
                  if (sc->is_hidden()) {
                    continue;
                  }
                  auto p = prefix;
                  p.push_back(sc->command());
                  all_cmds.emplace_back(p, sc.get());
                  collect(p, sc.get());
                }
              };
      collect(root_path, this);
    }

    for (auto const& [cmd_path, _] : all_cmds) {
      os << "            " << cmd_path.back() << ")\n";
      os << "                cmd=\"" << bash_func_name(cmd_path) << "\"\n";
      os << "                break ;;\n";
    }
    os << "            -*)\n";
    os << "                continue ;;\n";
    os << "        esac\n";
    os << "    done\n";
    os << "\n";
    os << "    if [[ -n \"$cmd\" ]]; then\n";
    os << "        \"$cmd\" \"$cur\" \"$prev\"\n";
    os << "    else\n";
    os << "        " << root_func << "_impl \"$cur\" \"$prev\"\n";
    os << "    fi\n";
    os << "}\n";
    os << "\n";
    os << "complete -F " << root_func << " " << command_ << "\n";
  }
  void print_zsh_complete(std::ostream& os = std::cout) const {
    auto escape_zsh_spec = [](std::string const& s) -> std::string {
      std::string result;
      for (char c : s) {
        if (c == '\'') {
          result += "'\\''";
        } else {
          result += c;
        }
      }
      return result;
    };

    auto escape_zsh_desc = [&](std::string const& s) -> std::string {
      std::string result;
      for (char c : s) {
        if (c == ':' || c == '[' || c == ']') {
          result += '\\';
        }
        if (c == '\'') {
          result += "'\\''";
        } else {
          result += c;
        }
      }
      return result;
    };

    // Recursively generate completion functions for every command level.
    std::function<void(std::vector<std::string> const&, Command const*)>
        print_zsh_rec = [&](std::vector<std::string> const& path,
                            Command const* cmd) {
          std::string func_name = "_" + detail::join(path, "_");

          os << func_name << "() {\n";
          os << "    local -a options\n";
          if (!cmd->subcommands_.empty()) {
            os << "    local line state\n";
          }
          os << "    options=(\n";

          for (auto const& arg : cmd->args_) {
            if (arg->hidden_) {
              continue;
            }
            if (!arg->is_flag() && !arg->is_option()) {
              continue;
            }

            std::string desc = escape_zsh_desc(arg->description());
            bool is_opt = arg->is_option();
            auto* opt = is_opt ? dynamic_cast<OptionBase*>(arg.get()) : nullptr;
            bool has_choices = opt && !opt->choices_.empty();

            for (auto const& s : arg->short_opt_names_) {
              os << "        '-" << s;
              if (!desc.empty()) {
                os << "[" << desc << "]";
              }
              if (is_opt) {
                os << ":"
                   << (opt->value_placeholder_.empty()
                           ? "arg"
                           : escape_zsh_spec(opt->value_placeholder_));
                if (has_choices) {
                  os << ":(";
                  std::vector<std::string> choice_keys;
                  for (auto const& [k, _] : opt->choices_) {
                    choice_keys.push_back(escape_zsh_spec(k));
                  }
                  os << detail::join(choice_keys, ' ');
                  os << ")";
                } else {
                  os << ":_files";
                }
              }
              os << "'\n";
            }
            for (auto const& l : arg->long_opt_names_) {
              os << "        '--" << l;
              if (!desc.empty()) {
                os << "[" << desc << "]";
              }
              if (is_opt) {
                os << ":"
                   << (opt->value_placeholder_.empty()
                           ? "arg"
                           : escape_zsh_spec(opt->value_placeholder_));
                if (has_choices) {
                  os << ":(";
                  std::vector<std::string> choice_keys;
                  for (auto const& [k, _] : opt->choices_) {
                    choice_keys.push_back(escape_zsh_spec(k));
                  }
                  os << detail::join(choice_keys, ' ');
                  os << ")";
                } else {
                  os << ":_files";
                }
              }
              os << "'\n";
            }

            // negatable long-flag variants
            if (arg->is_flag()) {
              auto* flag = dynamic_cast<FlagBase*>(arg.get());
              if (flag && flag->is_negatable()) {
                for (auto const& l : arg->long_opt_names_) {
                  os << "        '--no-" << l;
                  if (!desc.empty()) {
                    os << "[" << desc << "]";
                  }
                  os << "'\n";
                }
              }
            }
          }

          os << "    )\n";
          os << "\n";

          if (!cmd->subcommands_.empty()) {
            os << "    _arguments -s $options \\\n";
            os << "        '1:subcommand:->subcmds' \\\n";
            os << "        '*::arg:->args'\n";
            os << "\n";
            os << "    case $state in\n";
            os << "        subcmds)\n";
            os << "            _values 'subcommand'";
            for (auto const& sc : cmd->subcommands_) {
              if (sc->is_hidden()) {
                continue;
              }
              os << " \\\n                '" << sc->command();
              if (!sc->description().empty()) {
                os << "[" << escape_zsh_desc(sc->description()) << "]";
              }
              os << "'";
            }
            os << "\n            ;;\n";
            os << "        args)\n";
            os << "            case $line[1] in\n";
            for (auto const& sc : cmd->subcommands_) {
              if (sc->is_hidden()) {
                continue;
              }
              auto sub_path = path;
              sub_path.push_back(sc->command());
              os << "                " << sc->command() << ")\n";
              os << "                    _" << detail::join(sub_path, "_")
                 << "\n";
              os << "                    ;;\n";
            }
            os << "            esac\n";
            os << "            ;;\n";
            os << "    esac\n";
          } else {
            // Collect positional arguments for this command
            std::vector<OptionBase*> positionals;
            for (auto const& arg : cmd->args_) {
              if (arg->hidden_) {
                continue;
              }
              if (!arg->is_positional()) {
                continue;
              }
              auto* pos = dynamic_cast<OptionBase*>(arg.get());
              if (pos) {
                positionals.push_back(pos);
              }
            }
            if (!positionals.empty()) {
              os << "    _arguments -s $options";
              for (size_t idx = 0; idx < positionals.size(); ++idx) {
                bool is_last = (idx + 1 == positionals.size());
                bool is_multi = positionals[idx]->is_multiple();
                os << " \\\n        '"
                   << (is_last && is_multi ? "*" : argparse::to_string(idx + 1))
                   << ":"
                   << (positionals[idx]->value_placeholder_.empty()
                           ? "arg" + argparse::to_string(idx + 1)
                           : escape_zsh_spec(
                                 positionals[idx]->value_placeholder_));
                if (!positionals[idx]->choices_.empty()) {
                  os << ":(";
                  std::vector<std::string> choice_keys;
                  for (auto const& [k, _] : positionals[idx]->choices_) {
                    choice_keys.push_back(escape_zsh_spec(k));
                  }
                  os << detail::join(choice_keys, ' ');
                  os << ")";
                } else {
                  os << ":_files";
                }
                os << "'";
              }
              os << "\n";
            } else {
              os << "    _arguments -s $options\n";
            }
          }
          os << "}\n";
          os << "\n";

          // Recurse into subcommands
          for (auto const& sc : cmd->subcommands_) {
            if (sc->is_hidden()) {
              continue;
            }
            auto sub_path = path;
            sub_path.push_back(sc->command());
            print_zsh_rec(sub_path, sc.get());
          }
        };

    std::vector<std::string> root_path{command_};
    os << "#compdef " << command_ << "\n";
    os << "\n";
    print_zsh_rec(root_path, this);
    os << "_" << command_ << " \"$@\"\n";
  }
  /// Print a Fish completion script to @p os.
  void print_fish_complete(std::ostream& os = std::cout) const {
    auto escape_fish = [](std::string const& s) -> std::string {
      std::string result;
      for (char c : s) {
        if (c == '\'') {
          result += "'\\''";
        } else {
          result += c;
        }
      }
      return result;
    };

    // Build the condition string for having seen the given subcommand
    // path.  E.g., for {"myprog","build","release"} →
    //   "__fish_seen_subcommand_from build; and __fish_seen_subcommand_from
    //   release"
    auto seen_condition =
        [](std::vector<std::string> const& cmd_chain) -> std::string {
      if (cmd_chain.empty()) {
        return "";
      }
      std::string cond;
      for (size_t i = 0; i < cmd_chain.size(); ++i) {
        if (i > 0) {
          cond += "; and ";
        }
        cond += "__fish_seen_subcommand_from " + cmd_chain[i];
      }
      return cond;
    };

    // Recursively emit completions for a command and all its
    // subcommands.
    std::function<void(std::vector<std::string> const&, Command const*)>
        print_fish_rec = [&](std::vector<std::string> const& cmd_chain,
                             Command const* cmd) {
          std::string condition = seen_condition(cmd_chain);
          std::string cond_prefix;
          if (!condition.empty()) {
            cond_prefix = " -n '" + condition + "'";
          }

          // Options / flags for this command
          for (auto const& arg : cmd->args_) {
            if (arg->hidden_) {
              continue;
            }
            if (!arg->is_flag() && !arg->is_option()) {
              continue;
            }

            std::string desc = arg->description();
            bool is_opt = arg->is_option();
            auto* opt = is_opt ? dynamic_cast<OptionBase*>(arg.get()) : nullptr;
            bool has_choices = opt && !opt->choices_.empty();

            for (auto const& s : arg->short_opt_names_) {
              os << "complete -c " << command_ << cond_prefix << " -s " << s;
              if (!desc.empty()) {
                os << " -d '" << escape_fish(desc) << "'";
              }
              if (is_opt) {
                os << " -r";
                if (has_choices) {
                  os << " -f";
                  std::vector<std::string> choice_keys;
                  for (auto const& [k, _] : opt->choices_) {
                    choice_keys.push_back(k);
                  }
                  os << " -a '" << escape_fish(detail::join(choice_keys, ' '))
                     << "'";
                }
              }
              os << "\n";
            }
            for (auto const& l : arg->long_opt_names_) {
              os << "complete -c " << command_ << cond_prefix << " -l " << l;
              if (!desc.empty()) {
                os << " -d '" << escape_fish(desc) << "'";
              }
              if (is_opt) {
                os << " -r";
                if (has_choices) {
                  os << " -f";
                  std::vector<std::string> choice_keys;
                  for (auto const& [k, _] : opt->choices_) {
                    choice_keys.push_back(k);
                  }
                  os << " -a '" << escape_fish(detail::join(choice_keys, ' '))
                     << "'";
                }
              }
              os << "\n";
            }

            // negatable long-flag variants
            if (arg->is_flag()) {
              auto* flag = dynamic_cast<FlagBase*>(arg.get());
              if (flag && flag->is_negatable()) {
                for (auto const& l : arg->long_opt_names_) {
                  os << "complete -c " << command_ << cond_prefix << " -l no-"
                     << l;
                  if (!desc.empty()) {
                    os << " -d '" << escape_fish(desc) << "'";
                  }
                  os << "\n";
                }
              }
            }
          }

          // Subcommand listing for this level.
          // Use __fish_use_subcommand when we're at this command
          // but no subcommand has been seen yet.
          if (!cmd->subcommands_.empty()) {
            std::string sub_cond;
            if (cmd_chain.empty()) {
              // Top level
              sub_cond = "__fish_use_subcommand";
            } else {
              // Nested: we are inside the parent chain but no
              // further subcommand has been chosen yet.
              sub_cond = condition;
              // Also exclude situations where a sub-subcommand is
              // already present (so we only complete one level at
              // a time).  We already emit completions for deeper
              // levels with their own conditions below.
            }
            for (auto const& sc : cmd->subcommands_) {
              if (sc->is_hidden()) {
                continue;
              }
              os << "complete -c " << command_ << " -n '" << sub_cond
                 << "' -f -a '" << sc->command() << "'";
              if (!sc->description().empty()) {
                os << " -d '" << escape_fish(sc->description()) << "'";
              }
              os << "\n";
            }
          } else {
            // Positional arguments (only when there are no subcommands,
            // as they are mutually exclusive).
            std::vector<OptionBase*> positionals;
            for (auto const& arg : cmd->args_) {
              if (arg->hidden_) {
                continue;
              }
              if (!arg->is_positional()) {
                continue;
              }
              auto* pos = dynamic_cast<OptionBase*>(arg.get());
              if (pos) {
                positionals.push_back(pos);
              }
            }
            for (auto const* pos : positionals) {
              if (pos->choices_.empty()) {
                continue;
              }
              std::vector<std::string> choice_keys;
              for (auto const& [k, _] : pos->choices_) {
                choice_keys.push_back(k);
              }
              os << "complete -c " << command_ << cond_prefix << " -f -a '"
                 << escape_fish(detail::join(choice_keys, ' ')) << "'";
              if (!pos->description().empty()) {
                os << " -d '" << escape_fish(pos->description()) << "'";
              }
              os << "\n";
            }
          }

          // Recurse into subcommands with extended chain
          for (auto const& sc : cmd->subcommands_) {
            if (sc->is_hidden()) {
              continue;
            }
            auto sub_chain = cmd_chain;
            sub_chain.push_back(sc->command());
            print_fish_rec(sub_chain, sc.get());
          }
        };
    // Start from the root (empty chain → no condition)
    print_fish_rec({}, this);
  }
};

}  // namespace argparse

#endif  // __ARGPARSE_ARGPARSE_HPP__
