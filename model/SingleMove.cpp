#include "SingleMove.hpp"
#include <fmt/core.h>
#include <iostream>

namespace model {

std::ostream &
operator<<(std::ostream & os, SingleMove const & move) {
  return os << "SingleMove<" << to_string(move.action_) << ", "
            << to_string(move.state_) << "(" << move.row_ << ", " << move.col_
            << ")>";
}

std::string
to_string(SingleMove const & move) {
  return fmt::format("{},{},{},{}",
                     to_string(move.action_),
                     to_string(move.state_),
                     move.row_,
                     move.col_);
}

// expecting a 4-tuple string "<action>,<state>,<row>,<col>"
SingleMove
get_move_from_string(std::string_view str) {
  if (not str.empty()) {

    auto tok = [&str, view = std::string_view{str}]() mutable {
      auto off = view.find(',');
      if (off == std::string_view::npos) {
        return view;
      }
      std::string_view result(view.begin(), view.begin() + off);
      view.remove_prefix(off + 1);
      return result;
    };

    auto action = tok();
    auto state  = tok();
    auto row    = tok();
    auto col    = tok();

    return SingleMove{get_action_from_string(action),
                      get_state_from_string(state),
                      std::atoi(row.data()),
                      std::atoi(col.data())};
  }
  throw std::runtime_error("invalid move string: " +
                           std::string(str.data(), str.size()));
}

} // namespace model
