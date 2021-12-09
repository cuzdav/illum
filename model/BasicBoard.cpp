#include "BasicBoard.hpp"
#include <iostream>

namespace model {

std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {
  os << fmt::format("{}", board);
  return os;
}

} // namespace model
