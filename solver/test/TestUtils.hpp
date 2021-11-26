#pragma once

#include "CellState.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "Solution.hpp"

namespace solver::test {

AnnotatedMove
make_cell_at(model::CellState cell,
             model::Coord     where,
             DecisionType     why,
             MoveMotive       motive,
             model::OptCoord  ref_location) {
  return AnnotatedMove{
      model::SingleMove{
          model::Action::Add, model::CellState::Empty, cell, where},
      why,
      motive,
      ref_location};
}

AnnotatedMove
bulb_at(model::Coord    where,
        DecisionType    why,
        MoveMotive      motive,
        model::OptCoord ref_location = std::nullopt) {
  return make_cell_at(model::CellState::Bulb, where, why, motive, ref_location);
}

AnnotatedMove
mark_at(model::Coord    where,
        DecisionType    why,
        MoveMotive      motive,
        model::OptCoord ref_location = std::nullopt) {
  return make_cell_at(model::CellState::Mark, where, why, motive, ref_location);
}

} // namespace solver::test
