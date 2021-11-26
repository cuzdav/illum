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

class SolverSpeculationTest : public ::testing::Test {
public:
  SolverSpeculationTest() { model::BasicBoard::visit_cell_counter = 0; }
  ~SolverSpeculationTest() {
    if (model::BasicBoard::visit_cell_counter > 0) {
      std::cout << "Took " << model::BasicBoard::visit_cell_counter
                << " cell visitations.\n";
    }
    else {
      std::cout << "odd, no visitations?\n";
    }
  }
};

TEST_F(SolverSpeculationTest, detects_board_with_multiple_solutions) {
  model::test::ASCIILevelCreator creator;
  creator("..");
  creator("..");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  EXPECT_FALSE(solution.board().is_solved());
  EXPECT_TRUE(solution.board().has_error());
  EXPECT_EQ(DecisionType::VIOLATES_SINGLE_UNIQUE_SOLUTION,
            solution.board().decision_type());
  std::cout << board << std::endl;
}

TEST_F(SolverSpeculationTest, detects_board_with_unilluminable_marks) {
  model::test::ASCIILevelCreator creator;
  creator("XX");
  creator("XX");
  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);
  EXPECT_FALSE(solution.board().is_solved());
  EXPECT_TRUE(solution.board().has_error());
  EXPECT_EQ(DecisionType::MARK_CANNOT_BE_ILLUMINATED,
            solution.board().decision_type());
  std::cout << board << std::endl;
}

TEST_F(SolverSpeculationTest, simple_deductions1) {
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

TEST_F(SolverSpeculationTest, simple_deductions2) {
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

TEST_F(SolverSpeculationTest, realistic_game1) {
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

TEST_F(SolverSpeculationTest, realistic_game2) {
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

TEST_F(SolverSpeculationTest, realistic_game3) {
  model::test::ASCIILevelCreator creator;
  creator(".......0.......");
  creator(".301.......010.");
  creator("0...0.....3...0");
  creator("0......0......0");
  creator("01..0.....2..10");
  creator(".....2...0.....");
  creator("......2.1......");
  creator("1...1.101.0...1");
  creator(".1.10.....00.0.");
  creator("...............");
  creator("...............");
  creator(".3..1.0.0.2..1.");
  creator("...0.3...0.2...");
  creator("0.0.0.0.0.0.0.0");
  creator(".......2.......");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

TEST_F(SolverSpeculationTest, realistic_game4) {
  model::test::ASCIILevelCreator creator;
  creator("0.....");
  creator("..2.3.");
  creator("...0..");
  creator("..2...");
  creator(".3.0..");
  creator(".....0");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

} // namespace solver::test
