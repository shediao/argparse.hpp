#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "argparse/argparse.hpp"

// ============================================================
// Helper types used across multiple test categories
// ============================================================

class StringConstructible {
 public:
  explicit StringConstructible(const std::string&) {}
};

class StringConvertible {
 public:
  StringConvertible(const std::string&) {}
};

class NonStringConstructible {
 public:
  explicit NonStringConstructible(int) {}
};

// ============================================================
// 2. is_string_v
// ============================================================

TEST(ConceptTest, IsString) {
  // Standard string types
  ASSERT_TRUE(argparse::detail::is_string_v<std::string>);
  ASSERT_TRUE(argparse::detail::is_string_v<std::wstring>);
  ASSERT_TRUE(argparse::detail::is_string_v<std::u8string>);
  ASSERT_TRUE(argparse::detail::is_string_v<std::u16string>);
  ASSERT_TRUE(argparse::detail::is_string_v<std::u32string>);

  // Non-string types
  ASSERT_FALSE(argparse::detail::is_string_v<const char*>);
  ASSERT_FALSE(argparse::detail::is_string_v<int>);
  ASSERT_FALSE(argparse::detail::is_string_v<std::vector<char>>);
  ASSERT_FALSE(argparse::detail::is_string_v<char[10]>);
  ASSERT_FALSE(argparse::detail::is_string_v<std::string_view>);
  ASSERT_FALSE(argparse::detail::is_string_v<std::wstring_view>);
}

// Custom allocator and traits for testing is_string with non-standard
// basic_string instantiations
template <typename T>
struct CustomAllocator {
  template <typename U>
  constexpr CustomAllocator(const CustomAllocator<U>&) noexcept {}
  [[nodiscard]] T* allocate(std::size_t n) {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }
  void deallocate(T* p, std::size_t) noexcept { ::operator delete(p); }
  bool operator==(const CustomAllocator&) const { return true; }
  bool operator!=(const CustomAllocator&) const { return false; }
};

struct CustomTraits : std::char_traits<char> {};

TEST(ConceptTest, IsStringEnhanced) {
  // basic_string with custom allocator
  ASSERT_TRUE((argparse::detail::is_string_v<std::basic_string<
                   char, std::char_traits<char>, CustomAllocator<char>>>));
  ASSERT_TRUE(
      (argparse::detail::is_string_v<std::basic_string<
           wchar_t, std::char_traits<wchar_t>, CustomAllocator<wchar_t>>>));

  // basic_string with custom traits
  ASSERT_TRUE(
      (argparse::detail::is_string_v<std::basic_string<char, CustomTraits>>));
  ASSERT_TRUE((argparse::detail::is_string_v<
               std::basic_string<char, CustomTraits, CustomAllocator<char>>>));
}

// ============================================================
// 3. is_optional_v
// ============================================================

TEST(ConceptTest, IsOptional) {
  ASSERT_TRUE(argparse::detail::is_optional_v<std::optional<int>>);
  ASSERT_TRUE(argparse::detail::is_optional_v<std::optional<std::string>>);
  ASSERT_FALSE(argparse::detail::is_optional_v<int>);
  ASSERT_FALSE(argparse::detail::is_optional_v<std::string>);
}

// ============================================================
// 4. can_from_string_without_delim
// ============================================================

TEST(ConceptTest, CanFromStringWithoutDelim) {
  // Basic types
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<bool>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<char>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<short>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<unsigned short>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<int>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<long>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<unsigned long>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<long long>);
  ASSERT_TRUE(
      argparse::detail::can_from_string_without_delim<unsigned long long>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<float>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<double>);
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<long double>);

  // String types
  ASSERT_TRUE(argparse::detail::can_from_string_without_delim<std::string>);

  // Wide/unicode character and string types are not supported
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<wchar_t>);
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<char16_t>);
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<char32_t>);
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<unsigned char>);
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<std::wstring>);
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<std::u8string>);
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<std::u16string>);
  ASSERT_FALSE(argparse::detail::can_from_string_without_delim<std::u32string>);

  // Compound types are not supported
  ASSERT_FALSE(
      (argparse::detail::can_from_string_without_delim<std::vector<int>>));
  ASSERT_FALSE(
      (argparse::detail::can_from_string_without_delim<std::pair<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::can_from_string_without_delim<std::tuple<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::can_from_string_without_delim<std::optional<int>>));
}

// ============================================================
// 5. can_from_string_with_delim
// ============================================================

TEST(ConceptTest, CanFromStringWithDelim) {
  // Tuple types
  ASSERT_TRUE((argparse::detail::can_from_string_with_delim<std::tuple<int>>));
  ASSERT_TRUE((argparse::detail::can_from_string_with_delim<
               std::tuple<int, std::string>>));
  ASSERT_TRUE((argparse::detail::can_from_string_with_delim<
               std::tuple<bool, int, double>>));

  // Pair types
  ASSERT_TRUE((argparse::detail::can_from_string_with_delim<
               std::pair<int, std::string>>));
  ASSERT_TRUE(
      (argparse::detail::can_from_string_with_delim<std::pair<int, double>>));
  ASSERT_TRUE(
      (argparse::detail::can_from_string_with_delim<std::pair<double, bool>>));

  // Array types
  ASSERT_TRUE(
      (argparse::detail::can_from_string_with_delim<std::array<int, 1>>));
  ASSERT_TRUE(
      (argparse::detail::can_from_string_with_delim<std::array<int, 3>>));
  ASSERT_TRUE(
      (argparse::detail::can_from_string_with_delim<std::array<int, 8>>));
  ASSERT_TRUE(
      (argparse::detail::can_from_string_with_delim<std::array<double, 3>>));

  // Custom types
  ASSERT_TRUE((argparse::detail::can_from_string_with_delim<
               std::tuple<StringConstructible>>));
  ASSERT_TRUE((argparse::detail::can_from_string_with_delim<
               std::tuple<StringConvertible>>));
  ASSERT_TRUE((argparse::detail::can_from_string_with_delim<
               std::pair<StringConstructible, StringConvertible>>));

  // Non-tuple-like types
  ASSERT_FALSE(argparse::detail::can_from_string_with_delim<int>);
  ASSERT_FALSE(argparse::detail::can_from_string_with_delim<std::string>);
  ASSERT_FALSE(
      (argparse::detail::can_from_string_with_delim<std::vector<int>>));

  // Tuple-like types with unsupported element types
  ASSERT_FALSE((argparse::detail::can_from_string_with_delim<
                std::tuple<NonStringConstructible>>));
  ASSERT_FALSE((argparse::detail::can_from_string_with_delim<
                std::tuple<std::vector<int>>>));
  ASSERT_FALSE((argparse::detail::can_from_string_with_delim<
                std::pair<int, std::vector<int>>>));

  // Nested tuple-like types are not supported
  ASSERT_FALSE((argparse::detail::can_from_string_with_delim<
                std::tuple<std::tuple<int>>>));
  ASSERT_FALSE((argparse::detail::can_from_string_with_delim<
                std::pair<std::pair<int, int>, int>>));
}

