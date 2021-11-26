#include "AnalysisBoard.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "Direction.hpp"
#include "utils/DebugLog.hpp"
#include <iostream>

namespace solver {

using model::CellState;
using model::Coord;
using model::Direction;

PositionBoard::PositionBoard(model::BasicBoard const & current) : board_{} {

  board_.reset(current.height(), current.width());

  // first copy the walls and update counts
  current.visit_board([&](model::Coord coord, auto cell) {
    if (model::is_dynamic_entity(cell)) {
      needs_illum_count_++;
    }
    else if (is_wall(cell)) {
      walls_with_deps_count_ += model::is_wall_with_deps(cell);
      board_.set_cell(coord, cell);
    }
  });

  // now just play the moves from their board into ours.
  current.visit_board([&](model::Coord coord, auto cell) {
    if (has_error()) {
      return model::STOP_VISITING;
    }
    if (model::is_bulb(cell)) {
      add_bulb(coord);
    }
    else if (is_mark(cell)) {
      add_mark(coord);
    }
    return model::KEEP_VISITING;
  });

  assert(current.width() == board_.width());
  assert(current.height() == board_.height());
}

model::BasicBoard const &
PositionBoard::board() const {
  return board_;
}

model::BasicBoard &
PositionBoard::mut_board() {
  return board_;
}

bool
PositionBoard::is_solved() const {
  return has_error_ == false && needs_illum_count_ == 0 &&
         walls_with_deps_count_ == 0;
}

int
PositionBoard::num_cells_needing_illumination() const {
  return needs_illum_count_;
}

int
PositionBoard::num_walls_with_deps() const {
  return walls_with_deps_count_;
}

int
PositionBoard::width() const {
  return board_.width();
}
int
PositionBoard::height() const {
  return board_.height();
}

bool
PositionBoard::has_error() const {
  return has_error_;
}

DecisionType
PositionBoard::decision_type() const {
  return decision_type_;
}

model::OptCoord
PositionBoard::get_ref_location() const {
  return ref_location_;
}

void
PositionBoard::set_has_error(bool yn, DecisionType decision_type) {
  has_error_     = yn;
  decision_type_ = decision_type;
}

int
PositionBoard::needs_illum_count() const {
  return needs_illum_count_;
}

int
PositionBoard::walls_with_deps_count() const {
  return walls_with_deps_count_;
}

// Handles a state change when a light is played:
// 1) did the light block a face that makes the wall impossible to satisfy?
// 2) did the bulb over-subscribe the wall?
// 3) did the bulb just satisfy the wall, and we should reduce our counter?
void
PositionBoard::update_wall(model::Coord wall_coord,
                           CellState    wall_cell,
                           CellState    play_cell,
                           bool         coord_is_adjacent_to_play) {
  if (int deps = model::num_wall_deps(wall_cell); deps > 0) {
    int empty_neighbors = 0;
    int bulb_neighbors  = 0;
    board().visit_adjacent(wall_coord,
                           [&](model::Coord coord, CellState neighbor) {
                             bulb_neighbors += neighbor == CellState::Bulb;
                             empty_neighbors += neighbor == CellState::Empty;
                           });

    if (bulb_neighbors > deps) {
      has_error_     = true;
      decision_type_ = DecisionType::WALL_HAS_TOO_MANY_BULBS;
      ref_location_  = wall_coord;
    }
    else if ((deps - bulb_neighbors) > empty_neighbors) {
      has_error_     = true;
      decision_type_ = DecisionType::WALL_CANNOT_BE_SATISFIED;
      ref_location_  = wall_coord;
    }
    else if (bulb_neighbors == deps && coord_is_adjacent_to_play &&
             play_cell == CellState::Bulb) {
      // just-played bulb satisfied this wall, so decrement counter
      walls_with_deps_count_--;
    }
  }
}

bool
PositionBoard::add_bulb(model::Coord bulb_coord) {
  CellState bulb_target = board().get_cell(bulb_coord);
  // A mark's job is to prevent playing a bulb there, so don't allow it.
  // Otherwise, allow them to play a mistake, but only one.
  if (bulb_target != (bulb_target & (CellState::Empty | CellState::Illum))) {
    return false;
  }
  mut_board().set_cell(bulb_coord, CellState::Bulb);
  needs_illum_count_ -= bulb_target == CellState::Empty;

  // update walls immediately adjacent to the bulb
  board().visit_adjacent(
      bulb_coord, [&](model::Coord adj_coord, CellState neighbor) {
        update_wall(adj_coord, neighbor, CellState::Bulb, true);
      });

  // now emit light outwards, and see if it affects walls nearby
  board().visit_rows_cols_outward(
      bulb_coord, [&](model::Coord illum_coord, CellState cell) {
        if (is_illuminable(cell)) {
          mut_board().set_cell(illum_coord, model::CellState::Illum);
          needs_illum_count_--;

          // illuminating a cell adjacent to a wall with deps affects it
          board().visit_adjacent(
              illum_coord, [&](model::Coord adj_coord, CellState neighbor) {
                update_wall(adj_coord, neighbor, CellState::Illum, false);
              });
        }
        else if (cell == CellState::Bulb) {
          decision_type_ = DecisionType::BULBS_SEE_EACH_OTHER;
          has_error_     = true;
        }
      });

  return true;
}

bool
PositionBoard::add_mark(model::Coord mark_coord) {
  CellState mark_target = board().get_cell(mark_coord);
  if (not is_empty(mark_target)) {
    return false;
  }
  mut_board().set_cell(mark_coord, CellState::Mark);

  // update walls immediately adjacent to the mark
  board().visit_adjacent(
      mark_coord, [&](model::Coord neighbor_coord, CellState neighbor_cell) {
        update_wall(neighbor_coord, neighbor_cell, CellState::Mark, true);
      });
  return true;
}

bool
PositionBoard::apply_move(const model::SingleMove & move) {
  assert(move.action_ == model::Action::Add);

  if (move.to_ == CellState::Bulb) {
    add_bulb(move.coord_);
    return true;
  }
  if (move.to_ == CellState::Mark) {
    add_mark(move.coord_);
    return true;
  }
  else {
    return false;
  }
}

std::ostream &
operator<<(std::ostream & os, PositionBoard const & pos_board) {
  os << "PositionBoard{NeedsIllum=" << pos_board.needs_illum_count()
     << ", Solved=" << pos_board.is_solved()
     << ", HasError=" << pos_board.has_error() << ", " << pos_board.board()
     << "}";
  return os;
}

} // namespace solver
