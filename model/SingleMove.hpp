#pragma once

#include "Action.hpp"
#include "CellState.hpp"
#include <iosfwd>
#include <string>
#include <string_view>

namespace model {

struct SingleMove {
  Action    action_;
  CellState state_;
  int       row_;
  int       col_;

  bool operator==(SingleMove const &) const = default;

  friend std::ostream & operator<<(std::ostream &, SingleMove const &);
};

SingleMove get_move_from_string(std::string_view str);

std::string to_string(SingleMove const & move);

} // namespace model
