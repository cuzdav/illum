#include "ASCIILevelCreator.hpp"
#include "Action.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "trivial/trivial_moves.hpp"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include <set>

namespace solver::test {

using namespace ::testing;
using model::Action;
using model::CellState;
using model::Coord;
using model::SingleMove;

TEST(TrivialMovesTest, find_isolated_cell) {
  model::test::ASCIILevelCreator creator;
  creator("0.0");
  creator(".4.");
  creator("0.0");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);

  IsolatedCell result = find_isolated_cell(basic_board);

  std::set<AnnotatedMove> moves;

  auto is_a_valid_move = AnyOf(
      VariantWith<AnnotatedMove>(AnnotatedMove{
          SingleMove{
              Action::Add, CellState::Empty, CellState::Bulb, Coord{0, 1}},
          DecisionType::ISOLATED_EMPTY_SQUARE,
          MoveMotive::FORCED,
          Coord{1, 1}}),
      VariantWith<AnnotatedMove>(AnnotatedMove{
          SingleMove{
              Action::Add, CellState::Empty, CellState::Bulb, Coord{1, 0}},
          DecisionType::ISOLATED_EMPTY_SQUARE,
          MoveMotive::FORCED,
          Coord{1, 1}}),
      VariantWith<AnnotatedMove>(AnnotatedMove{
          SingleMove{
              Action::Add, CellState::Empty, CellState::Bulb, Coord{1, 2}},
          DecisionType::ISOLATED_EMPTY_SQUARE,
          MoveMotive::FORCED,
          Coord{1, 1}}),
      VariantWith<AnnotatedMove>(AnnotatedMove{
          SingleMove{
              Action::Add, CellState::Empty, CellState::Bulb, Coord{2, 1}},
          DecisionType::ISOLATED_EMPTY_SQUARE,
          MoveMotive::FORCED,
          Coord{1, 1}}));

  // === MOVE 1 ===
  ASSERT_THAT(result, is_a_valid_move);
  AnnotatedMove last_move = std::get<AnnotatedMove>(result);
  moves.insert(last_move);
  basic_board.set_cell(last_move.next_move.coord_, last_move.next_move.to_);

  // === MOVE 2 ===
  result = find_isolated_cell(basic_board);
  ASSERT_THAT(result, is_a_valid_move);

  last_move = std::get<AnnotatedMove>(result);
  moves.insert(last_move);
  basic_board.set_cell(last_move.next_move.coord_, last_move.next_move.to_);
  EXPECT_THAT(moves.size(), Eq(2));

  // === MOVE 3 ===
  result = find_isolated_cell(basic_board);
  ASSERT_THAT(result, is_a_valid_move);

  last_move = std::get<AnnotatedMove>(result);
  moves.insert(last_move);
  basic_board.set_cell(last_move.next_move.coord_, last_move.next_move.to_);
  EXPECT_THAT(moves.size(), Eq(3));

  // === MOVE 4 ===
  result = find_isolated_cell(basic_board);
  ASSERT_THAT(result, is_a_valid_move);

  last_move = std::get<AnnotatedMove>(result);
  moves.insert(last_move);
  basic_board.set_cell(last_move.next_move.coord_, last_move.next_move.to_);
  EXPECT_THAT(moves.size(), Eq(4));

  // === (there is no move 5) ===
  result = find_isolated_cell(basic_board);
  ASSERT_THAT(result, VariantWith<std::monostate>(std::monostate{}));
}

TEST(TrivialMovesTest, test_find_wall_with_deps_equal_open_faces) {
  model::test::ASCIILevelCreator creator;
  creator(".00");
  creator(".10");
  creator("000");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);

  OptCoord result = find_wall_with_deps_equal_open_faces(basic_board);

  EXPECT_THAT(result, Eq(Coord{1, 1}));
}

} // namespace solver::test