// ============================================================
// 6. can_from_string
// ============================================================

TEST(ConceptTest, CanFromString) {
  // Basic types
  ASSERT_TRUE(argparse::detail::can_from_string<int>);
  ASSERT_TRUE(argparse::detail::can_from_string<std::string>);

  // Tuple-like types (via delimiter)
  ASSERT_TRUE((argparse::detail::can_from_string<std::pair<int, int>>));
  ASSERT_TRUE((argparse::detail::can_from_string<std::pair<int, double>>));

  // Custom types constructible from std::string
  ASSERT_TRUE(argparse::detail::can_from_string<StringConstructible>);
  ASSERT_TRUE(argparse::detail::can_from_string<StringConvertible>);
  ASSERT_FALSE(argparse::detail::can_from_string<NonStringConstructible>);

  // Container types are not supported directly
  ASSERT_FALSE(argparse::detail::can_from_string<std::vector<int>>);

  // std::optional is not supported (use nargs('?') instead)
  ASSERT_FALSE(argparse::detail::can_from_string<std::optional<int>>);
  ASSERT_FALSE((argparse::detail::can_from_string<
                std::optional<std::pair<int, double>>>));
  ASSERT_FALSE((argparse::detail::can_from_string<
                std::optional<std::tuple<int, double>>>));
  ASSERT_FALSE(
      (argparse::detail::can_from_string<std::optional<std::array<int, 3>>>));
  ASSERT_FALSE(
      argparse::detail::can_from_string<std::optional<std::vector<int>>>);
}

// ============================================================
// 7. from_string_container
// ============================================================

TEST(ConceptTest, FromStringContainer) {
  // Sequence containers
  ASSERT_TRUE((argparse::detail::from_string_container<std::vector<int>>));
  ASSERT_TRUE(
      (argparse::detail::from_string_container<std::vector<std::string>>));
  ASSERT_TRUE((argparse::detail::from_string_container<std::vector<double>>));
  ASSERT_TRUE((argparse::detail::from_string_container<std::list<int>>));
  ASSERT_TRUE(
      (argparse::detail::from_string_container<std::deque<std::string>>));

  // Associative containers
  ASSERT_TRUE((argparse::detail::from_string_container<std::set<int>>));
  ASSERT_TRUE(
      (argparse::detail::from_string_container<std::unordered_set<double>>));
  ASSERT_TRUE((argparse::detail::from_string_container<std::map<int, int>>));
  ASSERT_TRUE((argparse::detail::from_string_container<
               std::unordered_map<std::string, int>>));

  // Containers of string-constructible custom types
  ASSERT_TRUE((argparse::detail::from_string_container<
               std::vector<StringConstructible>>));
  ASSERT_TRUE(
      (argparse::detail::from_string_container<std::list<StringConvertible>>));
  ASSERT_TRUE(
      (argparse::detail::from_string_container<std::set<StringConstructible>>));

  // Containers of tuple-like types
  ASSERT_TRUE((argparse::detail::from_string_container<
               std::vector<std::pair<int, int>>>));
  ASSERT_TRUE((argparse::detail::from_string_container<
               std::vector<std::tuple<int, std::string>>>));
  ASSERT_TRUE(
      (argparse::detail::from_string_container<std::list<std::array<int, 2>>>));

  // Non-container types
  ASSERT_FALSE(argparse::detail::from_string_container<int>);
  ASSERT_FALSE(argparse::detail::from_string_container<std::string>);
  ASSERT_FALSE((argparse::detail::from_string_container<std::pair<int, int>>));
  ASSERT_FALSE((argparse::detail::from_string_container<std::tuple<int, int>>));
  ASSERT_FALSE((argparse::detail::from_string_container<std::array<int, 3>>));

  // Containers with unsupported element types
  ASSERT_FALSE((argparse::detail::from_string_container<
                std::vector<NonStringConstructible>>));
  ASSERT_FALSE(
      (argparse::detail::from_string_container<std::vector<std::vector<int>>>));
  ASSERT_FALSE((argparse::detail::from_string_container<
                std::vector<std::tuple<std::tuple<int>>>>));
}

// ============================================================
// 8. is_container
// ============================================================

TEST(ConceptTest, IsContainer) {
  // Sequence containers
  ASSERT_TRUE((argparse::detail::is_container<std::vector<int>>));
  ASSERT_TRUE((argparse::detail::is_container<std::vector<std::string>>));
  ASSERT_TRUE((argparse::detail::is_container<std::deque<int>>));
  ASSERT_TRUE((argparse::detail::is_container<std::list<double>>));

  // Associative containers
  ASSERT_TRUE((argparse::detail::is_container<std::set<int>>));
  ASSERT_TRUE((argparse::detail::is_container<std::multiset<int>>));
  ASSERT_TRUE((argparse::detail::is_container<std::unordered_set<int>>));

  // Container adaptors
  ASSERT_TRUE((argparse::detail::is_container<std::stack<int>>));
  ASSERT_TRUE((argparse::detail::is_container<std::queue<int>>));
  ASSERT_TRUE((argparse::detail::is_container<std::priority_queue<int>>));

  // std::string and friends are also containers in the STL sense
  ASSERT_TRUE((argparse::detail::is_container<std::string>));
  ASSERT_TRUE((argparse::detail::is_container<std::wstring>));
  ASSERT_TRUE((argparse::detail::is_container<std::u8string>));
  ASSERT_TRUE((argparse::detail::is_container<std::u16string>));
  ASSERT_TRUE((argparse::detail::is_container<std::u32string>));

  // Non-container types
  ASSERT_FALSE((argparse::detail::is_container<int>));
  ASSERT_FALSE((argparse::detail::is_container<double>));
  ASSERT_FALSE((argparse::detail::is_container<bool>));
  ASSERT_FALSE((argparse::detail::is_container<std::pair<int, int>>));
  ASSERT_FALSE((argparse::detail::is_container<std::array<int, 3>>));
  ASSERT_FALSE((argparse::detail::is_container<std::optional<int>>));
  ASSERT_FALSE((argparse::detail::is_container<int[5]>));
  ASSERT_FALSE((argparse::detail::is_container<int*>));
}

// ============================================================
// 9. is_non_string_container
// ============================================================

