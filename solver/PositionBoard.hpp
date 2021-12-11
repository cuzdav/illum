#pragma once

#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "Direction.hpp"
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
  using Coord     = model::Coord;
  using CellState = model::CellState;

  enum class ResetPolicy { STOP_PLAYING_MOVES_ON_ERROR, KEEP_ERRORS };

  PositionBoard() = default;
  PositionBoard(int height, int width);
  PositionBoard(model::BasicBoard const & board,
                ResetPolicy = ResetPolicy::STOP_PLAYING_MOVES_ON_ERROR);

  // Returns bool indicating request was successful.
  bool add_bulb(Coord);
  bool add_mark(Coord);
  bool add_wall(Coord, CellState); // TODO
  bool remove_bulb(Coord);         // TODO
  bool remove_mark(Coord);         // TODO
  bool remove_wall(Coord);         // TODO
  bool apply_move(model::SingleMove const &);

  // Some set-cell calls can cause the underlying board to get out of sync with
  // the position board error model, illumination, etc. Normally it will not
  // reevaluate for moves that are easy to handle (add bulb, add mark, etc.,
  // since that's supported), but for adding walls, or removing them, or
  // changing wall deps, etc., the board should be fully reevaluated. However,
  // if a cluster of changes will happen, we might want to not update until the
  // last change, for efficiency and to make the whole group more "atomic".
  enum class SetCellPolicy {
    FORCE_REEVALUATE_BOARD,
    NO_REEVALUATE_BOARD,
    REEVALUATE_IF_NECESSARY
  };
  bool set_cell(Coord,
                CellState,
                SetCellPolicy = SetCellPolicy::REEVALUATE_IF_NECESSARY);
  // if any number of calls to set_cell have been made with NO_REEVALUATE_BOARD
  // policy, then calling reevaluate_board_state can re-sync the PositionBoard
  // gamestate with the actual underlying board.  Calling this computes the same
  // thing as set_cell with FORCE_REEVALUATE_BOARD policy.
  void reevaluate_board_state(
      ResetPolicy = ResetPolicy::STOP_PLAYING_MOVES_ON_ERROR);

  void reset(int height, int width);
  void reset(model::BasicBoard const & board,
             ResetPolicy = ResetPolicy::STOP_PLAYING_MOVES_ON_ERROR);

  DecisionType    decision_type() const;
  model::OptCoord get_ref_location() const;
  bool            has_error() const;
  void set_has_error(bool, DecisionType, model::OptCoord = std::nullopt);

  bool is_solved() const;
  bool is_ambiguous() const;
  int  num_cells_needing_illumination() const;
  int  num_walls_with_deps() const;

  int width() const;
  int height() const;

  CellState                get_cell(Coord coord) const;
  std::optional<CellState> get_opt_cell(Coord coord) const;

  model::BasicBoard const & board() const;
  model::BasicBoard &       mut_board();

  auto operator<=>(PositionBoard const &) const = default;

  bool visit_board(model::CellVisitor auto && visitor) const;
  bool
       visit_board_if(model::CellVisitor auto &&        visitor,
                      model::CellVisitPredicate auto && should_visit_pred) const;
  bool visit_adjacent(Coord                            coord,
                      model::OptDirCellVisitor auto && visitor) const;
  bool visit_adj_flank(model::Coord                     coord,
                       model::Direction                 dir,
                       model::OptDirCellVisitor auto && visitor) const;

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

  void visit_rows_cols_outward(
      model::Coord                     coord,
      model::OptDirCellVisitor auto && visitor,
      model::Direction directions = model::directiongroups::all) const;

private:
  friend std::ostream & operator<<(std::ostream &, PositionBoard const &);

  void remove_illum_in_direction_from(model::Coord     start_at,
                                      model::Direction dir);

  // returns indication if wall at wall_coord "is satisfied"
  bool update_wall(Coord     wall_coord,
                   CellState wall_cell,
                   CellState play_cell,
                   bool      is_adjacent_to_play);

  bool              has_error_                      = false;
  int               num_cells_needing_illumination_ = 0;
  int               num_walls_with_deps_            = 0;
  DecisionType      decision_type_                  = DecisionType::NONE;
  model::OptCoord   ref_location_;
  model::BasicBoard board_{};
};

inline void
PositionBoard::reset(int height, int width) {
  has_error_                      = false;
  num_cells_needing_illumination_ = 0;
  num_walls_with_deps_            = 0;
  decision_type_                  = DecisionType::NONE;
  ref_location_                   = std::nullopt;
  board_.reset(height, width);
}

inline model::CellState
PositionBoard::get_cell(model::Coord coord) const {
  return board_.get_cell(coord);
}

inline std::optional<model::CellState>
PositionBoard::get_opt_cell(model::Coord coord) const {
  return board_.get_opt_cell(coord);
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
PositionBoard::visit_adjacent(Coord                            coord,
                              model::OptDirCellVisitor auto && visitor) const {
  return board_.visit_adjacent(coord, std::forward<decltype(visitor)>(visitor));
}
inline bool
PositionBoard::visit_adj_flank(model::Coord                     coord,
                               model::Direction                 dir,
                               model::OptDirCellVisitor auto && visitor) const {
  return board_.visit_adj_flank(
      coord, dir, std::forward<decltype(visitor)>(visitor));
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
inline void
PositionBoard::visit_rows_cols_outward(model::Coord                     coord,
                                       model::OptDirCellVisitor auto && visitor,
                                       model::Direction directions) const {
  return board_.visit_rows_cols_outward(
      coord, std::forward<decltype(visitor)>(visitor), directions);
}

} // namespace solver

template <>
struct fmt::formatter<::solver::PositionBoard> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::solver::PositionBoard const & board, FormatContext & ctx) {
    return fmt::format_to(ctx.out(),
                          "PositionBoard{{\n\t"
                          "CellS Needing Illuminatation: {}\n\t"
                          "Unsatisfied Walls: {}\n\t"
                          "Solved={}\n\t"
                          "HasError={}\n\t"
                          "DecisionType={}\n\t"
                          "RefLocation={}\n\t"
                          "{}"
                          "}}",
                          board.num_cells_needing_illumination(),
                          board.num_walls_with_deps(),
                          board.is_solved(),
                          board.has_error(),
                          board.decision_type(),
                          board.get_ref_location(),
                          board.board());
  }
};
