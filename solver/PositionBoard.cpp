#include "AnalysisBoard.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "DebugLog.hpp"
#include "Direction.hpp"

namespace solver {

using model::CellState;
using model::Coord;
using model::Direction;

PositionBoard::PositionBoard(model::BasicBoard const & current) {
  board().visit_board([&](model::Coord coord, auto cell) {
    if (model::is_illumable(cell)) {
      needs_illum_count_++;
    }
    else if (is_wall_with_deps(cell)) {
      switch (compute_wall_state(coord, cell)) {
        case WallState::Unsatisfied:
          walls_with_deps_count_++;
          break;

        case WallState::Satisfied:
          break;

        case WallState::Error:
          has_error_ = true;
          break;
      }
    }
    else if (is_bulb(cell)) {
      auto bulb_checker = [&](auto, CellState other) {
        has_error_ |= is_bulb(other);
      };
      // If error between X-Y not caught with X, it will be caught with Y
      board().visit_col_below(coord, bulb_checker);
      board().visit_row_right_of(coord, bulb_checker);
    }
  });
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

bool
PositionBoard::has_error() const {
  return has_error_;
}

void
PositionBoard::set_has_error(bool yn) {
  has_error_ = yn;
}

int
PositionBoard::needs_illum_count() const {
  return needs_illum_count_;
}

int
PositionBoard::walls_with_deps_count() const {
  return walls_with_deps_count_;
}

PositionBoard::WallState
PositionBoard::compute_wall_state(model::Coord     wall_coord,
                                  model::CellState wall_cell) const {
  int deps = model::num_wall_deps(wall_cell);

  // count empty adjacent cells and bulbs to this wall
  int empty_neighbors = 0;
  int bulb_neighbors  = 0;
  board().visit_adjacent(wall_coord,
                         [&](model::Coord coord, CellState neighbor) {
                           bulb_neighbors += neighbor == CellState::Bulb;
                           empty_neighbors += neighbor == CellState::Empty;
                         });

  if (bulb_neighbors > deps || (deps - bulb_neighbors) > empty_neighbors) {
    // there are too many bulbs around this wall, or it cannot be satisfied
    // due to marks/illuminations covering too many sides.
    return WallState::Error;
  }

  return bulb_neighbors == deps ? WallState::Satisfied : WallState::Unsatisfied;
}

void
PositionBoard::update_wall(model::Coord wall_coord,
                           CellState    wall_cell,
                           CellState    play_cell,
                           bool         coord_is_adjacent_to_play) {
  // did this play add the final bulb to satisfy the wall deps? If so, account
  // for it in our stats.
  if (model::is_wall_with_deps(wall_cell)) {
    switch (compute_wall_state(wall_coord, wall_cell)) {
      case WallState::Satisfied:
        if (coord_is_adjacent_to_play && play_cell == CellState::Bulb) {
          walls_with_deps_count_--;
        }
        break;
      case WallState::Unsatisfied:
        break;
      case WallState::Error:
        has_error_ = true;
        break;
    }
  }
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

bool
PositionBoard::add_bulb(model::Coord bulb_coord) {
  CellState bulb_target = board().get_cell(bulb_coord);
  if (not is_empty(bulb_target)) {
    return false;
  }
  mut_board().set_cell(bulb_coord, CellState::Bulb);
  needs_illum_count_--;

  // update walls immediately adjacent to the bulb
  board().visit_adjacent(
      bulb_coord, [&](model::Coord adj_coord, CellState neighbor) {
        update_wall(adj_coord, neighbor, CellState::Bulb, true);
      });

  // now emit light outwards, and see if it affects walls nearby
  board().visit_rows_cols_outward(
      bulb_coord,
      [&](model::Direction dir, model::Coord illum_coord, CellState cell) {
        if (is_illumable(cell)) {
          mut_board().set_cell(illum_coord, model::CellState::Illum);
          needs_illum_count_--;

          // illuminating a cell adjacent to a wall with deps affects it
          board().visit_adjacent(
              illum_coord, [&](model::Coord adj_coord, CellState neighbor) {
                update_wall(adj_coord, neighbor, CellState::Illum, false);
              });
        }
        else if (cell == CellState::Bulb) {
          has_error_ = true;
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

} // namespace solver
