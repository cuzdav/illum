#pragma once

#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "SingleMove.hpp"
#include <iosfwd>
#include <vector>

namespace solver {

// A position board is a rich wrapper around an underlying basic board. It
// applies bulbs and marks, and illuminates the cells that bulbs illuminate. It
// also tracks the validity of the move, the number of cells needing
// illumination, and the number of walls still having unsatisfied deps.
// If it detects an error, it tries to record why in the decision type,
// referring to the ref_location on the board.

// For convenience, the underlying board's rich visit interface is made
// available here.

class PositionBoard {
public:
  using Coord = model::Coord;

  PositionBoard(model::BasicBoard const & board);

  // clones board, applies move, returns all affected cells. Returns bool
  // indicating request was successful.
  bool add_bulb(Coord);
  bool add_mark(Coord);
  bool apply_move(model::SingleMove const & move);

  DecisionType    decision_type() const;
  model::OptCoord get_ref_location() const;
  bool            has_error() const;
  void            set_has_error(bool, DecisionType = DecisionType::NONE);

  bool is_solved() const;
  int  needs_illum_count() const;
  int  walls_with_deps_count() const;

  int num_cells_needing_illumination() const;
  int num_walls_with_deps() const;

  int width() const;
  int height() const;

  model::CellState get_cell(Coord coord) const;

  model::BasicBoard const & board() const;
  model::BasicBoard &       mut_board();

  auto operator<=>(PositionBoard const &) const = default;

  bool visit_board(model::CellVisitor auto && visitor) const;
  bool
       visit_board_if(model::CellVisitor auto &&        visitor,
                      model::CellVisitPredicate auto && should_visit_pred) const;
  bool visit_adjacent(Coord coord, model::CellVisitor auto && visitor) const;
  bool visit_empty(model::CellVisitor auto && visitor) const;
  bool visit_row_left_of(Coord                            coord,
                         model::OptDirCellVisitor auto && visitor) const;
  bool visit_row_right_of(Coord                            coord,
                          model::OptDirCellVisitor auto && visitor) const;
  bool visit_col_above(Coord                            coord,
                       model::OptDirCellVisitor auto && visitor) const;
  bool visit_col_below(Coord                            coord,
                       model::OptDirCellVisitor auto && visitor) const;

  void visit_perpendicular(Coord                            coord,
                           model::Direction                 dir,
                           model::OptDirCellVisitor auto && visitor) const;

private:
  friend std::ostream & operator<<(std::ostream &, PositionBoard const &);

  void update_wall(Coord            wall_coord,
                   model::CellState wall_cell,
                   model::CellState play_cell,
                   bool             is_adjacent_to_play);

  bool              has_error_             = false;
  int               needs_illum_count_     = 0;
  int               walls_with_deps_count_ = 0;
  DecisionType      decision_type_;
  model::OptCoord   ref_location_;
  model::BasicBoard board_;
};

inline model::CellState
PositionBoard::get_cell(model::Coord coord) const {
  return board_.get_cell(coord);
}

inline bool
PositionBoard::visit_board(model::CellVisitor auto && visitor) const {
  return board_.visit_board(std::forward<decltype(visitor)>(visitor));
}
inline bool
PositionBoard::visit_board_if(
    model::CellVisitor auto &&        visitor,
    model::CellVisitPredicate auto && should_visit_pred) const {
  return board_.visit_board_if(
      std::forward<decltype(visitor)>(visitor),
      std::forward<decltype(should_visit_pred)>(should_visit_pred));
}
inline bool
PositionBoard::visit_adjacent(Coord                      coord,
                              model::CellVisitor auto && visitor) const {
  return board_.visit_adjacent(coord, std::forward<decltype(visitor)>(visitor));
}
inline bool
PositionBoard::visit_empty(model::CellVisitor auto && visitor) const {
  return board_.visit_empty(std::forward<decltype(visitor)>(visitor));
}
inline bool
PositionBoard::visit_row_left_of(
    Coord coord, model::OptDirCellVisitor auto && visitor) const {
  return board_.visit_row_left_of(coord,
                                  std::forward<decltype(visitor)>(visitor));
}
inline bool
PositionBoard::visit_row_right_of(
    Coord coord, model::OptDirCellVisitor auto && visitor) const {
  return board_.visit_row_right_of(coord,
                                   std::forward<decltype(visitor)>(visitor));
}
inline bool
PositionBoard::visit_col_above(Coord                            coord,
                               model::OptDirCellVisitor auto && visitor) const {
  return board_.visit_col_above(std::forward<decltype(visitor)>(visitor));
}
inline bool
PositionBoard::visit_col_below(Coord                            coord,
                               model::OptDirCellVisitor auto && visitor) const {
  return board_.visit_col_below(coord,
                                std::forward<decltype(visitor)>(visitor));
}
inline void
PositionBoard::visit_perpendicular(
    Coord                            coord,
    model::Direction                 dir,
    model::OptDirCellVisitor auto && visitor) const {
  return board_.visit_perpendicular(
      coord, dir, std::forward<decltype(visitor)>(visitor));
}

} // namespace solver
