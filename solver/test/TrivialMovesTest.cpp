#include "ASCIILevelCreator.hpp"
#include "Action.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "TestUtils.hpp"
#include "trivial_moves.hpp"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include <set>

namespace solver::test {

using namespace ::testing;
using model::Action;
using model::CellState;
using model::Coord;
using model::SingleMove;

TEST(TrivialMovesTest, find_isolated_cell_detects_invalid_mark) {
  model::test::ASCIILevelCreator creator;
  creator("0.0");
  creator("X4.");
  creator("0.0");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  std::unique_ptr board_analysis = create_board_analysis(basic_board);

  AnnotatedMoves moves;
  OptCoord       invalid_mark_coord =
      find_isolated_cells(basic_board, board_analysis.get(), moves);
  ASSERT_TRUE(invalid_mark_coord.has_value());
  ASSERT_EQ(Coord(1, 0), *invalid_mark_coord);
}

TEST(TrivialMovesTest, find_isolated_cell) {
  model::test::ASCIILevelCreator creator;
  creator("0.0");
  creator(".4.");
  creator("0.0");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  std::unique_ptr board_analysis = create_board_analysis(basic_board);

  AnnotatedMoves moves;

  OptCoord invalid_mark_coord =
      find_isolated_cells(basic_board, board_analysis.get(), moves);
  ASSERT_FALSE(invalid_mark_coord.has_value());
  ASSERT_EQ(4, moves.size());
  ASSERT_THAT(
      moves,
      UnorderedElementsAre(
          bulb_at(
              {0, 1}, DecisionType::ISOLATED_EMPTY_SQUARE, MoveMotive::FORCED),
          bulb_at(
              {1, 0}, DecisionType::ISOLATED_EMPTY_SQUARE, MoveMotive::FORCED),
          bulb_at(
              {1, 2}, DecisionType::ISOLATED_EMPTY_SQUARE, MoveMotive::FORCED),
          bulb_at({2, 1},
                  DecisionType::ISOLATED_EMPTY_SQUARE,
                  MoveMotive::FORCED)));
}

TEST(TrivialMovesTest, find_around_walls_with_deps1) {
  model::test::ASCIILevelCreator creator;
  creator(".00");
  creator(".10");
  creator("000");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  std::unique_ptr board_analysis = create_board_analysis(basic_board);

  AnnotatedMoves moves;
  find_around_walls_with_deps(basic_board, board_analysis.get(), moves);
  ASSERT_THAT(1, moves.size());
  EXPECT_THAT(moves[0],
              Eq(bulb_at({1, 0},
                         DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
                         MoveMotive::FORCED,
                         Coord{1, 1})));
}

TEST(TrivialMovesTest, find_around_walls_with_deps2) {
  model::test::ASCIILevelCreator creator;
  creator("01*");
  creator("..0");

  model::BasicBoard board;
  creator.finished(&board);
  std::unique_ptr board_analysis = create_board_analysis(board);
  AnnotatedMoves  moves;
  find_trivial_moves(board, board_analysis.get(), moves);

  EXPECT_THAT(
      moves,
      ElementsAre(mark_at({1, 1},
                          DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
                          MoveMotive::FORCED,
                          Coord{0, 1})));
}

TEST(TrivialMovesTest, ambigus_cells_in_rows) {
  model::test::ASCIILevelCreator creator;
  creator("..0....");
  creator(".1..200");
  creator("0000+..");
  creator("...0*++");
  model::BasicBoard board;
  creator.finished(&board);

  AnnotatedMoves moves;
  find_ambiguous_linear_aligned_row_cells(board, moves);

  ASSERT_THAT(moves,
              ElementsAre(mark_at({0, 5},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP),
                          mark_at({0, 6},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP)));

  board.set_cell({0, 5}, CellState::Mark);
  board.set_cell({0, 6}, CellState::Mark);

  moves.clear();

  find_ambiguous_linear_aligned_row_cells(board, moves);

  ASSERT_THAT(moves,
              ElementsAre(mark_at({2, 5},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP),
                          mark_at({2, 6},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP)));

  board.set_cell({2, 5}, CellState::Mark);
  board.set_cell({5, 6}, CellState::Mark);
  moves.clear();
  find_ambiguous_linear_aligned_row_cells(board, moves);

  ASSERT_THAT(moves,
              ElementsAre(mark_at({3, 0},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP),
                          mark_at({3, 1},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP),
                          mark_at({3, 2},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP)));
}

TEST(TrivialMovesTest, ambigus_cells_in_cols_far_away_wall_with_deps) {
  model::test::ASCIILevelCreator creator;
  creator("0.0*0*0");
  creator("0.++++1");
  creator("0.++++.");
  creator("0+*0*0.");
  model::BasicBoard board;
  creator.finished(&board);

  AnnotatedMoves moves;
  find_ambiguous_linear_aligned_col_cells(board, moves);

  ASSERT_THAT(moves,
              ElementsAre(mark_at({0, 1},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP),
                          mark_at({1, 1},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP)));
}

TEST(TrivialMovesTest,
     ambigus_cells_in_cols_far_endpoints_are_walls_with_deps) {
  model::test::ASCIILevelCreator creator;
  creator("0100");
  creator("0.0.");
  creator("0.0.");
  creator("0001");
  model::BasicBoard board;
  creator.finished(&board);

  AnnotatedMoves moves;
  find_ambiguous_linear_aligned_col_cells(board, moves);

  // not just the "sides" should be checked, but the ends of the column should
  // too. (the 1 at {0,1} disqualifies {1,1} from consideration, as does {3,3}
  // disqualifies {2,3})
  ASSERT_THAT(moves, ElementsAre());
}

TEST(TrivialMovesTest, ambigus_cells_in_cols) {
  model::test::ASCIILevelCreator creator;
  creator("0.0..0.");
  creator("....20.");
  creator("0.0.+..");
  creator("0.10*++");
  model::BasicBoard board;
  creator.finished(&board);

  AnnotatedMoves moves;
  find_ambiguous_linear_aligned_col_cells(board, moves);

  ASSERT_THAT(moves,
              ElementsAre(mark_at({0, 1},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP),
                          mark_at({2, 1},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP)));

  board.set_cell({0, 1}, CellState::Mark);
  board.set_cell({2, 1}, CellState::Mark);

  //     ("0X0..0.");
  //     ("....20.");
  //     ("0X0.+..");
  //     ("0.10*++");

  moves.clear();

  find_ambiguous_linear_aligned_col_cells(board, moves);

  ASSERT_THAT(moves,
              ElementsAre(mark_at({0, 6},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP),
                          mark_at({1, 6},
                                  DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
                                  MoveMotive::FOLLOWUP)));
}

} // namespace solver::test
