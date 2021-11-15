#pragma once

#include "BasicBoard.hpp"
#include "Coord.hpp"
#include <vector>

namespace solver {

class AnalysisBoard {
public:
  enum class WallState : char { Unsatisfied, Satisfied, Error };

  AnalysisBoard(model::BasicBoard const & current);

  // clones board, applies move, returns all affected cells. Returns bool
  // indicating request was successful.
  bool add_bulb(model::Coord);
  bool add_mark(model::Coord);

  WallState compute_wall_state(model::Coord     wall_coord,
                               model::CellState wall_cell) const;

  // removes most recently cloned board, stats, to previous position
  void clone_position();
  void pop();

  bool is_solved() const;
  bool has_error() const;

  int num_cells_needing_illumination() const;
  int num_walls_with_deps() const;

  model::BasicBoard const & board() const;

  void set_has_error(bool);

private:
  struct Position {
    bool              has_error_             = false;
    int               needs_illum_count_     = 0;
    int               walls_with_deps_count_ = 0;
    model::BasicBoard board_;

    Position(model::BasicBoard const & board) : board_(board) {}
  };

  Position &          cur();
  Position const &    cur() const;
  model::BasicBoard & mut_board();

  void update_wall(model::Coord     wall_coord,
                   model::CellState wall_cell,
                   model::CellState play_cell,
                   bool             is_adjacent_to_play);

private:
  std::vector<Position> positions_;
};

} // namespace solver
