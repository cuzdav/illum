#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "LevelCreator.hpp"
#include "Solution.hpp"
#include <gtest/gtest.h>
#include <iostream>

namespace solver::test {
using namespace ::testing;

TEST(SolverTest, isolated_empty) {
  model::test::LevelCreator creator;
  creator("0.0");
  creator(".0.");
  creator("0.0");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(check_solved(solution.model_.get_underlying_board()));
}

TEST(SolverTest, isolated_empty2) {
  model::test::LevelCreator creator;
  creator("*++");
  creator("+.0");
  creator("+00");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(check_solved(solution.model_.get_underlying_board()));
}

TEST(SolverTest, isolated_empty3_no_solution) {
  model::test::LevelCreator creator;
  creator("*++");
  creator("+..");
  creator("+00");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_FALSE(check_solved(solution.model_.get_underlying_board()));
}

TEST(SolverTest, wall_with_deps_equal_to_open_faces1) {
  model::test::LevelCreator creator;
  creator("0.0");
  creator(".4.");
  creator("0.0");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(check_solved(solution.model_.get_underlying_board()));
}

TEST(SolverTest, wall_with_deps_equal_to_open_faces2) {
  model::test::LevelCreator creator;
  creator("2.0");
  creator(".0.");
  creator("0.2");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(check_solved(solution.model_.get_underlying_board()));
}

TEST(SolverTest, wall_with_deps_equal_to_open_faces3) {
  model::test::LevelCreator creator;
  creator("2.");
  creator(".3");
  creator("2.");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  EXPECT_TRUE(check_solved(solution.model_.get_underlying_board()));
}

TEST(SolverTest, wall_with_deps_and_open_face_gets_mark) {
  model::test::LevelCreator creator;
  creator("1*");
  creator("..");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  EXPECT_FALSE(check_solved(solution.model_.get_underlying_board()));
  EXPECT_EQ(model::CellState::Mark, solution.model_.get_cell(1, 0));
}

} // namespace solver::test
