#include "AnalysisBoard.hpp"
#include "ASCIILevelCreator.hpp"
#include "BasicBoard.hpp"
#include "gtest/gtest.h"

namespace solver::test {

TEST(AnalysisBoardTest, initialize1) {
  model::test::ASCIILevelCreator creator;
  creator("0.0");
  creator(".4.");
  creator("0.0");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(4, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(1, analysis_board.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize2) {
  model::test::ASCIILevelCreator creator;
  creator("1*1");
  creator("0+1");
  creator(".+.");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(2, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(1, analysis_board.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize3) {
  model::test::ASCIILevelCreator creator;
  creator("....");
  creator("....");
  creator("....");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(12, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(0, analysis_board.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize4a) {
  model::test::ASCIILevelCreator creator;
  creator("+*+++");
  creator("*4*++");
  creator("+*+0.");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(1, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(0, analysis_board.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize4b) {
  model::test::ASCIILevelCreator creator;
  creator("+*+++");
  creator("*4*++");
  creator("+*+0*");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_TRUE(analysis_board.is_solved());
  EXPECT_EQ(0, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(0, analysis_board.num_walls_with_deps());
}

TEST(AnalysisBoardTest, solved1_pathological) {
  model::test::ASCIILevelCreator creator;
  creator("0");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_TRUE(analysis_board.is_solved());
}

TEST(AnalysisBoardTest, not_solved1_pathological) {
  model::test::ASCIILevelCreator creator;
  creator(".");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
}

TEST(AnalysisBoardTest, invalid_board) {
  model::test::ASCIILevelCreator creator;
  creator("X1");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_TRUE(analysis_board.has_error());
}

TEST(AnalysisBoardTest, invalid_board2) {
  model::test::ASCIILevelCreator creator;
  creator("X20");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_TRUE(analysis_board.has_error());
}

TEST(AnalysisBoardTest, invalid_board3) {
  model::test::ASCIILevelCreator creator;
  creator("X3.");
  creator("X..");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_TRUE(analysis_board.has_error());
}

TEST(AnalysisBoardTest, board_scenarios4) {
  // 4 cannot be satisfied
  {
    model::test::ASCIILevelCreator creator;
    creator("..++..");
    creator("++*+++");
    creator(".X4*++");
    creator("++*+++");
    creator("..++..");
    model::BasicBoard basic_board;
    creator.finished(&basic_board);
    AnalysisBoard analysis_board(basic_board);

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_TRUE(analysis_board.has_error());
  }

  // error removed (X to left of 4)
  {
    model::test::ASCIILevelCreator creator;
    creator("..++..");
    creator("++*+++");
    creator("..4*++");
    creator("++*+++");
    creator("..++..");
    model::BasicBoard basic_board;
    creator.finished(&basic_board);
    AnalysisBoard analysis_board(basic_board);

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_FALSE(analysis_board.has_error());
  }

  // solved
  {
    model::test::ASCIILevelCreator creator;
    creator("++++*");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator("*++++");
    model::BasicBoard basic_board;
    creator.finished(&basic_board);
    AnalysisBoard analysis_board(basic_board);

    EXPECT_TRUE(analysis_board.is_solved());
    EXPECT_FALSE(analysis_board.has_error());
  }
}

TEST(AnalysisBoardTest, add_bulbs1) {
  model::test::ASCIILevelCreator creator;
  creator(".....");
  creator(".....");
  creator("..4..");
  creator(".....");
  creator(".....");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(24, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(1, analysis_board.num_walls_with_deps());

  /// move 1
  {
    model::test::ASCIILevelCreator creator;
    creator("..+..");
    creator("++*++");
    creator("..4..");
    creator(".....");
    creator(".....");
    model::BasicBoard board_move1;
    creator.finished(&board_move1);

    analysis_board.add_bulb({1, 2});

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_EQ(board_move1, analysis_board.board());
    EXPECT_EQ(18, analysis_board.num_cells_needing_illumination());
  }

  /// move 2
  {
    model::test::ASCIILevelCreator creator;
    creator(".++..");
    creator("++*++");
    creator("+*4..");
    creator(".+...");
    creator(".+...");
    model::BasicBoard board_move2;
    creator.finished(&board_move2);

    analysis_board.add_bulb({2, 1});

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_EQ(board_move2, analysis_board.board());
    EXPECT_EQ(13, analysis_board.num_cells_needing_illumination());
  }

  /// move 3
  {
    model::test::ASCIILevelCreator creator;
    creator(".+++.");
    creator("++*++");
    creator("+*4*+");
    creator(".+.+.");
    creator(".+.+.");
    model::BasicBoard board_move3;
    creator.finished(&board_move3);

    analysis_board.add_bulb({2, 3});

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_EQ(board_move3, analysis_board.board());
    EXPECT_EQ(8, analysis_board.num_cells_needing_illumination());
  }

  /// move 4
  {
    model::test::ASCIILevelCreator creator;
    creator(".+++.");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator(".+++.");
    model::BasicBoard board_move4;
    creator.finished(&board_move4);

    analysis_board.add_bulb({3, 2});

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_EQ(board_move4, analysis_board.board());
    EXPECT_EQ(4, analysis_board.num_cells_needing_illumination());
  }

  /// move 5
  {
    model::test::ASCIILevelCreator creator;
    creator("*++++");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator("++++.");
    model::BasicBoard board_move5;
    creator.finished(&board_move5);

    analysis_board.add_bulb({0, 0});

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_EQ(board_move5, analysis_board.board());
    EXPECT_EQ(1, analysis_board.num_cells_needing_illumination());
  }

  /// move 6
  {
    model::test::ASCIILevelCreator creator;
    creator("*++++");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator("++++*");
    model::BasicBoard board_move5;
    creator.finished(&board_move5);

    analysis_board.add_bulb({4, 4});

    EXPECT_TRUE(analysis_board.is_solved());
    EXPECT_EQ(board_move5, analysis_board.board());
    EXPECT_EQ(0, analysis_board.num_cells_needing_illumination());
  }
}

TEST(AnalysisBoardTest, add_bulbs2_to_invalid_state) {
  model::test::ASCIILevelCreator creator;
  creator(".....");
  creator(".....");
  creator("..4..");
  creator(".....");
  creator(".....");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  /// move 1
  {
    model::test::ASCIILevelCreator creator;
    creator("+*+++");
    creator(".+...");
    creator(".+4..");
    creator(".+...");
    creator(".+...");
    model::BasicBoard board_move1;
    creator.finished(&board_move1);

    analysis_board.add_bulb({0, 1});

    EXPECT_FALSE(analysis_board.is_solved());
    EXPECT_TRUE(analysis_board.has_error());
    EXPECT_EQ(board_move1, analysis_board.board());
    EXPECT_EQ(15, analysis_board.num_cells_needing_illumination());
  }
}

TEST(AnalysisBoardTest, add_bulbs3_to_invalid_state) {
  model::test::ASCIILevelCreator creator;
  creator(".....");
  creator(".....");
  creator("..3..");
  creator(".....");
  creator(".....");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  /// move 1
  analysis_board.add_bulb({0, 1});
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(15, analysis_board.num_cells_needing_illumination());

  /// move 2
  analysis_board.add_bulb({4, 3});
  EXPECT_TRUE(analysis_board.has_error());
  EXPECT_EQ(8, analysis_board.num_cells_needing_illumination());
}

TEST(AnalysisBoardTest, invalid_state_bulbs_see_each_other) {
  model::test::ASCIILevelCreator creator;
  creator("+..+");
  creator("*++*");
  creator("+..+");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_TRUE(analysis_board.has_error());
  EXPECT_EQ(4, analysis_board.num_cells_needing_illumination());
}

TEST(AnalysisBoardTest, adding_bulb_to_nonempty_cell_fails) {
  model::test::ASCIILevelCreator creator;
  creator("+0.1.2+");
  creator("*+++0*+");
  creator("+.XX0+*");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(5, analysis_board.num_cells_needing_illumination());

  ASSERT_FALSE(analysis_board.add_bulb({0, 1}));
  ASSERT_FALSE(analysis_board.add_bulb({0, 3}));
  ASSERT_FALSE(analysis_board.add_bulb({0, 5}));
  ASSERT_FALSE(analysis_board.add_bulb({1, 0}));
  ASSERT_FALSE(analysis_board.add_bulb({2, 2}));
}

TEST(AnalysisBoardTest, add_bulbs_that_see_each_other) {
  model::test::ASCIILevelCreator creator;
  creator(".....");
  creator(".....");
  creator("..*..");
  creator(".....");
  creator(".....");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  // Above - same col as existing bulb
  analysis_board.clone_position();
  analysis_board.add_bulb({0, 2});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();

  analysis_board.clone_position();
  EXPECT_FALSE(analysis_board.has_error());
  analysis_board.add_bulb({1, 2});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();
  EXPECT_FALSE(analysis_board.has_error());

  // Below
  analysis_board.clone_position();
  analysis_board.add_bulb({4, 2});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();

  analysis_board.clone_position();
  EXPECT_FALSE(analysis_board.has_error());
  analysis_board.add_bulb({3, 2});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();
  EXPECT_FALSE(analysis_board.has_error());

  // From Left
  analysis_board.clone_position();
  analysis_board.add_bulb({0, 2});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();

  analysis_board.clone_position();
  EXPECT_FALSE(analysis_board.has_error());
  analysis_board.add_bulb({1, 2});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();
  EXPECT_FALSE(analysis_board.has_error());

  // From Right
  analysis_board.clone_position();
  analysis_board.add_bulb({2, 4});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();

  analysis_board.clone_position();
  EXPECT_FALSE(analysis_board.has_error());
  analysis_board.add_bulb({2, 3});
  EXPECT_TRUE(analysis_board.has_error());
  analysis_board.pop();
  EXPECT_FALSE(analysis_board.has_error());
}

TEST(AnalysisBoardTest, adding_bulbs_cannot_see_each_other_through_walls) {
  model::test::ASCIILevelCreator creator;
  creator(".....");
  creator("..0..");
  creator(".3*4.");
  creator("..2..");
  creator(".....");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  // Above - same col as existing bulb
  analysis_board.add_bulb({0, 2});
  EXPECT_FALSE(analysis_board.has_error());

  // Below
  analysis_board.add_bulb({4, 2});
  EXPECT_FALSE(analysis_board.has_error());

  // From Left
  analysis_board.add_bulb({0, 2});
  EXPECT_FALSE(analysis_board.has_error());

  // From Right
  analysis_board.add_bulb({2, 4});
  EXPECT_FALSE(analysis_board.has_error());
}

TEST(AnalysisBoardTest, load_board_ok_if_wall_between_bulbs) {
  model::test::ASCIILevelCreator creator;
  creator("..*..");
  creator("..0..");
  creator("*3*4*");
  creator("..2..");
  creator("..*..");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.has_error());
}

TEST(AnalysisBoardTest, add_bulb_adjacent_to_multiple_walls_with_deps) {
  model::test::ASCIILevelCreator creator;
  creator("010");
  creator("1.1");
  creator("010");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(1, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(4, analysis_board.num_walls_with_deps());

  analysis_board.add_bulb({1, 1});

  EXPECT_TRUE(analysis_board.is_solved());
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(0, analysis_board.num_cells_needing_illumination());
  EXPECT_EQ(0, analysis_board.num_walls_with_deps());
}

TEST(AnalysisBoardTest, add_mark1) {
  model::test::ASCIILevelCreator creator;
  creator("....");
  creator("..2.");
  creator("....");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysis_board(basic_board);

  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(11, analysis_board.num_cells_needing_illumination());

  ASSERT_FALSE(analysis_board.add_mark({1, 2})); // on wall fails
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(11, analysis_board.num_cells_needing_illumination());

  // mark 1 ok - NOTE: marks don't decrease num cells needing illum
  analysis_board.add_mark({0, 2});
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(11, analysis_board.num_cells_needing_illumination());

  // mark 2 ok
  analysis_board.add_mark({2, 2});
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_EQ(11, analysis_board.num_cells_needing_illumination());

  // mark 3 error - wall unsatisfiable now
  analysis_board.add_mark({1, 1});
  EXPECT_TRUE(analysis_board.has_error()); //
  EXPECT_EQ(11, analysis_board.num_cells_needing_illumination());

  // final sanity check
  {
    model::test::ASCIILevelCreator creator;
    creator("..X.");
    creator(".X2.");
    creator("..X.");

    model::BasicBoard marked_board;
    creator.finished(&marked_board);

    ASSERT_EQ(marked_board, analysis_board.board());
  }
}

TEST(AnalysisBoardTest, pop1) {
  model::test::ASCIILevelCreator creator;
  creator("...");
  creator(".3.");
  creator("...");
  model::BasicBoard board;
  creator.finished(&board);
  AnalysisBoard analysis_board(board);

  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(8, analysis_board.num_cells_needing_illumination());

  // MOVE 1
  analysis_board.clone_position();
  analysis_board.add_bulb({0, 0});
  EXPECT_TRUE(analysis_board.has_error()); // unsatisfiable
  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(3, analysis_board.num_cells_needing_illumination());

  // MOVE 2
  analysis_board.clone_position();
  analysis_board.add_bulb({2, 2});
  EXPECT_TRUE(analysis_board.has_error()); // unsatisfiable
  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(0, analysis_board.num_cells_needing_illumination());

  // POP MOVE2 back to move1 state
  analysis_board.pop();
  EXPECT_TRUE(analysis_board.has_error()); // unsatisfiable
  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(3, analysis_board.num_cells_needing_illumination());

  // POP MOVE1 back to original board
  analysis_board.pop();
  EXPECT_FALSE(analysis_board.has_error());
  EXPECT_FALSE(analysis_board.is_solved());
  EXPECT_EQ(8, analysis_board.num_cells_needing_illumination());
}

} // namespace solver::test