TEST(ConceptTest, IsNonStringContainer) {
  // Standard containers (non-string)
  ASSERT_TRUE((argparse::detail::is_non_string_container<std::vector<int>>));
  ASSERT_TRUE((argparse::detail::is_non_string_container<std::deque<double>>));
  ASSERT_TRUE((argparse::detail::is_non_string_container<std::list<bool>>));
  ASSERT_TRUE((argparse::detail::is_non_string_container<std::set<int>>));
  ASSERT_TRUE(
      (argparse::detail::is_non_string_container<std::unordered_set<int>>));
  ASSERT_TRUE((argparse::detail::is_non_string_container<std::map<int, int>>));
  ASSERT_TRUE((
      argparse::detail::is_non_string_container<std::unordered_map<int, int>>));

  // vector<string> is a container but not a string type itself
  ASSERT_TRUE(
      (argparse::detail::is_non_string_container<std::vector<std::string>>));

  // Container adaptors
  ASSERT_TRUE((argparse::detail::is_non_string_container<std::stack<int>>));
  ASSERT_TRUE((argparse::detail::is_non_string_container<std::queue<int>>));
  ASSERT_TRUE(
      (argparse::detail::is_non_string_container<std::priority_queue<int>>));

  // std::string family is excluded
  ASSERT_FALSE((argparse::detail::is_non_string_container<std::string>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<std::wstring>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<std::u8string>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<std::u16string>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<std::u32string>));

  // Non-container types
  ASSERT_FALSE((argparse::detail::is_non_string_container<int>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<double>));
  ASSERT_FALSE(
      (argparse::detail::is_non_string_container<std::pair<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::is_non_string_container<std::tuple<int, int>>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<std::array<int, 3>>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<std::optional<int>>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<int[5]>));
  ASSERT_FALSE((argparse::detail::is_non_string_container<int*>));
}

// ============================================================
// 10. bindable_without_delim
// ============================================================

TEST(ConceptTest, BindableWithoutDelim) {
  // Single-value basic types
  ASSERT_TRUE(argparse::detail::bindable_without_delim<int>);
  ASSERT_TRUE(argparse::detail::bindable_without_delim<bool>);
  ASSERT_TRUE(argparse::detail::bindable_without_delim<double>);
  ASSERT_TRUE(argparse::detail::bindable_without_delim<std::string>);

  // Custom types constructible from std::string
  ASSERT_TRUE(argparse::detail::bindable_without_delim<StringConstructible>);
  ASSERT_TRUE(argparse::detail::bindable_without_delim<StringConvertible>);

  // std::optional of single-value types
  ASSERT_TRUE((argparse::detail::bindable_without_delim<std::optional<int>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_without_delim<std::optional<double>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<std::optional<bool>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_without_delim<std::optional<std::string>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<
               std::optional<StringConstructible>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<
               std::optional<StringConvertible>>));

  // std::optional with const-qualified types
  ASSERT_TRUE(
      (argparse::detail::bindable_without_delim<std::optional<const int>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<
               std::optional<const std::string>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<
               std::optional<const StringConstructible>>));

  // Containers of single-value types
  ASSERT_TRUE((argparse::detail::bindable_without_delim<std::vector<int>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_without_delim<std::vector<std::string>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<std::list<double>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_without_delim<std::list<std::string>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<std::set<int>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<std::set<double>>));
  ASSERT_TRUE((argparse::detail::bindable_without_delim<std::deque<bool>>));

  // Tuple-like types are not bindable without delimiter
  ASSERT_FALSE((argparse::detail::bindable_without_delim<std::pair<int, int>>));
  ASSERT_FALSE(
      (argparse::detail::bindable_without_delim<std::tuple<int, std::string>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<std::array<int, 3>>));

  // Containers of tuple-like types are not bindable without delimiter
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::vector<std::pair<int, int>>>));

  // Unsupported types
  ASSERT_FALSE(
      argparse::detail::bindable_without_delim<NonStringConstructible>);
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<NonStringConstructible>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<std::pair<int, int>>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<std::tuple<int, std::string>>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<std::vector<int>>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<std::pair<int, int>>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<std::tuple<int>>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<std::array<int, 1>>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::optional<std::optional<int>>>));
  ASSERT_FALSE((argparse::detail::bindable_without_delim<
                std::vector<NonStringConstructible>>));
  ASSERT_FALSE((
      argparse::detail::bindable_without_delim<std::vector<std::vector<int>>>));
}

// ============================================================
// 11. bindable_with_delim
// ============================================================

TEST(ConceptTest, BindableWithDelim) {
  // Tuple-like types
  ASSERT_TRUE((argparse::detail::bindable_with_delim<std::tuple<int>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<std::tuple<int, std::string>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<std::tuple<bool, int, double>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<std::pair<int, int>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<std::pair<int, std::string>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<std::pair<double, bool>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<std::array<int, 1>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<std::array<int, 3>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<std::array<double, 3>>));

  // Tuple-like types with custom string-constructible elements
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<std::tuple<StringConstructible>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<std::tuple<StringConvertible>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::pair<StringConstructible, StringConvertible>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::tuple<StringConstructible, int>>));

  // std::optional of tuple-like types
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<std::optional<std::tuple<int>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::tuple<int, std::string>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::tuple<bool, int, double>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::pair<int, std::string>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::pair<double, bool>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::array<int, 1>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::array<double, 3>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::tuple<StringConstructible>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::tuple<StringConvertible>>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<
          std::optional<std::pair<StringConstructible, StringConvertible>>>));

  // std::optional with const-qualified types
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<const std::tuple<int>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<std::tuple<const int>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::optional<const std::pair<const int, const std::string>>>));

  // Containers of tuple-like types
  ASSERT_TRUE((
      argparse::detail::bindable_with_delim<std::vector<std::pair<int, int>>>));
  ASSERT_TRUE((argparse::detail::bindable_with_delim<
               std::list<std::tuple<int, std::string>>>));
  ASSERT_TRUE(
      (argparse::detail::bindable_with_delim<std::vector<std::array<int, 2>>>));

  // Non-tuple-like types are not bindable with delimiter
  ASSERT_FALSE(argparse::detail::bindable_with_delim<int>);
  ASSERT_FALSE(argparse::detail::bindable_with_delim<std::string>);
  ASSERT_FALSE(argparse::detail::bindable_with_delim<StringConstructible>);
  ASSERT_FALSE((argparse::detail::bindable_with_delim<std::optional<int>>));
  ASSERT_FALSE(
      (argparse::detail::bindable_with_delim<std::optional<std::string>>));
  ASSERT_FALSE((argparse::detail::bindable_with_delim<std::vector<int>>));
  ASSERT_FALSE(
      (argparse::detail::bindable_with_delim<std::vector<std::vector<int>>>));

  // Tuple-like types with unsupported element types
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::optional<std::tuple<NonStringConstructible>>>));
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::optional<std::tuple<std::vector<int>>>>));
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::vector<std::tuple<NonStringConstructible>>>));

  // Nested tuple-like types
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::tuple<std::tuple<int, int>, int>>));
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::pair<std::pair<int, int>, int>>));
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::optional<std::tuple<std::tuple<int>>>>));
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::optional<std::pair<std::pair<int, int>, int>>>));
  ASSERT_FALSE((argparse::detail::bindable_with_delim<
                std::optional<std::optional<std::tuple<int>>>>));
}

