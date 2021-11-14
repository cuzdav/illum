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
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_EQ(4, analysisBoard.num_cells_needing_illumination());
  EXPECT_EQ(1, analysisBoard.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize2) {
  model::test::ASCIILevelCreator creator;
  creator("1*1");
  creator("0+1");
  creator(".+.");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_EQ(2, analysisBoard.num_cells_needing_illumination());
  EXPECT_EQ(1, analysisBoard.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize3) {
  model::test::ASCIILevelCreator creator;
  creator("....");
  creator("....");
  creator("....");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_EQ(12, analysisBoard.num_cells_needing_illumination());
  EXPECT_EQ(0, analysisBoard.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize4a) {
  model::test::ASCIILevelCreator creator;
  creator("+*+++");
  creator("*4*++");
  creator("+*+0.");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_EQ(1, analysisBoard.num_cells_needing_illumination());
  EXPECT_EQ(0, analysisBoard.num_walls_with_deps());
}

TEST(AnalysisBoardTest, initialize4b) {
  model::test::ASCIILevelCreator creator;
  creator("+*+++");
  creator("*4*++");
  creator("+*+0*");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_TRUE(analysisBoard.is_solved());
  EXPECT_EQ(0, analysisBoard.num_cells_needing_illumination());
  EXPECT_EQ(0, analysisBoard.num_walls_with_deps());
}

TEST(AnalysisBoardTest, solved1_pathological) {
  model::test::ASCIILevelCreator creator;
  creator("0");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_TRUE(analysisBoard.is_solved());
}

TEST(AnalysisBoardTest, not_solved1_pathological) {
  model::test::ASCIILevelCreator creator;
  creator(".");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
}

TEST(AnalysisBoardTest, invalid_board) {
  model::test::ASCIILevelCreator creator;
  creator("X1");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_TRUE(analysisBoard.has_error());
}

TEST(AnalysisBoardTest, invalid_board2) {
  model::test::ASCIILevelCreator creator;
  creator("X20");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_TRUE(analysisBoard.has_error());
}

TEST(AnalysisBoardTest, invalid_board3) {
  model::test::ASCIILevelCreator creator;
  creator("X3.");
  creator("X..");
  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_TRUE(analysisBoard.has_error());
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
    AnalysisBoard analysisBoard(basic_board);

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_TRUE(analysisBoard.has_error());
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
    AnalysisBoard analysisBoard(basic_board);

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_FALSE(analysisBoard.has_error());
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
    AnalysisBoard analysisBoard(basic_board);

    EXPECT_TRUE(analysisBoard.is_solved());
    EXPECT_FALSE(analysisBoard.has_error());
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
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_FALSE(analysisBoard.is_solved());
  EXPECT_FALSE(analysisBoard.has_error());
  EXPECT_EQ(24, analysisBoard.num_cells_needing_illumination());
  EXPECT_EQ(1, analysisBoard.num_walls_with_deps());

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

    analysisBoard.add_bulb({1, 2});

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_EQ(board_move1, analysisBoard.board());
    EXPECT_EQ(18, analysisBoard.num_cells_needing_illumination());
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

    analysisBoard.add_bulb({2, 1});

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_EQ(board_move2, analysisBoard.board());
    EXPECT_EQ(13, analysisBoard.num_cells_needing_illumination());
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

    analysisBoard.add_bulb({2, 3});

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_EQ(board_move3, analysisBoard.board());
    EXPECT_EQ(8, analysisBoard.num_cells_needing_illumination());
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

    analysisBoard.add_bulb({3, 2});

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_EQ(board_move4, analysisBoard.board());
    EXPECT_EQ(4, analysisBoard.num_cells_needing_illumination());
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

    analysisBoard.add_bulb({0, 0});

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_EQ(board_move5, analysisBoard.board());
    EXPECT_EQ(1, analysisBoard.num_cells_needing_illumination());
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

    analysisBoard.add_bulb({4, 4});

    EXPECT_TRUE(analysisBoard.is_solved());
    EXPECT_EQ(board_move5, analysisBoard.board());
    EXPECT_EQ(0, analysisBoard.num_cells_needing_illumination());
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
  AnalysisBoard analysisBoard(basic_board);

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

    analysisBoard.add_bulb({0, 1});

    EXPECT_FALSE(analysisBoard.is_solved());
    EXPECT_TRUE(analysisBoard.has_error());
    EXPECT_EQ(board_move1, analysisBoard.board());
    EXPECT_EQ(15, analysisBoard.num_cells_needing_illumination());
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
  AnalysisBoard analysisBoard(basic_board);

  /// move 1
  analysisBoard.add_bulb({0, 1});
  EXPECT_FALSE(analysisBoard.has_error());
  EXPECT_EQ(16, analysisBoard.num_cells_needing_illumination());

  /// move 2
  analysisBoard.add_bulb({4, 3});
  EXPECT_TRUE(analysisBoard.has_error());
  EXPECT_EQ(8, analysisBoard.num_cells_needing_illumination());
}

TEST(AnalysisBoardTest, invalid_state_bulbs_see_each_other) {
  model::test::ASCIILevelCreator creator;
  creator("+..+");
  creator("*++*");
  creator("+..+");

  model::BasicBoard basic_board;
  creator.finished(&basic_board);
  AnalysisBoard analysisBoard(basic_board);

  EXPECT_TRUE(analysisBoard.has_error());
  EXPECT_EQ(4, analysisBoard.num_cells_needing_illumination());
}

} // namespace solver::test
