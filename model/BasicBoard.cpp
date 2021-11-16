#include "BasicBoard.hpp"
#include <iostream>

namespace model {

char const * const indent = "    ";

std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {

  os << indent << "Board: [\n";

  for (int i = 0, r = board.width_, e = r * board.height_; i < e; ++i) {
    auto state = board.cells_[i];
    if (r == board.width_) {
      os << indent;
    }
    os << to_char(state);
    if (--r == 0) {
      os << "\n";
      r = board.width_;
    }
  }
  os << indent << "]";
  return os;
}

} // namespace model