// ============================================================
// 12. bindable
// ============================================================

TEST(ConceptTest, Bindable) {
  // Basic types
  ASSERT_TRUE(argparse::detail::bindable<int>);
  ASSERT_TRUE(argparse::detail::bindable<std::string>);
  ASSERT_TRUE(argparse::detail::bindable<bool>);
  ASSERT_TRUE(argparse::detail::bindable<double>);

  // Custom types
  ASSERT_TRUE(argparse::detail::bindable<StringConstructible>);
  ASSERT_TRUE(argparse::detail::bindable<StringConvertible>);

  // std::optional types
  ASSERT_TRUE((argparse::detail::bindable<std::optional<int>>));
  ASSERT_TRUE((argparse::detail::bindable<std::optional<std::string>>));
  ASSERT_TRUE((argparse::detail::bindable<std::optional<StringConstructible>>));

  // Tuple-like types
  ASSERT_TRUE((argparse::detail::bindable<std::pair<int, int>>));
  ASSERT_TRUE((argparse::detail::bindable<std::tuple<int, std::string>>));
  ASSERT_TRUE((argparse::detail::bindable<std::array<int, 3>>));

  // Container types
  ASSERT_TRUE((argparse::detail::bindable<std::vector<int>>));
  ASSERT_TRUE((argparse::detail::bindable<std::list<std::string>>));
  ASSERT_TRUE((argparse::detail::bindable<std::set<double>>));
  ASSERT_TRUE((argparse::detail::bindable<std::vector<std::pair<int, int>>>));

  // Unsupported types
  ASSERT_FALSE(argparse::detail::bindable<NonStringConstructible>);
  ASSERT_FALSE(
      (argparse::detail::bindable<std::vector<NonStringConstructible>>));
  ASSERT_FALSE((argparse::detail::bindable<std::vector<std::vector<int>>>));
}

// ============================================================
// 13. Combined scenarios
// ============================================================

TEST(ConceptTest, CombinedScenarios) {
  // Tuple containing container elements
  ASSERT_FALSE((argparse::detail::can_from_string<
                std::tuple<int, std::vector<std::string>>>));

  // Pair containing optional
  ASSERT_FALSE((argparse::detail::can_from_string<
                std::pair<std::optional<int>, std::string>>));

  // Container of optional
  ASSERT_TRUE(
      !argparse::detail::can_from_string<std::vector<std::optional<int>>>);

  // Optional of container
  ASSERT_TRUE(
      !argparse::detail::can_from_string<std::optional<std::vector<int>>>);
}

// ============================================================
// 14. has_to_string / has_to_wstring
// ============================================================

namespace custom_ns {

struct WithToString {};
inline std::string to_string(WithToString const&) { return "custom"; }

struct WithToWstring {};
inline std::wstring to_wstring(WithToWstring const&) { return L"custom"; }

struct WithBoth {};
inline std::string to_string(WithBoth const&) { return "both"; }
inline std::wstring to_wstring(WithBoth const&) { return L"both"; }

struct BadToString {};
inline int to_string(BadToString const&) { return 42; }

struct BadToWstring {};
inline int to_wstring(BadToWstring const&) { return 42; }

}  // namespace custom_ns

TEST(ConceptTest, HasToString) {
  // Types supported by std::to_string
  ASSERT_TRUE(argparse::detail::has_to_string<int>);
  ASSERT_TRUE(argparse::detail::has_to_string<long>);
  ASSERT_TRUE(argparse::detail::has_to_string<long long>);
  ASSERT_TRUE(argparse::detail::has_to_string<unsigned>);
  ASSERT_TRUE(argparse::detail::has_to_string<unsigned long>);
  ASSERT_TRUE(argparse::detail::has_to_string<unsigned long long>);
  ASSERT_TRUE(argparse::detail::has_to_string<float>);
  ASSERT_TRUE(argparse::detail::has_to_string<double>);
  ASSERT_TRUE(argparse::detail::has_to_string<long double>);

  // Types that promote to integral types supported by std::to_string
  ASSERT_TRUE(argparse::detail::has_to_string<bool>);
  ASSERT_TRUE(argparse::detail::has_to_string<char>);
  ASSERT_TRUE(argparse::detail::has_to_string<short>);
  ASSERT_TRUE(argparse::detail::has_to_string<unsigned short>);
  ASSERT_TRUE(argparse::detail::has_to_string<unsigned char>);

  // std::string (via identity overload in argparse)
  ASSERT_TRUE(argparse::detail::has_to_string<std::string>);

  // const char* is implicitly convertible to std::string
  ASSERT_TRUE(argparse::detail::has_to_string<const char*>);

  // Custom types with ADL to_string
  ASSERT_TRUE(argparse::detail::has_to_string<custom_ns::WithToString>);
  ASSERT_TRUE(argparse::detail::has_to_string<custom_ns::WithBoth>);

  // Types without to_string support
  ASSERT_FALSE(argparse::detail::has_to_string<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_to_string<std::vector<int>>);
  ASSERT_FALSE((argparse::detail::has_to_string<std::pair<int, int>>));
  ASSERT_FALSE(argparse::detail::has_to_string<custom_ns::WithToWstring>);

  // to_string exists but returns wrong type
  ASSERT_FALSE(argparse::detail::has_to_string<custom_ns::BadToString>);

  // Unrelated pointer types
  ASSERT_FALSE(argparse::detail::has_to_string<int*>);
}

TEST(ConceptTest, HasToWstring) {
  // Types supported by std::to_wstring
  ASSERT_TRUE(argparse::detail::has_to_wstring<int>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<long long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<unsigned>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<unsigned long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<unsigned long long>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<float>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<double>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<long double>);

  // Types that promote to integral types supported by std::to_wstring
  ASSERT_TRUE(argparse::detail::has_to_wstring<bool>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<char>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<short>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<unsigned short>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<unsigned char>);

  // std::wstring (via identity overload in argparse)
  ASSERT_TRUE(argparse::detail::has_to_wstring<std::wstring>);

  // const wchar_t* is implicitly convertible to std::wstring
  ASSERT_TRUE(argparse::detail::has_to_wstring<const wchar_t*>);

  // Custom types with ADL to_wstring
  ASSERT_TRUE(argparse::detail::has_to_wstring<custom_ns::WithToWstring>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<custom_ns::WithBoth>);

  // Types without to_wstring support
  ASSERT_FALSE(argparse::detail::has_to_wstring<std::string>);
  ASSERT_FALSE(argparse::detail::has_to_wstring<std::vector<int>>);
  ASSERT_FALSE((argparse::detail::has_to_wstring<std::pair<int, int>>));
  ASSERT_FALSE(argparse::detail::has_to_wstring<custom_ns::WithToString>);

  // to_wstring exists but returns wrong type
  ASSERT_FALSE(argparse::detail::has_to_wstring<custom_ns::BadToWstring>);

  // Unrelated pointer types
  ASSERT_FALSE(argparse::detail::has_to_wstring<int*>);
}

