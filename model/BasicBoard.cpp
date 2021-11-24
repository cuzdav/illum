#include "BasicBoard.hpp"
#include <iostream>

namespace {
char const * const indent = "    ";
}

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
    const int cell_width = 1 + (board.width() > 9);

    fmt::format_to(ctx.out(), "{}{:{}}  ", indent, "", cell_width + 1);
    for (int i = 0; i < board.width(); ++i) {
      fmt::format_to(ctx.out(), "{:^{}}", i, cell_width + 2);
    }
    fmt::format_to(ctx.out(), "\n");

    fmt::format_to(ctx.out(), "{}{: <{}}", indent, "", cell_width + 1);
    fmt::format_to(ctx.out(), "{:-<{}}", "", (cell_width + 2) * board.width());
    fmt::format_to(ctx.out(), "\n");

    for (int i = 0, r = board.width(), e = r * board.height(); i < e; ++i) {
      auto state = board.get_cell_flat_unchecked(i);
      if (r == board.width()) {
        fmt::format_to(
            ctx.out(), "{} {:{}}| ", indent, i / board.width(), cell_width);
      }

      if (model::Coord coord{i / board.width(), i % board.width()};
          board.get_last_move_coord() &&
          (board.get_last_move_coord() == coord)) {
        fmt::format_to(ctx.out(), "[{:{}c}]", to_char(state), cell_width);
      }
      else {
        fmt::format_to(ctx.out(), " {:{}c} ", to_char(state), cell_width);
      }
      if (--r == 0) {
        fmt::format_to(ctx.out(), "\n");
        r = board.width();
      }
    }
    return fmt::format_to(ctx.out(), "{}", indent);
  }
};

namespace model {
std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {
  os << fmt::format("{}", board);
  return os;
}

} // namespace model
