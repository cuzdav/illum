#include "ASCIILevelCreator.hpp"
#include "BasicBoard.hpp"
#include "Solution.hpp"
#include "Solver.hpp"
#include "utils/DebugLog.hpp"
#include <fmt/core.h>
#include <gtest/gtest.h>
#include <iostream>

namespace solver::test {
using namespace ::testing;

class DebugProfileCounter {
public:
  DebugProfileCounter(char const * name, int & value)
      : name_{name}, value_(value) {
    value = 0;
  }

  ~DebugProfileCounter() {
#if defined(DEBUGPROFILE)
    fmt::print("{:35s} {:>11d}\n", name_, value_);
#endif
  }

private:
  char const * name_;
  int &        value_;
};

#define DEBUGPROFILE_COUNTER(COUNTER)                                          \
  DebugProfileCounter COUNTER##_ = {#COUNTER, model::BasicBoard::COUNTER}

// These tests requires some reasoning.  First let's do 1 level.

class SolverSpeculationTest : public ::testing::Test {
  DEBUGPROFILE_COUNTER(visit_cell_counter);
  DEBUGPROFILE_COUNTER(visit_board_counter);
  DEBUGPROFILE_COUNTER(visit_adjacent_counter);
  DEBUGPROFILE_COUNTER(visit_adj_corners_counter);
  DEBUGPROFILE_COUNTER(visit_adj_flank_counter);
  DEBUGPROFILE_COUNTER(visit_empty_counter);
  DEBUGPROFILE_COUNTER(visit_row_left_counter);
  DEBUGPROFILE_COUNTER(visit_row_right_counter);
  DEBUGPROFILE_COUNTER(visit_col_up_counter);
  DEBUGPROFILE_COUNTER(visit_col_down_counter);
  DEBUGPROFILE_COUNTER(visit_perp_counter);
  DEBUGPROFILE_COUNTER(visit_rows_cols_outward_counter);
  DEBUGPROFILE_COUNTER(visit_row_outward_counter);
  DEBUGPROFILE_COUNTER(visit_col_outward_counter);
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

TEST_F(SolverSpeculationTest, realistic_game5) {
  model::test::ASCIILevelCreator creator;
  creator(".........");
  creator(".2.....0.");
  creator("1.3.0.0.0");
  creator(".0.....1.");
  creator("..3...0..");
  creator(".0.....0.");
  creator("3.0.2.2.0");
  creator(".2.....0.");
  creator(".........");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

TEST_F(SolverSpeculationTest, realistic_game6) {
  model::test::ASCIILevelCreator creator;
  creator("....10.2....2..0");
  creator("..0.....1....1.0");
  creator("..0......0...0..");
  creator(".....1.010.....0");
  creator(".200..0..0.1....");
  creator("2...1...1.1.....");
  creator("...2...0...1.01.");
  creator("..0..100......0.");
  creator("...00.0.........");
  creator(".3.....1...0...1");
  creator("0..0.0.1.....2.0");
  creator("1..1...0...20...");
  creator(".....0..0...0...");
  creator("..0..0.0.0......");
  creator("00.0....3.0.21.0");
  creator("0.3.....00......");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

TEST_F(SolverSpeculationTest, realistic_game7) {
  model::test::ASCIILevelCreator creator;
  creator(".2.2.0......2...");
  creator("........0....0.0");
  creator(".00.....12..02..");
  creator("2.010.10.0..0..0");
  creator("....1.....000...");
  creator("....00..100....0");
  creator("..00.1......0...");
  creator(".20..1......0...");
  creator("...0......2..02.");
  creator("...0......0.00..");
  creator("0....100..10....");
  creator("...100.....1....");
  creator("0..0..1.30.000.0");
  creator("..10..00.....00.");
  creator("2.0....1........");
  creator("...0......2.3.2.");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

TEST_F(SolverSpeculationTest, realistic_game8) {
  model::test::ASCIILevelCreator creator;
  creator("01.1...........0.00");
  creator("....2...1.1...1....");
  creator("0.................1");
  creator("1.0.00.......00.2.0");
  creator(".......10.10.......");
  creator("1......0...1......0");
  creator("..3.0...0.0...1.0..");
  creator("...000..0.0..000...");
  creator("...0...........0...");
  creator("1.0..01.....01..0.1");
  creator(".....00.2.0.00.....");
  creator("0...1.0..0..2.0...0");
  creator("0.0.0...300...0.2.2");
  creator("..020....0....010..");
  creator(".2...0.0...1.1...0.");
  creator(".000....0.0....100.");
  creator("..01.02.000.00.00..");
  creator(".....0.1...1.1.....");
  creator("1..0....0.2....0..1");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

TEST_F(SolverSpeculationTest, realistic_game9) {
  model::test::ASCIILevelCreator creator;
  creator("0......00.2.");
  creator("0..001.....0");
  creator(".4...0.0..1.");
  creator("..0.........");
  creator(".2...2......");
  creator("...0...1...1");
  creator("0...0...0...");
  creator("......0...2.");
  creator(".........0..");
  creator(".0..1.2...3.");
  creator("1.....200..0");
  creator(".1.00......0");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

TEST_F(SolverSpeculationTest, realistic_game10) {
  model::test::ASCIILevelCreator creator;
  creator("...1.0.0..");
  creator("..........");
  creator("1..10.....");
  creator(".....2.0.0");
  creator("1..1...0..");
  creator("..0...2..1");
  creator("1.2.0.....");
  creator(".....00..2");
  creator("..........");
  creator("..0.0.2...");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());
  std::cout << solution.board().board() << std::endl;
}

TEST_F(SolverSpeculationTest, realistic_game11) {
  model::test::ASCIILevelCreator creator;
  creator("0...1.00..20");
  creator("0...........");
  creator("..2....0.1..");
  creator("...2.0..0...");
  creator("0.2.0..3...1");
  creator("0.......0...");
  creator("...0.......0");
  creator("2...0..3.0.1");
  creator("...1..1.0...");
  creator("..0.2....0..");
  creator("...........0");
  creator("01..00.1...0");

  model::BasicBoard board;
  creator.finished(&board);
  auto solution = solver::solve(board);

  EXPECT_TRUE(solution.board().is_solved());

  std::cout << solution.board().board() << std::endl;
}

} // namespace solver::test