TEST(ConceptTest, HasToStringConstVolatile) {
  // Const-qualified types should still work
  ASSERT_TRUE(argparse::detail::has_to_string<const int>);
  ASSERT_TRUE(argparse::detail::has_to_string<const double>);
  ASSERT_TRUE(argparse::detail::has_to_string<const std::string>);

  ASSERT_TRUE(argparse::detail::has_to_wstring<const int>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<const double>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<const std::wstring>);
}

TEST(ConceptTest, HasToStringEnum) {
  enum UnscopedColor { Red, Green, Blue };
  enum class ScopedFruit { Apple, Banana };

  // Unscoped enums implicitly convert to int, so std::to_string works
  ASSERT_TRUE(argparse::detail::has_to_string<UnscopedColor>);
  ASSERT_TRUE(argparse::detail::has_to_wstring<UnscopedColor>);

  // Scoped enums do not implicitly convert, so std::to_string is unavailable
  ASSERT_FALSE(argparse::detail::has_to_string<ScopedFruit>);
  ASSERT_FALSE(argparse::detail::has_to_wstring<ScopedFruit>);
}

// ============================================================
// 15. has_*_memfunc traits
// ============================================================

namespace memfunc_ns {

struct WithToStringMemfunc {
  std::string to_string() const { return "via to_string()"; }
};

struct WithStringMemfunc {
  std::string string() const { return "via string()"; }
};

struct WithToWstringMemfunc {
  std::wstring to_wstring() const { return L"via to_wstring()"; }
};

struct WithWstringMemfunc {
  std::wstring wstring() const { return L"via wstring()"; }
};

struct WithCStrMemfunc {
  const char* c_str() const { return "via c_str()"; }
};

struct WithWCStrMemfunc {
  const wchar_t* c_str() const { return L"via c_str() [wide]"; }
};

struct WithAll {
  std::string to_string() const { return "all"; }
  std::string string() const { return "all"; }
  std::wstring to_wstring() const { return L"all"; }
  std::wstring wstring() const { return L"all"; }
  const char* c_str() const { return "all"; }
};

// Types with wrong return types
struct BadToStringMemfunc {
  int to_string() const { return 42; }
};

struct BadStringMemfunc {
  int string() const { return 42; }
};

struct BadToWstringMemfunc {
  int to_wstring() const { return 42; }
};

struct BadWstringMemfunc {
  int wstring() const { return 42; }
};

struct BadCStrMemfunc {
  int c_str() const { return 42; }
};

// Types with wrong string return types
struct WrongStringReturn {
  std::wstring to_string() const { return L"wrong"; }
};

struct WrongWstringReturn {
  std::string to_wstring() const { return "wrong"; }
};

// Non-const member functions
struct NonConstToString {
  std::string to_string() { return "non-const"; }
};

struct NonConstString {
  std::string string() { return "non-const"; }
};

struct NonConstToWstring {
  std::wstring to_wstring() { return L"non-const"; }
};

struct NonConstWstring {
  std::wstring wstring() { return L"non-const"; }
};

struct NonConstCStr {
  const char* c_str() { return "non-const"; }
};

struct Empty {};

}  // namespace memfunc_ns

TEST(ConceptTest, HasToStringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct to_string() -> std::string
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc<WithToStringMemfunc>);

  // Non-const member function also accepted (tested via T&)
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc<NonConstToString>);

  // Types without to_string() member function
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<std::string>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<WithStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<BadToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<WrongStringReturn>);

  // Const qualification
  ASSERT_TRUE(
      argparse::detail::has_to_string_memfunc<const WithToStringMemfunc>);

  // Volatile qualification is not accepted
  ASSERT_FALSE(
      (argparse::detail::has_to_string_memfunc<volatile WithToStringMemfunc>));
}

TEST(ConceptTest, HasStringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct string() -> std::string
  ASSERT_TRUE(argparse::detail::has_string_memfunc<WithStringMemfunc>);
  ASSERT_TRUE(argparse::detail::has_string_memfunc<WithAll>);

  // Non-const member function also accepted
  ASSERT_TRUE(argparse::detail::has_string_memfunc<NonConstString>);

  // Types without string() member function
  ASSERT_FALSE(argparse::detail::has_string_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<std::string>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_string_memfunc<BadStringMemfunc>);

  // Const qualification
  ASSERT_TRUE(argparse::detail::has_string_memfunc<const WithStringMemfunc>);
}

TEST(ConceptTest, HasToWstringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct to_wstring() -> std::wstring
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc<WithToWstringMemfunc>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc<WithAll>);

  // Non-const member function also accepted
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc<NonConstToWstring>);

  // Types without to_wstring() member function
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<WithWstringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<BadToWstringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<WrongWstringReturn>);

  // Const qualification
  ASSERT_TRUE(
      argparse::detail::has_to_wstring_memfunc<const WithToWstringMemfunc>);
}

TEST(ConceptTest, HasWstringMemfunc) {
  using namespace memfunc_ns;

  // Types with correct wstring() -> std::wstring
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc<WithWstringMemfunc>);
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc<WithAll>);

  // Non-const member function also accepted
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc<NonConstWstring>);

  // Types without wstring() member function
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<BadWstringMemfunc>);

  // Const qualification
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc<const WithWstringMemfunc>);
}

TEST(ConceptTest, HasCStrMemfunc) {
  using namespace memfunc_ns;

  // Types with correct c_str() -> std::string
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<WithCStrMemfunc>);
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<WithAll>);

  // Non-const member function also accepted
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<NonConstCStr>);

  // std::string has c_str() returning const char* (convertible to std::string)
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<std::string>);

  // Types without c_str() member function
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<Empty>);

  // Wrong return type
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<BadCStrMemfunc>);

  // Const qualification
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<const WithCStrMemfunc>);

  // wchar_t overload: WithWCStrMemfunc has c_str() -> const wchar_t*
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<WithWCStrMemfunc>);
  ASSERT_TRUE((argparse::detail::has_c_str_memfunc<WithWCStrMemfunc, wchar_t>));

  // std::wstring has c_str() -> const wchar_t*
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<std::wstring>);
  ASSERT_TRUE((argparse::detail::has_c_str_memfunc<std::wstring, wchar_t>));

  // WithAll returns const char*, not const wchar_t*
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<WithAll>);
  ASSERT_FALSE((argparse::detail::has_c_str_memfunc<WithAll, wchar_t>));
}

