#include "SpeculationContext.hpp"
#include <iostream>

namespace solver {

std::ostream &
operator<<(std::ostream & os, SpeculationContext const & sc) {

  os << "Context:[\n\tdepth:    " << sc.depth << "\n"
     << "solution: <solution>\n"
     << "board:    " << sc.board << "first_move:  " << sc.first_move << "\n";
  return os;
}

} // namespace solver
