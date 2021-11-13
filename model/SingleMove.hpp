#pragma once

#include "Action.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include <fmt/format.h>
#include <iosfwd>
#include <string>
#include <string_view>

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

namespace fmt {

template <>
struct formatter<model::SingleMove> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(model::SingleMove const & single_move, FormatContext & ctx) {
    return fmt::format_to(ctx.out(),
                          "([r:{},c:{}]: {} {}->{})",
                          single_move.coord_.row_,
                          single_move.coord_.col_,
                          to_string(single_move.action_),
                          to_string(single_move.from_),
                          to_string(single_move.to_));
  }
};

} // namespace fmt