TEST(ConceptTest, MemfuncTraitsCombined) {
  using namespace memfunc_ns;

  // WithAll should satisfy all five traits
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc<WithAll>);
  ASSERT_TRUE(argparse::detail::has_string_memfunc<WithAll>);
  ASSERT_TRUE(argparse::detail::has_to_wstring_memfunc<WithAll>);
  ASSERT_TRUE(argparse::detail::has_wstring_memfunc<WithAll>);
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<WithAll>);

  // Each specialized type should only satisfy its own trait
  ASSERT_TRUE(argparse::detail::has_to_string_memfunc<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<WithToStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<WithToStringMemfunc>);

  ASSERT_TRUE(argparse::detail::has_string_memfunc<WithStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<WithStringMemfunc>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<WithStringMemfunc>);

  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<WithCStrMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<WithCStrMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<WithCStrMemfunc>);
  // WithCStrMemfunc returns const char*, not const wchar_t*
  ASSERT_FALSE((argparse::detail::has_c_str_memfunc<WithCStrMemfunc, wchar_t>));

  // WithWCStrMemfunc only satisfies c_str with wchar_t
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<WithWCStrMemfunc>);
  ASSERT_TRUE((argparse::detail::has_c_str_memfunc<WithWCStrMemfunc, wchar_t>));
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<WithWCStrMemfunc>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<WithWCStrMemfunc>);

  // std::string: only has c_str()
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<std::string>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<std::string>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<std::string>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<std::string>);
  ASSERT_TRUE(argparse::detail::has_c_str_memfunc<std::string>);

  // std::wstring: none of the member functions
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<std::wstring>);
  ASSERT_TRUE((argparse::detail::has_c_str_memfunc<std::wstring, wchar_t>));

  // Empty type satisfies none
  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<Empty>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<Empty>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<Empty>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<Empty>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<Empty>);
}

// ============================================================
// 16. to_string / to_wstring runtime dispatch
// ============================================================

TEST(ConceptTest, ToStringViaMemberFunctions) {
  using namespace memfunc_ns;

  WithToStringMemfunc obj1;
  ASSERT_EQ(argparse::detail::to_string(obj1), "via to_string()");

  WithStringMemfunc obj2;
  ASSERT_EQ(argparse::detail::to_string(obj2), "via string()");

  WithCStrMemfunc obj3;
  ASSERT_EQ(argparse::detail::to_string(obj3), "via c_str()");

  std::string s = "hello";
  ASSERT_EQ(argparse::detail::to_string(s), "hello");
}

TEST(ConceptTest, ToWstringViaMemberFunctions) {
  using namespace memfunc_ns;

  WithToWstringMemfunc obj1;
  ASSERT_EQ(argparse::detail::to_wstring(obj1), L"via to_wstring()");

  WithWstringMemfunc obj2;
  ASSERT_EQ(argparse::detail::to_wstring(obj2), L"via wstring()");

  WithWCStrMemfunc obj3;
  ASSERT_EQ(argparse::detail::to_wstring(obj3), L"via c_str() [wide]");

  std::wstring ws = L"hello";
  ASSERT_EQ(argparse::detail::to_wstring(ws), L"hello");
}

TEST(ConceptTest, ToStringCStrWorksForStdString) {
  std::string s = "test";
  ASSERT_EQ(argparse::detail::to_string(s), "test");

  ASSERT_EQ(argparse::detail::to_string(std::string("temp")), "temp");
}

TEST(ConceptTest, ToStringAndToWstringNoMatch) {
  // Verify that types without matching member functions are correctly
  // excluded by the traits (the actual to_string/to_wstring calls are
  // constrained at compile time).

  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<int>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<int>);

  ASSERT_FALSE(argparse::detail::has_to_string_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_string_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_c_str_memfunc<std::wstring>);
  ASSERT_TRUE((argparse::detail::has_c_str_memfunc<std::wstring, wchar_t>));
  ASSERT_FALSE(argparse::detail::has_to_wstring_memfunc<std::wstring>);
  ASSERT_FALSE(argparse::detail::has_wstring_memfunc<std::wstring>);
}

// ============================================================
// 17. Constructor constraints (compile-time verification)
// ============================================================

namespace constructor_test {

struct FromStringType {
  std::string value;
  FromStringType() = default;
  explicit FromStringType(const std::string& s) : value(s) {}
};

using IntPair = std::pair<int, int>;
using StringIntTuple = std::tuple<std::string, int>;

}  // namespace constructor_test

TEST(ConceptTest, OptionConstructorConstraints) {
  using namespace constructor_test;

  argparse::ArgParser parser("test", "Test parser");

  // Single-value option
  {
    FromStringType bind_val{""};
    auto& opt =
        parser.add_option("s,single", "A single value option", bind_val);
    (void)opt;
  }

  // Basic type option
  {
    int bind_val = 0;
    auto& opt = parser.add_option("i,int", "An int option", bind_val);
    (void)opt;
  }

  // std::string option
  {
    std::string bind_val;
    auto& opt = parser.add_option("str", "A string option", bind_val);
    (void)opt;
  }

  // Pair option (with delimiter)
  {
    IntPair bind_val{};
    auto& opt = parser.add_option("pair", "A pair option", bind_val, ',');
    (void)opt;
  }

  // Tuple option (with delimiter)
  {
    StringIntTuple bind_val{};
    auto& opt = parser.add_option("tuple", "A tuple option", bind_val, ',');
    (void)opt;
  }

  // Container of tuple-like types (with delimiter)
  {
    std::vector<IntPair> bind_val;
    auto& opt =
        parser.add_option("vecpair", "A vector of pairs", bind_val, ';');
    (void)opt;
  }
}

TEST(ConceptTest, PositionalConstructorConstraints) {
  using namespace constructor_test;

  argparse::ArgParser parser("test", "Test parser");

  // Single-value positional
  {
    FromStringType bind_val{""};
    auto& pos =
        parser.add_positional("single", "A single value positional", bind_val);
    (void)pos;
  }

  // Pair positional (with delimiter)
  {
    IntPair bind_val{};
    auto& pos =
        parser.add_positional("pair", "A pair positional", bind_val, ',');
    (void)pos;
  }

  // Container positional (without delimiter)
  {
    std::vector<std::string> bind_val;
    auto& pos = parser.add_positional("items", "A vector of strings", bind_val);
    (void)pos;
  }

  // Container of tuple-like types (with delimiter)
  {
    std::vector<StringIntTuple> bind_val;
    auto& pos =
        parser.add_positional("vec_tuple", "A vector of tuples", bind_val, ';');
    (void)pos;
  }
}

TEST(ConceptTest, OptionPositionalMultipleContainer) {
  argparse::ArgParser parser("test", "Test parser");

  // Container option with delimiter
  {
    std::vector<std::pair<int, int>> bind_val;
    auto& opt = parser.add_option("points", "A list of points", bind_val, ';');
    (void)opt;
  }

  // Container option without delimiter
  {
    std::vector<std::string> bind_val;
    auto& opt = parser.add_option("files", "A list of files", bind_val);
    (void)opt;
  }

  // Container positional without delimiter
  argparse::ArgParser parser2("test2", "Test parser 2");
  {
    std::vector<int> bind_val;
    auto& pos = parser2.add_positional("numbers", "List of numbers", bind_val);
    (void)pos;
  }
}

