#pragma once

#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "PositionBoard.hpp"
#include <vector>

namespace solver {

// Essentially, a stack of PositionBoard, with the PositionBoard interface
// reflecting the top object
class AnalysisBoard {
public:
  AnalysisBoard(model::BasicBoard const & current);

  // removes most recently cloned board, stats, to previous position
  void clone_position();
  void pop();

  // clones board, applies move, returns all affected cells. Returns bool
  // indicating request was successful.
  bool
  add_bulb(model::Coord coord) {
    return cur().add_bulb(coord);
  }
  bool
  add_mark(model::Coord coord) {
    return cur().add_mark(coord);
  }

  bool
  is_solved() const {
    return cur().is_solved();
  }
  bool
  has_error() const {
    return cur().has_error();
  }

  int
  num_cells_needing_illumination() const {
    return cur().num_cells_needing_illumination();
  }
  int
  num_walls_with_deps() const {
    return cur().num_walls_with_deps();
  }

  model::BasicBoard const &
  board() const {
    return cur().board();
  }

private:
  PositionBoard &       cur();
  PositionBoard const & cur() const;
  model::BasicBoard &   mut_board();

private:
  std::vector<PositionBoard> position_boards_;
};

} // namespace solver
