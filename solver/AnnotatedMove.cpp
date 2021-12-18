#include "AnnotatedMove.hpp"
#include <iostream>

namespace solver {

std::ostream &
operator<<(std::ostream & os, AnnotatedMove const & solution_move) {
  fmt::print(os, "{}", solution_move);
  return os;
}

} // namespace solver
