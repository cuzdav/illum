#include "Solver.hpp"
#include "BasicBoard.hpp"
#include "LevelCreator.hpp"
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

  std::cout << solution.model_.get_underlying_board() << std::endl;

  ASSERT_TRUE(check_solved(solution.model_.get_underlying_board()));
}

} // namespace solver::test
