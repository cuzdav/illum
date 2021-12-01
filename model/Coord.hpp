#pragma once

#include <cassert>
#include <cstdint>
#include <fmt/format.h>

#include <functional>
#include <iosfwd>
#include <limits>
#include <optional>

namespace model {

class Coord {
public:
  std::int8_t row_;
  std::int8_t col_;

  constexpr Coord() : row_{-1}, col_{-1} {}
  constexpr Coord(int row, int col) : row_{narrow(row)}, col_{narrow(col)} {}

  constexpr bool
  in_range(int height, int width) const {
    return row_ >= 0 && widen(row_) < height && col_ >= 0 &&
           widen(col_) < width;
  }

  friend Coord
  operator+(Coord lhs, Coord rhs) {
    return {lhs.row_ + rhs.row_, lhs.col_ + rhs.col_};
  }

  friend Coord
  operator-(Coord lhs, Coord rhs) {
    return {lhs.row_ - rhs.row_, lhs.col_ - rhs.col_};
  }

  friend auto operator<=>(Coord lhs, Coord rhs) = default;

  friend std::ostream & operator<<(std::ostream & os, Coord coord);

private:
  constexpr static std::int8_t
  narrow(int x) {
    assert(x <= std::numeric_limits<std::int8_t>::max());
    return static_cast<std::int8_t>(x);
  }

  constexpr static int
  widen(std::int8_t x) {
    return static_cast<int>(x);
  }
};

using OptCoord = std::optional<model::Coord>;

} // namespace model

template <>
struct fmt::formatter<model::Coord> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(model::Coord const & coord, FormatContext & ctx) {
    return fmt::format_to(ctx.out(), "[r:{},c:{}]", coord.row_, coord.col_);
  }
};

template <>
struct fmt::formatter<model::OptCoord> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(model::OptCoord const & coord, FormatContext & ctx) {
    if (coord) {
      return fmt::format_to(ctx.out(), "[r:{},c:{}]", coord->row_, coord->col_);
    }
    else {
      return fmt::format_to(ctx.out(), "(none)");
    }
  }
};

template <>
struct std::hash<::model::Coord> {
  size_t
  operator()(::model::Coord const & coord) const {
    return (size_t(coord.row_) << 1) + size_t(coord.col_);
  }
};
