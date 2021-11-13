#include "AnalysisBoard.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"

namespace solver {

using model::CellState;

AnalysisBoard::AnalysisBoard(model::BasicBoard const & current)
    : positions_{{current}} {
  board().visit_board([&](model::Coord coord, auto cell) {
    if (model::is_illumable(cell)) {
      cur().needs_illum_count_++;
    }
    else if (is_wall_with_deps(cell)) {
      switch (compute_wall_state(coord, cell)) {
        case WallState::Unsatisfied:
          cur().walls_with_deps_count_++;
          break;

        case WallState::Satisfied:
          break;

        case WallState::Error:
          cur().has_error_ = true;
          break;
      }
    }
  });
}

AnalysisBoard::Position &
AnalysisBoard::cur() {
  return positions_.back();
}

AnalysisBoard::Position const &
AnalysisBoard::cur() const {
  return positions_.back();
}

model::BasicBoard &
AnalysisBoard::board() {
  return cur().board_;
}

model::BasicBoard const &
AnalysisBoard::board() const {
  return cur().board_;
}

void
AnalysisBoard::clone_position() {
  positions_.emplace_back(positions_.back());
}

void
AnalysisBoard::pop() {
  if (positions_.size() == 1) {
    return;
  }
  positions_.pop_back();
}

bool
AnalysisBoard::is_solved() const {
  auto const & pos = cur();
  return pos.has_error_ == false && pos.needs_illum_count_ == 0 &&
         pos.walls_with_deps_count_ == 0;
}

int
AnalysisBoard::num_cells_needing_illumination() const {
  return cur().needs_illum_count_;
}

int
AnalysisBoard::num_walls_with_deps() const {
  return cur().walls_with_deps_count_;
}

bool
AnalysisBoard::has_error() const {
  return cur().has_error_;
}

AnalysisBoard::WallState
AnalysisBoard::compute_wall_state(model::Coord     wall_coord,
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
AnalysisBoard::update_wall(model::Coord wall_coord,
                           CellState    wall_cell,
                           CellState    play_cell,
                           bool         coord_is_adjacent_to_play) {
  // did this play add the final bulb to satisfy the wall deps? If so, account
  // for it in our stats.
  if (model::is_wall_with_deps(wall_cell)) {
    switch (compute_wall_state(wall_coord, wall_cell)) {
      case WallState::Satisfied:
        if (coord_is_adjacent_to_play && play_cell == CellState::Bulb) {
          cur().walls_with_deps_count_--;
        }
        break;
      case WallState::Unsatisfied:
        break;
      case WallState::Error:
        cur().has_error_ = true;
        break;
    }
  }
}

bool
AnalysisBoard::add_bulb(model::Coord coord) {
  if (board().get_cell(coord) != CellState::Empty) {
    return false;
    ;
  }
  clone_position();
  board().set_cell(coord, CellState::Bulb);

  // update walls immediately adjacent to the bulb
  board().visit_adjacent(
      coord, [&](model::Coord adj_coord, CellState neighbor) {
        update_wall(adj_coord, neighbor, CellState::Bulb, true);
      });

  board().visit_rows_cols_outward(
      coord, [&](model::Coord cur_coord, CellState cell) {
        if (is_illumable(cell)) {
          board().set_cell(cur_coord, model::CellState::Illum);
          cur().needs_illum_count_--;

          // illuminating a cell adjacent to a wall with deps affects it
          board().visit_adjacent(
              coord, [&](model::Coord adj_coord, CellState neighbor) {
                update_wall(adj_coord, neighbor, CellState::Illum, false);
              });
        }
      });

  return true;
}

} // namespace solver
