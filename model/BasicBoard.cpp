#include "BasicBoard.hpp"
#include <iostream>

namespace model {

std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {
  fmt::print(os, "{}", board);
  return os;
}

} // namespace model
