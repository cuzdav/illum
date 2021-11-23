#include "BasicBoard.hpp"
#include <iostream>

namespace model {

char const * const indent = "    ";

std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {

  os << indent << "Board: \n";

  for (int i = 0, r = board.width_, e = r * board.height_; i < e; ++i) {
    auto state = board.cells_[i];
    if (r == board.width_) {
      os << indent;
    }

    if ((board.get_last_move_coord()) &&
        (board.get_last_move_coord() ==
         model::Coord{i / board.width_, i % board.width_})) {
      os << '[' << to_char(state) << ']';
    }
    else {
      os << ' ' << to_char(state) << ' ';
    }

    if (--r == 0) {
      os << "\n";
      r = board.width_;
    }
  }
  os << indent;
  return os;
}

} // namespace model

template <>
struct fmt::formatter<::model::BasicBoard> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::model::BasicBoard const & board, FormatContext & ctx) {
    fmt::format_to(ctx.out(), "Board:\n");
    for (int i = 0, r = board.width(), e = r * board.height(); i < e; ++i) {
      auto state = board.get_cell_flat_unchecked(i);
      if (r == board.width()) {
        fmt::format_to(ctx.out(), "{}", ::model::indent);
      }

      if (model::Coord coord{i / board.width(), i % board.width()};
          board.get_last_move_coord() &&
          (board.get_last_move_coord() == coord)) {
        fmt::format_to(ctx.out(), "[{}]", to_char(state));
      }
      else {
        fmt::format_to(ctx.out(), " {} ", to_char(state));
      }
      if (--r == 0) {
        fmt::format_to(ctx.out(), "\n");
        r = board.width();
      }
    }
    return fmt::format_to(ctx.out(), "{}", ::model::indent);
  }
};
