#include "SingleMove.hpp"
#include "Coord.hpp"
#include <fmt/core.h>
#include <iostream>

namespace model {

std::ostream &
operator<<(std::ostream & os, SingleMove const & move) {
  return os << "SingleMove<" << to_string(move.action_) << ", "
            << to_string(move.from_) << ", " << to_string(move.to_) << ", "
            << move.coord_ << ")>";
}

std::string
to_string(SingleMove const & move) {
  return fmt::format("{},{},{},{},{}",
                     to_string(move.action_),
                     to_string(move.from_),
                     to_string(move.to_),
                     move.coord_.row_,
                     move.coord_.col_);
}

// expecting a 5-tuple string "<action>,<from-state>,<to-state>,<row>,<col>"
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

    auto action     = tok();
    auto from_state = tok();
    auto to_state   = tok();
    auto row        = tok();
    auto col        = tok();

    return SingleMove{get_action_from_string(action),
                      get_state_from_string(from_state),
                      get_state_from_string(to_state),
                      Coord{std::atoi(row.data()), std::atoi(col.data())}};
  }
  throw std::runtime_error("invalid move string: " +
                           std::string(str.data(), str.size()));
}

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
