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

  auto                  operator<=>(SingleMove const &) const = default;
  friend std::ostream & operator<<(std::ostream &, SingleMove const &);
};

SingleMove get_move_from_string(std::string_view str);

std::string to_string(SingleMove const & move);

} // namespace model

template <>
struct std::hash<::model::SingleMove>;

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
                          "({}: {}->{} at {})",
                          to_string(single_move.action_),
                          to_string(single_move.from_),
                          to_string(single_move.to_),
                          single_move.coord_);
  }
};
