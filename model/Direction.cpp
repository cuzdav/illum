#include "Direction.hpp"
#include <iostream>

namespace model {

std::ostream &
operator<<(std::ostream & os, Direction d) {
  return os << to_string(d);
}

} // namespace model