// ============================================================
// 18. extract_value_type / extract_value_type_t
// ============================================================

TEST(ConceptTest, ExtractValueType) {
  // Primary template: T → T for plain types
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<int>, int>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<bool>, bool>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<double>, double>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::string>,
                     std::string>);
  static_assert(std::is_same_v<
                argparse::detail::extract_value_type_t<StringConstructible>,
                StringConstructible>);
  static_assert(std::is_same_v<
                argparse::detail::extract_value_type_t<NonStringConstructible>,
                NonStringConstructible>);

  // std::optional<T> → T (unwraps the optional)
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::optional<int>>,
                     int>);
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::optional<bool>>, bool>);
  static_assert(std::is_same_v<
                argparse::detail::extract_value_type_t<std::optional<double>>,
                double>);
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::optional<std::string>>,
          std::string>);
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::optional<StringConstructible>>,
                               StringConstructible>);

  // from_string_container<T> → T::value_type (extracts element type)
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::vector<int>>,
                     int>);
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::vector<std::string>>,
          std::string>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::list<double>>,
                     double>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::deque<bool>>,
                     bool>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::set<int>>,
                     int>);
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::unordered_set<float>>,
          float>);

  // Containers of tuple-like types — extracts the tuple-like element type
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::vector<std::pair<int, int>>>,
                               std::pair<int, int>>);
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::vector<std::tuple<int, std::string>>>,
                               std::tuple<int, std::string>>);
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::list<std::array<int, 3>>>,
          std::array<int, 3>>);

  // Containers of string-constructible custom types
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::vector<StringConstructible>>,
                               StringConstructible>);
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::set<StringConvertible>>,
          StringConvertible>);

  // std::optional<std::vector<T>> — the std::optional<T> specialization
  // matches first, yielding std::vector<T> (the container type itself,
  // not its value_type)
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::optional<std::vector<int>>>,
                               std::vector<int>>);
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::optional<std::vector<std::string>>>,
                               std::vector<std::string>>);
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::optional<std::list<double>>>,
                               std::list<double>>);

  // Edge case: container of std::optional.
  // std::optional<T> is never can_from_string_without_delim (the from_string
  // function is constrained by !is_optional_v<T>).  Therefore a container
  // whose value_type is std::optional<U> is never a from_string_container,
  // and the primary template yields the container itself.
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::vector<std::optional<int>>>,
                               std::vector<std::optional<int>>>);
  static_assert(std::is_same_v<argparse::detail::extract_value_type_t<
                                   std::deque<std::optional<std::string>>>,
                               std::deque<std::optional<std::string>>>);

  // Tuple-like types are NOT from_string_container, so they hit the
  // primary template and yield themselves.
  static_assert(std::is_same_v<
                argparse::detail::extract_value_type_t<std::pair<int, int>>,
                std::pair<int, int>>);
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::tuple<int, std::string>>,
          std::tuple<int, std::string>>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::array<int, 3>>,
                     std::array<int, 3>>);

  // std::string is excluded from from_string_container by
  // is_non_string_container, so it hits the primary template and stays as
  // std::string.
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::string>,
                     std::string>);
  static_assert(
      std::is_same_v<argparse::detail::extract_value_type_t<std::wstring>,
                     std::wstring>);

  // Non-container, non-optional types that happen to have a ::value_type
  // typedef but are not actually containers (like std::optional itself)
  // — the primary template handles them, yielding the type as-is.
  // std::reference_wrapper, std::function, etc. are not containers.
  static_assert(
      std::is_same_v<
          argparse::detail::extract_value_type_t<std::reference_wrapper<int>>,
          std::reference_wrapper<int>>);
}

TEST(ConceptTest, ExtractValueTypeGtest) {
  // Same checks as static_assert above, but via ASSERT_TRUE for gtest
  // reporting.

  // Plain types → themselves
  ASSERT_TRUE(
      (std::is_same_v<argparse::detail::extract_value_type_t<int>, int>));
  ASSERT_TRUE(
      (std::is_same_v<argparse::detail::extract_value_type_t<double>, double>));

  // std::optional unwrapping
  ASSERT_TRUE((
      std::is_same_v<argparse::detail::extract_value_type_t<std::optional<int>>,
                     int>));
  ASSERT_TRUE(
      (std::is_same_v<
          argparse::detail::extract_value_type_t<std::optional<std::string>>,
          std::string>));

  // Container element extraction
  ASSERT_TRUE(
      (std::is_same_v<argparse::detail::extract_value_type_t<std::vector<int>>,
                      int>));
  ASSERT_TRUE((std::is_same_v<
               argparse::detail::extract_value_type_t<std::vector<std::string>>,
               std::string>));
  ASSERT_TRUE(
      (std::is_same_v<argparse::detail::extract_value_type_t<std::set<double>>,
                      double>));

  // Container of tuple-like
  ASSERT_TRUE((std::is_same_v<argparse::detail::extract_value_type_t<
                                  std::vector<std::pair<int, int>>>,
                              std::pair<int, int>>));
  ASSERT_TRUE((std::is_same_v<argparse::detail::extract_value_type_t<
                                  std::list<std::tuple<int, std::string>>>,
                              std::tuple<int, std::string>>));

  // std::optional of container → container itself (not unwrapped further)
  ASSERT_TRUE((std::is_same_v<argparse::detail::extract_value_type_t<
                                  std::optional<std::vector<int>>>,
                              std::vector<int>>));

  // Container of std::optional:
  // std::optional<T> is never can_from_string_without_delim, so the
  // container itself is yielded by the primary template.
  ASSERT_TRUE((std::is_same_v<argparse::detail::extract_value_type_t<
                                  std::vector<std::optional<int>>>,
                              std::vector<std::optional<int>>>));
  ASSERT_TRUE((std::is_same_v<argparse::detail::extract_value_type_t<
                                  std::deque<std::optional<std::string>>>,
                              std::deque<std::optional<std::string>>>));

  // Tuple-like types → themselves
  ASSERT_TRUE((std::is_same_v<
               argparse::detail::extract_value_type_t<std::pair<int, int>>,
               std::pair<int, int>>));
  ASSERT_TRUE(
      (std::is_same_v<argparse::detail::extract_value_type_t<std::tuple<int>>,
                      std::tuple<int>>));
  ASSERT_TRUE((
      std::is_same_v<argparse::detail::extract_value_type_t<std::array<int, 3>>,
                     std::array<int, 3>>));

  // std::string excluded from container → itself
  ASSERT_TRUE(
      (std::is_same_v<argparse::detail::extract_value_type_t<std::string>,
                      std::string>));
}

// ============================================================
// 19. bindable_flag / bindable_int_flag / bindable_bool_flag
// ============================================================

