#include "ASCIILevelCreator.hpp"
#include "BasicBoard.hpp"
#include "DebugLog.hpp"
#include "Solution.hpp"
#include "Solver.hpp"
#include <gtest/gtest.h>
#include <iostream>

namespace solver::test {
using namespace ::testing;

// These tests requires some reasoning.  First let's do 1 level.

TEST(SolverSpeculationTest, simple_deductions1) {
  model::test::ASCIILevelCreator creator;
  creator("1...");
  creator("..2.");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  EXPECT_TRUE(solution.board_.is_solved());

  std::cout << to_string(solution.status_) << std::endl;
  std::cout << solution.board_.board() << std::endl;

  creator("1+*+");
  creator("*+2+");

  model::BasicBoard expected;
  creator.finished(&expected);

  ASSERT_TRUE(solution.known_solution_.has_value());
  ASSERT_EQ(expected, *solution.known_solution_);
}

} // namespace solver::test
