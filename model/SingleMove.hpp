#pragma once

#include "Action.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include <fmt/format.h>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <type_traits>

namespace model {

struct SingleMove {
  Action    action_;
  CellState from_;
  CellState to_;
  Coord     coord_;

  bool operator==(SingleMove const &) const = default;

  friend std::ostream & operator<<(std::ostream &, SingleMove const &);
};

SingleMove get_move_from_string(std::string_view str);

std::string to_string(SingleMove const & move);

} // namespace model

template <>
struct std::hash<::model::SingleMove> {
  size_t
  operator()(const ::model::SingleMove & m) const {
    static_assert(sizeof(::model::SingleMove) <= sizeof(size_t));
    static_assert(sizeof(::model::Coord::row_) == 1);
    static_assert(sizeof(::model::Coord::col_) == 1);

    size_t result = 0;
    char   buf[5]{};
    buf[0] = to_char(m.action_);
    buf[1] = to_char(m.from_);
    buf[2] = to_char(m.to_);
    buf[3] = m.coord_.row_;
    buf[4] = m.coord_.col_;
    memcpy(&result, buf, sizeof(buf));
    return result;
  }
};

template <>
struct fmt::formatter<::model::SingleMove> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(model::SingleMove const & single_move, FormatContext & ctx) {
    return fmt::format_to(ctx.out(),
                          "({}: {} {}->{})",
                          single_move.coord_,
                          to_string(single_move.action_),
                          to_string(single_move.from_),
                          to_string(single_move.to_));
  }
};
