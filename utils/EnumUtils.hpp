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

#define DECLARE_FORMATTER(EnumT)                                               \
  template <>                                                                  \
  struct fmt::formatter<EnumT> : enumutils::EnumFormatter<EnumT> {}

namespace enumutils {

template <typename T>
concept Stringable = requires(T obj) {
  { to_string(obj) } -> std::convertible_to<std::string>;
};

template <typename T>
concept Charable = requires(T obj) {
  { to_char(obj) } -> std::convertible_to<char>;
};

template <typename EnumT>
requires std::is_enum_v<EnumT>
struct EnumFormatter {
  char type = '?';

  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    auto it = ctx.begin();
    if (it == ctx.end()) {
      if constexpr (Stringable<EnumT>) {
        type = 's';
      }
    }
    else {
      switch (*it) {
        case 's':
          if constexpr (Stringable<EnumT>) {
            type = 's';
            ++it;
          }
          break;

        case 'c':
          if constexpr (Charable<EnumT>) {
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
        if constexpr (Stringable<EnumT>) {
          return fmt::format_to(ctx.out(), "{:s}", to_string(anEnum));
        }
      case 'c':
        if constexpr (Charable<EnumT>) {
          return fmt::format_to(ctx.out(), "{:c}", to_char(anEnum));
        }
      case 'd':
        return fmt::format_to(ctx.out(), "{:d}", +anEnum);
    }
    throw std::runtime_error("Unreachable");
  }
};

} // namespace enumutils
