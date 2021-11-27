#pragma once

#include "Coord.hpp"
#include "DecisionType.hpp"
#include "PositionBoard.hpp"
#include "SingleMove.hpp"
#include <vector>

namespace solver {

// Speculation involves "trying" to place a bulb (or mark) in each empty cell,
// and playing out the trivial/forced moves as a result to see if it causes a
// contradiction.
struct SpeculationContext;
using SpeculationContexts = std::vector<SpeculationContext>;

struct SpeculationContext {
  int               depth;
  PositionBoard     board;
  model::SingleMove first_move;
  DecisionType      decision_type = DecisionType::NONE;
  model::OptCoord   ref_location;
};

std::ostream & operator<<(std::ostream & os, SpeculationContext const & sc);

} // namespace solver
