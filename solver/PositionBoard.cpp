#include "PositionBoard.hpp"
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

PositionBoard::PositionBoard(int height, int width) {
  board_.reset(height, width);
}

PositionBoard::PositionBoard(model::BasicBoard const & current,
                             RESETPolicy               policy) {
  reset(current, policy);
}

void
PositionBoard::reset(model::BasicBoard const &  current,
                     PositionBoard::RESETPolicy policy) {
  reset(current.height(), current.width());

  // first copy the walls and update counts
  current.visit_board([&](model::Coord coord, auto cell) {
    if (model::is_dynamic_entity(cell)) {
      num_cells_needing_illumination_++;
    }
    else if (is_wall(cell)) {
      num_walls_with_deps_ += model::is_wall_with_deps(cell);
      board_.set_cell(coord, cell);
      update_wall(coord, cell, cell, false);
    }
  });

  // now just play the moves from their board into ours.
  current.visit_board([&](model::Coord coord, auto cell) {
    if (has_error() && policy == RESETPolicy::STOP_PLAYING_MOVES_ON_ERROR) {
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

void
PositionBoard::reevaluate_board_state(PositionBoard::RESETPolicy policy) {
  // Recompute the state of the board by replaying from start on a separate
  // position board at arm's reach, and take its results.
  //
  // NOTE: set_cell can mess up the accumulated state (error, num illuminated,
  // wall deps, etc) of the position board by blindly changing the underlying
  // board in ways we can't easily track. Creating a new position board with
  // our underlying is non-destructive way to evaluate our current board
  // without any changes to our current board or state, and then we can just
  // take the results.
  PositionBoard paranoid(board_, policy);
  has_error_                      = paranoid.has_error_;
  num_cells_needing_illumination_ = paranoid.num_cells_needing_illumination_;
  num_walls_with_deps_            = paranoid.num_walls_with_deps_;
  decision_type_                  = paranoid.decision_type_;
  ref_location_                   = paranoid.ref_location_;
  board_                          = paranoid.board_;
}

bool
PositionBoard::set_cell(model::Coord     coord,
                        model::CellState cell,
                        SetCellPolicy    policy) {

  // for known simple cases, try to not recompute the state unless forced to.
  // For a policy of FORCE_REEVALUATE_BOARD we can't bypass these shortcuts and
  // will reevaluate the board unconditionally.
  if (policy == SetCellPolicy::REEVALUATE_IF_NECESSARY) {
    // these replace the simple
    auto orig_cell = board_.get_cell(coord);
    if (cell == orig_cell) {
      return true;
    }

    // changing empty to something known how to handle:
    if (is_empty(orig_cell)) {
      switch (cell) {
        case CellState::BULB:
          return add_bulb(coord);
        case CellState::MARK:
          return add_mark(coord);
        default:
          if (is_wall(cell)) {
            if (add_wall(coord, cell)) {
              return true;
            }
          }
          break;
      }
    }
    else if (orig_cell == CellState::ILLUM && not has_error()) {
      if (is_wall(cell)) {
        return add_wall(coord, cell);
      }
    }

    // removing something known how to handle
    else if (is_empty(cell)) {
      switch (orig_cell) {
        case CellState::BULB:
          return remove_bulb(coord);
          // case CellState::MARK:
          //   return remove_mark(coord);
      }
    }
  }
  bool result = board_.set_cell(coord, cell);

  // unless explicitly forbidden, we should reevaluate after set_cell
  if (policy != SetCellPolicy::NO_REEVALUATE_BOARD) {
    reevaluate_board_state(PositionBoard::RESETPolicy::KEEP_ERRORS);
  }
  return result;
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
  return has_error_ == false && num_walls_with_deps_ == 0 &&
         num_cells_needing_illumination_ == 0;
}

bool
PositionBoard::is_ambiguous() const {
  return decision_type_ == DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION;
}

int
PositionBoard::num_cells_needing_illumination() const {
  return num_cells_needing_illumination_;
}

int
PositionBoard::num_walls_with_deps() const {
  return num_walls_with_deps_;
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

void
PositionBoard::set_has_error(bool            yn,
                             DecisionType    decision,
                             model::OptCoord location) {
  has_error_     = yn;
  decision_type_ = decision;
  ref_location_  = location;
}

DecisionType
PositionBoard::decision_type() const {
  return decision_type_;
}

model::OptCoord
PositionBoard::get_ref_location() const {
  return ref_location_;
}

// Handles a state change when a light is played:
// 1) did the light block a face that makes the wall impossible to satisfy?
// 2) did the bulb over-subscribe the wall?
// 3) did the bulb just satisfy the wall, and we should reduce our counter?

// NOTE: noops for any non-wall tile, or a WALL0, and considers them satisfied
bool
PositionBoard::update_wall(model::Coord wall_coord,
                           CellState    wall_cell,
                           CellState    play_cell,
                           bool         coord_is_adjacent_to_play) {
  assert(wall_cell == get_cell(wall_coord));

  if (int deps = model::num_wall_deps(wall_cell); deps > 0) {

    int empty_neighbors = 0;
    int bulb_neighbors  = 0;
    board().visit_adjacent(wall_coord,
                           [&](model::Coord coord, CellState neighbor) {
                             bulb_neighbors += neighbor == CellState::BULB;
                             empty_neighbors += neighbor == CellState::EMPTY;
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
             play_cell == CellState::BULB) {
      // just-played bulb satisfied this wall, so decrement counter
      num_walls_with_deps_--;
    }
    return bulb_neighbors == deps;
  }
  return true;
}

void
PositionBoard::remove_illum_in_direction_from(model::Coord start_at,
                                              Direction    direction) {
  visit_rows_cols_outward(
      start_at,
      [this, direction](Coord coord, CellState cell) {
        if (cell == CellState::ILLUM) {
          bool has_crossbeam = false;
          visit_perpendicular(
              coord, direction, [&](auto, CellState cross_cell) {
                has_crossbeam |= is_bulb(cross_cell);
              });
          if (not has_crossbeam) {
            board_.set_cell(coord, CellState::EMPTY);
            ++num_cells_needing_illumination_;
          }
        }
      },
      direction);
}

bool
PositionBoard::add_wall(model::Coord wall_coord, model::CellState wall_cell) {
  assert(is_wall(wall_cell));
  CellState const orig_cell = get_cell(wall_coord);
  if (not is_empty(orig_cell)) {
    // When placing a wall in an empty cell, the board state can remain
    // the same or enter an error state (e.g. wall is unsatisfiable, or
    // makes a neighbor unsatisfiable.) But can never REMOVE an error
    // state once set. That's beyond the bookkeeping of this class without
    // starting clean and building up the state again and visiting the
    // whole board. (If there are 2 errors, and we fix one, we don't know
    // it since there's only a bool.) So don't allow cases that could fix
    // errors, and require the slow rebuilding appraoch. Therefore, if
    // there is an error, we cannot allow adding a wall except on empty
    // tiles (or we might report an error that no longer exists.)
    if (has_error_) {
      return false;
    }
  }

  board_.set_cell(wall_coord, wall_cell);

  if (model::is_illuminable(orig_cell)) {
    --num_cells_needing_illumination_;
  }

  // This wall deps counter logic works even with WALL0, which has no
  // deps, because WALL0 is pathologically satisfied.
  num_walls_with_deps_++;
  bool is_satisfied = update_wall(wall_coord, wall_cell, wall_cell, false);
  num_walls_with_deps_ -= is_satisfied;

  visit_adjacent(wall_coord, [&](Coord adj_coord, CellState adj_cell) {
    update_wall(adj_coord, adj_cell, adj_cell, false);
  });

  // remove illumination we may have blocked by adding a wall,
  // as long as it's not also illuminated due to a crossbeam of light.
  //
  // Starting in this position:
  // ...+........+.
  // ++++++++++++*+
  // ...+........+.
  // ...*++++++++++

  // ADDing a wall:
  // ...+........+.
  // ++++++++++++*+
  // ...+........+.
  // ...*+++0....+.
  //
  // Note it blocks the light except in the column at the right since it's
  // still illuminated from above.

  //
  // ALSO: we are not in an error state, so don't have to check or handle
  // the case where two lights can see each other.
  //
  if (orig_cell == CellState::ILLUM) {
    // find light source(s)
    model::Direction light_sources{};
    visit_rows_cols_outward(
        wall_coord, [&](Direction dir, Coord coord, CellState cell) {
          if (is_bulb(cell)) {
            // clear illumination in other direction from bulb
            remove_illum_in_direction_from(wall_coord, flip(dir));
          }
        });
  }

  return true;
}

bool
PositionBoard::remove_bulb(model::Coord bulb_coord) {
  // TODO: this is inefficient logic since it recomputes the board from the
  // beginning after removing the bulb.  But it works, and is a placeholder
  // until I feel like dealing with it better.
  board_.set_cell(bulb_coord, model::CellState::EMPTY);
  auto board_copy = board_;

  // an unsolved board is an error, but we don't want that to stop us from
  // copying the moves back into the new board.
  reset(board_copy, PositionBoard::RESETPolicy::KEEP_ERRORS);
  return true;
}

bool
PositionBoard::add_bulb(model::Coord bulb_coord) {
  CellState bulb_target = get_cell(bulb_coord);
  // A mark's job is to prevent playing a bulb there, so don't allow it.
  // Otherwise, allow them to play a mistake, but only one.
  if (bulb_target != (bulb_target & (CellState::EMPTY | CellState::ILLUM))) {
    return false;
  }
  mut_board().set_cell(bulb_coord, CellState::BULB);
  num_cells_needing_illumination_ -= bulb_target == CellState::EMPTY;

  // update walls immediately adjacent to the bulb
  board().visit_adjacent(
      bulb_coord, [&](model::Coord adj_coord, CellState neighbor) {
        update_wall(adj_coord, neighbor, CellState::BULB, true);
      });

  // now emit light outwards, and see if it affects walls nearby
  board().visit_rows_cols_outward(
      bulb_coord, [&](Direction dir, model::Coord coord, CellState cell) {
        if (is_illuminable(cell)) {
          mut_board().set_cell(coord, model::CellState::ILLUM);
          num_cells_needing_illumination_--;

          // illuminating a cell adjacent to a wall with deps affects it. Only
          // check left/right (flank) because looking ahead is redundant since
          // we are walking in that direction anyway and will process when we
          // get there. Don't look behind because it's already processed.
          board().visit_adj_flank(
              coord, dir, [&](model::Coord adj_coord, CellState neighbor) {
                update_wall(adj_coord, neighbor, CellState::ILLUM, false);
              });
        }
        else if (is_bulb(cell)) {
          decision_type_ = DecisionType::BULBS_SEE_EACH_OTHER;
          has_error_     = true;
          ref_location_  = bulb_coord;
        }
        else if (is_wall_with_deps(cell)) {
          update_wall(coord, cell, CellState::ILLUM, false);
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
  mut_board().set_cell(mark_coord, CellState::MARK);

  // update walls immediately adjacent to the mark
  board().visit_adjacent(
      mark_coord, [&](model::Coord neighbor_coord, CellState neighbor_cell) {
        update_wall(neighbor_coord, neighbor_cell, CellState::MARK, true);
      });
  return true;
}

bool
PositionBoard::apply_move(const model::SingleMove & move) {
  if (move.action_ == model::Action::ADD) {
    if (move.to_ == CellState::BULB) {
      add_bulb(move.coord_);
      return true;
    }
    if (move.to_ == CellState::MARK) {
      add_mark(move.coord_);
      return true;
    }
    else {
      return false;
    }
  }
  else if (move.action_ == model::Action::REMOVE) {
    if (move.from_ == CellState::BULB || move.from_ == CellState::MARK) {
      set_cell(move.coord_, model::CellState::EMPTY);
      return true;
    }
  }
  return false;
}

std::ostream &
operator<<(std::ostream & os, PositionBoard const & pos_board) {
  fmt::print(os, "{}", pos_board);
  return os;
}

} // namespace solver
