#pragma once

#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include <vector>

namespace solver {

class PositionBoard {
public:
  enum class WallState : char { Unsatisfied, Satisfied, Error };

  PositionBoard(model::BasicBoard const & board);

  // clones board, applies move, returns all affected cells. Returns bool
  // indicating request was successful.
  bool add_bulb(model::Coord);
  bool add_mark(model::Coord);
  bool apply_move(model::SingleMove const & move);

  WallState compute_wall_state(model::Coord     wall_coord,
                               model::CellState wall_cell) const;

  bool is_solved() const;
  bool has_error() const;
  int  needs_illum_count() const;
  int  walls_with_deps_count() const;

  int num_cells_needing_illumination() const;
  int num_walls_with_deps() const;

  model::BasicBoard const & board() const;
  model::BasicBoard &       mut_board();

  void set_has_error(bool);

private:
  void update_wall(model::Coord     wall_coord,
                   model::CellState wall_cell,
                   model::CellState play_cell,
                   bool             is_adjacent_to_play);

  bool              has_error_             = false;
  int               needs_illum_count_     = 0;
  int               walls_with_deps_count_ = 0;
  model::BasicBoard board_;
};

} // namespace solver