TEST(ConceptTest, BindableFlag) {
  // bool and int directly
  ASSERT_TRUE((argparse::bindable_flag<bool>));
  ASSERT_TRUE((argparse::bindable_flag<int>));

  // std::optional of bool and int
  ASSERT_TRUE((argparse::bindable_flag<std::optional<bool>>));
  ASSERT_TRUE((argparse::bindable_flag<std::optional<int>>));

  // Non-flag types
  ASSERT_FALSE((argparse::bindable_flag<double>));
  ASSERT_FALSE((argparse::bindable_flag<float>));
  ASSERT_FALSE((argparse::bindable_flag<std::string>));
  ASSERT_FALSE((argparse::bindable_flag<char>));
  ASSERT_FALSE((argparse::bindable_flag<unsigned>));
  ASSERT_FALSE((argparse::bindable_flag<long>));
  ASSERT_FALSE((argparse::bindable_flag<std::optional<double>>));
  ASSERT_FALSE((argparse::bindable_flag<std::optional<std::string>>));
  ASSERT_FALSE((argparse::bindable_flag<std::optional<unsigned>>));
}

TEST(ConceptTest, BindableIntFlag) {
  // int directly
  ASSERT_TRUE((argparse::bindable_int_flag<int>));

  // std::optional of int
  ASSERT_TRUE((argparse::bindable_int_flag<std::optional<int>>));

  // Non-int-flag types
  ASSERT_FALSE((argparse::bindable_int_flag<bool>));
  ASSERT_FALSE((argparse::bindable_int_flag<std::optional<bool>>));
  ASSERT_FALSE((argparse::bindable_int_flag<double>));
  ASSERT_FALSE((argparse::bindable_int_flag<float>));
  ASSERT_FALSE((argparse::bindable_int_flag<std::string>));
  ASSERT_FALSE((argparse::bindable_int_flag<unsigned>));
  ASSERT_FALSE((argparse::bindable_int_flag<long>));
  ASSERT_FALSE((argparse::bindable_int_flag<char>));
  ASSERT_FALSE((argparse::bindable_int_flag<std::optional<double>>));
  ASSERT_FALSE((argparse::bindable_int_flag<std::optional<unsigned>>));
}

TEST(ConceptTest, BindableBoolFlag) {
  // bool directly
  ASSERT_TRUE((argparse::bindable_bool_flag<bool>));

  // std::optional of bool
  ASSERT_TRUE((argparse::bindable_bool_flag<std::optional<bool>>));

  // Non-bool-flag types
  ASSERT_FALSE((argparse::bindable_bool_flag<int>));
  ASSERT_FALSE((argparse::bindable_bool_flag<std::optional<int>>));
  ASSERT_FALSE((argparse::bindable_bool_flag<double>));
  ASSERT_FALSE((argparse::bindable_bool_flag<float>));
  ASSERT_FALSE((argparse::bindable_bool_flag<std::string>));
  ASSERT_FALSE((argparse::bindable_bool_flag<char>));
  ASSERT_FALSE((argparse::bindable_bool_flag<unsigned>));
  ASSERT_FALSE((argparse::bindable_bool_flag<long>));
  ASSERT_FALSE((argparse::bindable_bool_flag<std::optional<double>>));
  ASSERT_FALSE((argparse::bindable_bool_flag<std::optional<std::string>>));
}

TEST(ConceptTest, BindableFlagRelationships) {
  // bindable_bool_flag ⇒ bindable_flag (bool subset)
  ASSERT_TRUE((argparse::bindable_flag<bool>));
  ASSERT_TRUE((argparse::bindable_bool_flag<bool>));

  // bindable_int_flag ⇒ bindable_flag (int subset)
  ASSERT_TRUE((argparse::bindable_flag<int>));
  ASSERT_TRUE((argparse::bindable_int_flag<int>));

  // bindable_bool_flag ⊥ bindable_int_flag (mutually exclusive at type level)
  ASSERT_TRUE((argparse::bindable_bool_flag<bool>));
  ASSERT_FALSE((argparse::bindable_int_flag<bool>));
  ASSERT_TRUE((argparse::bindable_int_flag<int>));
  ASSERT_FALSE((argparse::bindable_bool_flag<int>));

  // Both optional variants
  ASSERT_TRUE((argparse::bindable_flag<std::optional<bool>>));
  ASSERT_TRUE((argparse::bindable_bool_flag<std::optional<bool>>));
  ASSERT_FALSE((argparse::bindable_int_flag<std::optional<bool>>));

  ASSERT_TRUE((argparse::bindable_flag<std::optional<int>>));
  ASSERT_TRUE((argparse::bindable_int_flag<std::optional<int>>));
  ASSERT_FALSE((argparse::bindable_bool_flag<std::optional<int>>));
}

TEST(ConceptTest, BindableFlagCompileTimeConstraint) {
  // Verify that the Flag template only accepts bindable_flag types.
  // This is a compile-time test: if the concept constraint were violated,
  // the code would not compile.
  argparse::ArgParser parser("test", "Test parser");

  // bool flag (bindable_bool_flag and bindable_flag)
  {
    bool bind_val = false;
    auto& flag = parser.add_flag("f,flag", "A bool flag", bind_val);
    (void)flag;
  }

  // int flag (bindable_int_flag and bindable_flag)
  {
    int bind_val = 0;
    auto& flag = parser.add_flag("i,int", "An int flag", bind_val);
    (void)flag;
  }

  // std::optional<bool> flag (bindable_bool_flag and bindable_flag)
  {
    std::optional<bool> bind_val;
    auto& flag = parser.add_flag("o,opt", "An optional bool flag", bind_val);
    (void)flag;
  }

  // std::optional<int> flag (bindable_int_flag and bindable_flag)
  {
    std::optional<int> bind_val;
    auto& flag = parser.add_flag("n,num", "An optional int flag", bind_val);
    (void)flag;
  }
}

TEST(ConceptTest, BindableFlagNegativeFlag) {
  // Test that add_negative_flag works with bindable_bool_flag and
  // bindable_int_flag constrained types.
  argparse::ArgParser parser("test", "Test parser");

  // Negative bool flag
  {
    bool bind_val = true;
    auto& flag =
        parser.add_negative_flag("n,neg", "A negative bool flag", bind_val);
    (void)flag;
  }

  // Negative int flag
  {
    int bind_val = 10;
    auto& flag =
        parser.add_negative_flag("v,val", "A negative int flag", bind_val);
    (void)flag;
  }

  // Negative std::optional<bool> flag
  {
    std::optional<bool> bind_val;
    auto& flag = parser.add_negative_flag(
        "no,neg-opt", "A negative optional bool flag", bind_val);
    (void)flag;
  }

  // Negative std::optional<int> flag
  {
    std::optional<int> bind_val;
    auto& flag = parser.add_negative_flag(
        "nv,neg-val", "A negative optional int flag", bind_val);
    (void)flag;
  }
}
