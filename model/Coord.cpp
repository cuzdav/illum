#include "Coord.hpp"
#include <iostream>

namespace model {

std::ostream &
operator<<(std::ostream & os, Coord coord) {
  return os << "(Row: " << static_cast<int>(coord.row_)
            << ", Col: " << static_cast<int>(coord.col_) << ')';
}
} // namespace model
