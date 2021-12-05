#pragma once

#include <concepts>
#include <fmt/format.h>
#include <stdexcept>
#include <type_traits>

template <typename EnumT>
requires std::is_enum_v<EnumT>
auto
operator+(EnumT e) {
  return static_cast<std::underlying_type_t<EnumT>>(e);
}

#define DEFINE_ENUM_BIT_OPERATIONS(ENUM_NAME)                                  \
  constexpr auto operator+(ENUM_NAME cell) {                                   \
    using T = std::underlying_type_t<ENUM_NAME>;                               \
    return static_cast<T>(cell);                                               \
  }                                                                            \
  constexpr ENUM_NAME operator|(ENUM_NAME lhs, ENUM_NAME rhs) {                \
    return ENUM_NAME(+lhs | +rhs);                                             \
  }                                                                            \
  constexpr ENUM_NAME & operator|=(ENUM_NAME & lhs, ENUM_NAME rhs) {           \
    lhs = lhs | rhs;                                                           \
    return lhs;                                                                \
  }                                                                            \
  constexpr ENUM_NAME operator&(ENUM_NAME lhs, ENUM_NAME rhs) {                \
    return ENUM_NAME(+lhs & +rhs);                                             \
  }                                                                            \
  constexpr ENUM_NAME & operator&=(ENUM_NAME & lhs, ENUM_NAME rhs) {           \
    lhs = lhs & rhs;                                                           \
    return lhs;                                                                \
  }                                                                            \
  constexpr bool contains_all(ENUM_NAME value, ENUM_NAME bits) {               \
    return (value & bits) == bits;                                             \
  }

namespace enumutils {

template <typename T>
concept Stringable = requires(T obj) {
  { to_string(obj) } -> std::convertible_to<std::string>;
};

template <typename T>
concept Charable = requires(T obj) {
  { to_char(obj) } -> std::convertible_to<char>;
};

} // namespace enumutils

template <typename EnumT>
requires std::is_enum_v<EnumT>
struct fmt::formatter<EnumT> {

  char type = '?';

  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    auto it = ctx.begin();
    if (it == ctx.end()) {
      if constexpr (enumutils::Stringable<EnumT>) {
        type = 's';
      }
    }
    else {
      switch (*it) {
        case 's':
          if constexpr (enumutils::Stringable<EnumT>) {
            type = 's';
            ++it;
          }
          break;

        case 'c':
          if constexpr (enumutils::Charable<EnumT>) {
            type = 'c';
            ++it;
          }
          break;

        case 'd':
          type = 'd';
          ++it;
      }
    }
    if (type == '?') {
      throw std::runtime_error(
          "Enum doesn't support operations for given format");
    }
    if (it != ctx.end() && *it != '}') {
      throw std::runtime_error("invalid format");
    }
    return it;
  }

  template <typename FormatContext>
  auto
  format(EnumT const & anEnum, FormatContext & ctx) {
    switch (type) {
      case 's':
        if constexpr (enumutils::Stringable<EnumT>) {
          return fmt::format_to(ctx.out(), "{:s}", to_string(anEnum));
        }
      case 'c':
        if constexpr (enumutils::Charable<EnumT>) {
          return fmt::format_to(ctx.out(), "{:c}", to_char(anEnum));
        }
      case 'd':
        return fmt::format_to(ctx.out(), "{:d}", +anEnum);
    }
    throw std::runtime_error("Unreachable");
  }
};
