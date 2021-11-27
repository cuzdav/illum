#include "AnnotatedMove.hpp"
#include <iostream>

namespace solver {

std::ostream &
operator<<(std::ostream & os, AnnotatedMove const & solution_move) {
  os << "{Moving: " << solution_move.next_move << " ==> "
     << to_string(solution_move.reason);
  if (solution_move.reference_location.has_value()) {
    os << " [In reference to " << *solution_move.reference_location << "]";
  }
  os << "}";
  return os;
}

} // namespace solver
