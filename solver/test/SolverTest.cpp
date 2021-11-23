#include "Solver.hpp"
#include "ASCIILevelCreator.hpp"
#include "BasicBoard.hpp"
#include "Solution.hpp"
#include <gtest/gtest.h>
#include <iostream>

namespace solver::test {
using namespace ::testing;

TEST(SolverTest, isolated_empty) {
  model::test::ASCIILevelCreator creator;
  creator("0.0");
  creator(".0.");
  creator("0.0");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(solution.board().is_solved());
}

TEST(SolverTest, isolated_empty2) {
  model::test::ASCIILevelCreator creator;
  creator("*++");
  creator("+.0");
  creator("+00");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(solution.board().is_solved());
}

TEST(SolverTest, isolated_empty3_no_solution) {
  model::test::ASCIILevelCreator creator;
  creator("*++");
  creator("+..");
  creator("+00");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_FALSE(solution.board().is_solved());
}

TEST(SolverTest, wall_with_deps_equal_to_open_faces1) {
  model::test::ASCIILevelCreator creator;
  creator("0.0");
  creator(".4.");
  creator("0.0");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(solution.board().is_solved());
}

TEST(SolverTest, wall_with_deps_equal_to_open_faces2) {
  model::test::ASCIILevelCreator creator;
  creator("2.0");
  creator(".0.");
  creator("0.2");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(solution.board().is_solved());
}

TEST(SolverTest, wall_with_deps_equal_to_open_faces3) {
  model::test::ASCIILevelCreator creator;
  creator("2.");
  creator(".3");
  creator("2.");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  EXPECT_TRUE(solution.board().is_solved());
}

} // namespace solver::test
