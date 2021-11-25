#include "ASCIILevelCreator.hpp"
#include "BasicBoard.hpp"
#include "Solution.hpp"
#include "Solver.hpp"
#include "utils/DebugLog.hpp"
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
  EXPECT_TRUE(solution.board().is_solved());

  creator("1+*+");
  creator("*+2*");

  model::BasicBoard expected;
  creator.finished(&expected);

  ASSERT_EQ(expected, solution.board().board());
}

TEST(SolverSpeculationTest, simple_deductions2) {
  model::test::ASCIILevelCreator creator;
  creator(".2..");
  creator("....");
  creator("0.2.");
  creator("..0.");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  ASSERT_TRUE(solution.board().is_solved());

  creator("*2*+");
  creator("++++");
  creator("0*2*");
  creator("*+0+");

  model::BasicBoard expected;
  creator.finished(&expected);
  ASSERT_EQ(expected, solution.board().board());
}

TEST(SolverSpeculationTest, realistic_game1) {
  model::test::ASCIILevelCreator creator;
  creator("....1...0.");
  creator("..2...0..1");
  creator("..0.......");
  creator("01.......2");
  creator(".....2.2..");
  creator("...0.....0");
  creator("1.....1...");
  creator("..0.0.....");
  creator("1.......01");
  creator(".......0..");
  creator("0..1...1..");
  creator(".3...1....");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  EXPECT_TRUE(solution.board().is_solved());

  std::cout << "*** Final board state: " << solution.board() << std::endl;

  creator("++*+1*++0*");
  creator("*+2*++0++1");
  creator("+*0++++*++");
  creator("01++++++*2");
  creator("++++*2*2+*");
  creator("++*0+++*+0");
  creator("1++++*1++*");
  creator("*+0+0+++++");
  creator("1+++*+++01");
  creator("+++*+++0+*");
  creator("0*+1+++1*+");
  creator("*3*++1*+++");

  model::BasicBoard expected;
  creator.finished(&expected);
  EXPECT_EQ(expected, solution.board().board());
}

TEST(SolverSpeculationTest, realistic_game2) {
  model::test::ASCIILevelCreator creator;
  creator("..0....2...0....");
  creator("....00001....1..");
  creator(".0.10..2..2.0..1");
  creator("..2........0.0..");
  creator("1..1.0.......02.");
  creator("..2...0....2..0.");
  creator("........1.2...0.");
  creator(".0....1......100");
  creator("100......2....2.");
  creator(".1...0.0........");
  creator(".0..0....2...2..");
  creator(".01.......0.0..0");
  creator("..0.0........0..");
  creator("0..1.4..0..00.3.");
  creator("..0....00000....");
  creator("....1...0....1..");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

} // namespace solver::test
