#pragma once

#include "Coord.hpp"
#include "DecisionType.hpp"
#include "PositionBoard.hpp"
#include "SingleMove.hpp"
#include <optional>
#include <vector>

namespace solver {

// Speculation involves "trying" to place a bulb (or mark) in each empty cell,
// and playing out the trivial/forced moves as a result to see if it causes a
// contradiction.
struct SpeculationContext;
using SpeculationContexts = std::vector<SpeculationContext>;

struct SpeculationContext {
  SpeculationContext(int                   depth,
                     PositionBoard const & board,
                     model::SingleMove     first_move,
                     DecisionType          decision_type = DecisionType::NONE,
                     model::OptCoord       ref_location  = std::nullopt)
      : depth{depth}
      , board{board}
      , first_move{first_move}
      , decision_type{decision_type}
      , ref_location{ref_location} {}

  int               depth;
  PositionBoard     board;
  model::SingleMove first_move;
  DecisionType      decision_type;
  model::OptCoord   ref_location;
};

std::ostream & operator<<(std::ostream & os, SpeculationContext const & sc);

} // namespace solver
