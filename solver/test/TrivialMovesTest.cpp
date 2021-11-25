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

  AnnotatedMoves moves;

  bool valid_board = find_isolated_cells(basic_board, moves);
  ASSERT_TRUE(valid_board);
  ASSERT_EQ(4, moves.size());
  ASSERT_THAT(
      moves,
      UnorderedElementsAre(
          AnnotatedMove{
              SingleMove{
                  Action::Add, CellState::Empty, CellState::Bulb, Coord{0, 1}},
              DecisionType::ISOLATED_EMPTY_SQUARE,
              MoveMotive::FORCED,
              Coord{0, 1}},
          AnnotatedMove{
              SingleMove{
                  Action::Add, CellState::Empty, CellState::Bulb, Coord{1, 0}},
              DecisionType::ISOLATED_EMPTY_SQUARE,
              MoveMotive::FORCED,
              Coord{1, 0}},
          AnnotatedMove{
              SingleMove{
                  Action::Add, CellState::Empty, CellState::Bulb, Coord{1, 2}},
              DecisionType::ISOLATED_EMPTY_SQUARE,
              MoveMotive::FORCED,
              Coord{1, 2}},
          AnnotatedMove{
              SingleMove{
                  Action::Add, CellState::Empty, CellState::Bulb, Coord{2, 1}},
              DecisionType::ISOLATED_EMPTY_SQUARE,
              MoveMotive::FORCED,
              Coord{2, 1}}));
}

TEST(TrivialMovesTest, test_find_wall_with_deps_equal_open_faces) {
  model::test::ASCIILevelCreator creator;
  creator(".00");
  creator(".10");
  creator("000");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);

  AnnotatedMoves moves;
  find_walls_with_deps_equal_open_faces(basic_board, moves);
  ASSERT_THAT(1, moves.size());
  EXPECT_THAT(moves[0],
              Eq(AnnotatedMove{SingleMove{Action::Add,
                                          CellState::Empty,
                                          CellState::Bulb,
                                          Coord{1, 0}},
                               DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
                               MoveMotive::FORCED,
                               Coord{1, 1}}));
}

TEST(TrivialMovesTest, wall_satisfied_with_open_face_gets_mark) {
  model::test::ASCIILevelCreator creator;
  creator("1*");
  creator(".+");

  model::BasicBoard board;
  creator.finished(&board);

  AnnotatedMoves moves;
  find_trivial_moves(board, moves);

  EXPECT_THAT(
      moves,
      ElementsAre(AnnotatedMove{
          SingleMove{
              Action::Add, CellState::Empty, CellState::Mark, Coord{1, 0}},
          DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
          MoveMotive::FORCED,
          Coord{0, 0}}));
}

} // namespace solver::test
