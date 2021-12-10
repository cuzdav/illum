#pragma once

#include "BasicBoard.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "PositionBoard.hpp"
#include "utils/scope.hpp"
#include <iosfwd>
#include <vector>

namespace solver {

// Essentially, a stack of PositionBoard, with the PositionBoard interface
// reflecting the top object
class AnalysisBoard {
public:
  AnalysisBoard(model::BasicBoard const & current);

  AnalysisBoard &
  operator=(solver::PositionBoard const & pboard) {
    cur() = pboard;
    return *this;
  }

  // removes most recently cloned board, stats, to previous position
  void clone_position();
  void pop();

  void
  reset(model::BasicBoard const & board) {
    cur().reset(board);
  }

  void
  reevaluate_board_state(
      PositionBoard::ResetPolicy policy =
          PositionBoard::ResetPolicy::STOP_PLAYING_MOVES_ON_ERROR) {
    cur().reevaluate_board_state(policy);
  }

  auto
  stack_size() const {
    return position_boards_.size();
  }

  void
  reset(solver::PositionBoard const & board) {
    cur().reset(board.board());
  }

  int
  width() const {
    return cur().width();
  }
  int
  height() const {
    return cur().height();
  }

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
  apply_move(model::SingleMove const & move) {
    return cur().apply_move(move);
  }

  bool
  is_solved() const {
    return cur().is_solved();
  }
  bool
  has_error() const {
    return cur().has_error();
  }

  solver::DecisionType
  decision_type() const {
    return cur().decision_type();
  }

  void
  set_has_error(bool            yn,
                DecisionType    decision,
                model::OptCoord coord = std::nullopt) {
    cur().set_has_error(yn, decision, coord);
  }

  model::OptCoord
  get_ref_location() const {
    return cur().get_ref_location();
  }

  bool
  is_ambiguous() const {
    return cur().is_ambiguous();
  }

  int
  num_cells_needing_illumination() const {
    return cur().num_cells_needing_illumination();
  }
  int
  num_walls_with_deps() const {
    return cur().num_walls_with_deps();
  }

  model::CellState
  get_cell(model::Coord coord) const {
    return cur().get_cell(coord);
  }

  int
  get_visit_depth() const {
    return visit_depth_;
  }

  bool
  set_cell(model::Coord                 coord,
           model::CellState             cell,
           PositionBoard::SetCellPolicy policy =
               PositionBoard::SetCellPolicy::REEVALUATE_IF_NECESSARY) {
    return cur().set_cell(coord, cell, policy);
  }

  model::BasicBoard const &
  basic_board() const {
    return cur().board();
  }

  bool
  visit_board(model::CellVisitor auto && visitor) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_board(std::forward<decltype(visitor)>(visitor));
  }
  bool
  visit_board_if(model::CellVisitor auto &&        visitor,
                 model::CellVisitPredicate auto && should_visit_pred) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_board_if(
        std::forward<decltype(visitor)>(visitor),
        std::forward<decltype(should_visit_pred)>(should_visit_pred));
  }

  bool
  visit_adjacent(model::Coord                     coord,
                 model::OptDirCellVisitor auto && visitor) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_adjacent(
        coord, std::forward<decltype(visitor)>(visitor));
  }

  bool
  visit_adj_flank(model::Coord                     coord,
                  model::Direction                 dir,
                  model::OptDirCellVisitor auto && visitor) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_adj_flank(
        coord, dir, std::forward<decltype(visitor)>(visitor));
  }

  bool
  visit_empty(model::CellVisitor auto && visitor) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_empty(std::forward<decltype(visitor)>(visitor));
  }

  bool
  visit_row_left_of(model::Coord                     coord,
                    model::OptDirCellVisitor auto && visitor,
                    model::BasicBoard::VisitPolicy   policy =
                        model::BasicBoard::VisitPolicy::DEFAULT) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_row_left_of(
        coord, std::forward<decltype(visitor)>(visitor), policy);
  }
  bool
  visit_row_right_of(model::Coord                     coord,
                     model::OptDirCellVisitor auto && visitor,
                     model::BasicBoard::VisitPolicy   policy =
                         model::BasicBoard::VisitPolicy::DEFAULT) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_row_right_of(
        coord, std::forward<decltype(visitor)>(visitor), policy);
  }
  bool
  visit_col_above(model::Coord                     coord,
                  model::OptDirCellVisitor auto && visitor,
                  model::BasicBoard::VisitPolicy   policy =
                      model::BasicBoard::VisitPolicy::DEFAULT) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_col_above(
        coord, std::forward<decltype(visitor)>(visitor), policy);
  }
  bool
  visit_col_below(model::Coord                     coord,
                  model::OptDirCellVisitor auto && visitor,
                  model::BasicBoard::VisitPolicy   policy =
                      model::BasicBoard::VisitPolicy::DEFAULT) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_col_below(
        coord, std::forward<decltype(visitor)>(visitor), policy);
  }

  void
  visit_perpendicular(model::Coord                     coord,
                      model::Direction                 dir,
                      model::OptDirCellVisitor auto && visitor) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_perpendicular(
        coord, dir, std::forward<decltype(visitor)>(visitor));
  }

  void
  visit_rows_cols_outward(
      model::Coord                     coord,
      model::OptDirCellVisitor auto && visitor,
      model::Direction directions = model::directiongroups::all) const {
    ++visit_depth_;
    pc::scoped_exit on_return([this]() { this->visit_depth_--; });
    return basic_board().visit_rows_cols_outward(
        coord, std::forward<decltype(visitor)>(visitor), directions);
  }

  friend std::ostream & operator<<(std::ostream &, AnalysisBoard const &);

  PositionBoard const &
  position_board() const {
    return cur();
  }

private:
  PositionBoard &       cur();
  PositionBoard const & cur() const;
  model::BasicBoard &   mut_board();

private:
  mutable int                visit_depth_ = 0;
  std::vector<PositionBoard> position_boards_;
};

} // namespace solver

template <>
struct fmt::formatter<::solver::AnalysisBoard> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::solver::AnalysisBoard const & board, FormatContext & ctx) {
    return fmt::format_to(ctx.out(),
                          "AnalysisBoard{{\n\t"
                          "StackDepth={}\n\n\t"
                          "{}"
                          "}}",
                          board.stack_size(),
                          board.position_board());
  }
};
