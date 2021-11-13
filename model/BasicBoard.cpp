#include "BasicBoard.hpp"
#include <iostream>

namespace model {

std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {
  os << "Board: [\n";

  for (int i = 0, r = board.width_, e = r * board.height_; i < e; ++i) {
    auto state = board.cells_[i];
    if (r == board.width_) {
      os << "    ";
    }
    os << to_char(state);
    if (--r == 0) {
      os << "\n";
      r = board.width_;
    }
  }
  os << "]";
  return os;
}
} // namespace model
