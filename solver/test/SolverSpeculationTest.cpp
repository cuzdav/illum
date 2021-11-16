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
  creator("*+2*");

  model::BasicBoard expected;
  creator.finished(&expected);

  ASSERT_TRUE(solution.known_solution_.has_value());
  ASSERT_EQ(expected, *solution.known_solution_);
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
  ASSERT_TRUE(solution.known_solution_.has_value());

  creator("*2*+");
  creator("++++");
  creator("0*2*");
  creator("*+0+");

  model::BasicBoard expected;
  creator.finished(&expected);
  ASSERT_EQ(expected, *solution.known_solution_);
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
  ASSERT_TRUE(solution.known_solution_.has_value());

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
  ASSERT_EQ(expected, *solution.known_solution_);
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
  ASSERT_TRUE(solution.known_solution_.has_value());

  std::cout << *solution.known_solution_ << std::endl;
}

} // namespace solver::test
