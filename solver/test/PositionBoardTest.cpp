#include "PositionBoard.hpp"
#include "ASCIILevelCreator.hpp"
#include "BasicBoard.hpp"
#include "DecisionType.hpp"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

namespace solver::test {

using namespace ::testing;
using namespace model;

TEST(PositionBoardTest, initialize1) {
  ASCIILevelCreator creator;
  creator("0.0");
  creator(".4.");
  creator("0.0");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_EQ(4, position_board.num_cells_needing_illumination());
  EXPECT_EQ(1, position_board.num_walls_with_deps());
}

TEST(PositionBoardTest, initialize2) {
  ASCIILevelCreator creator;
  creator("1*1");
  creator("0+1");
  creator(".+.");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_EQ(2, position_board.num_cells_needing_illumination());
  EXPECT_EQ(1, position_board.num_walls_with_deps());
}

TEST(PositionBoardTest, initialize3) {
  ASCIILevelCreator creator;
  creator("....");
  creator("....");
  creator("....");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_EQ(12, position_board.num_cells_needing_illumination());
  EXPECT_EQ(0, position_board.num_walls_with_deps());
}

TEST(PositionBoardTest, reset_with_dimensions) {
  ASCIILevelCreator creator;
  creator("+*++*");
  creator("*4*++");
  creator("+*+0+");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_THAT(position_board.height(), Eq(3));
  EXPECT_THAT(position_board.width(), Eq(5));
  EXPECT_TRUE(position_board.has_error());

  position_board.reset(4, 2);

  EXPECT_THAT(position_board.height(), Eq(4));
  EXPECT_THAT(position_board.width(), Eq(2));
  EXPECT_FALSE(position_board.has_error());

  int num_empty = 0;
  position_board.visit_board(
      [&](auto, CellState cell) { num_empty += is_empty(cell); });
  EXPECT_THAT(num_empty, Eq(4 * 2));
}

TEST(PositionBoardTest, initialize4a) {
  ASCIILevelCreator creator;
  creator("+*+++");
  creator("*4*++");
  creator("+*+0.");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_EQ(1, position_board.num_cells_needing_illumination());
  EXPECT_EQ(0, position_board.num_walls_with_deps());
}

TEST(PositionBoardTest, initialize4b) {
  ASCIILevelCreator creator;
  creator("+*+++");
  creator("*4*++");
  creator("+*+0*");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_TRUE(position_board.is_solved());
  EXPECT_EQ(0, position_board.num_cells_needing_illumination());
  EXPECT_EQ(0, position_board.num_walls_with_deps());
}

TEST(PositionBoardTest, solved1_pathological) {
  ASCIILevelCreator creator;
  creator("0");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_TRUE(position_board.is_solved());
}

TEST(PositionBoardTest, not_solved1_pathological) {
  ASCIILevelCreator creator;
  creator(".");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
}

TEST(PositionBoardTest, invalid_board) {
  ASCIILevelCreator creator;
  creator("X1");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_TRUE(position_board.has_error());
}

TEST(PositionBoardTest, invalid_board2) {
  ASCIILevelCreator creator;
  creator("X20");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_TRUE(position_board.has_error());
}

TEST(PositionBoardTest, invalid_board3) {
  ASCIILevelCreator creator;
  creator("X3.");
  creator("X..");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_TRUE(position_board.has_error());
}

TEST(PositionBoardTest, board_scenarios4) {
  // 4 cannot be satisfied
  {
    ASCIILevelCreator creator;
    creator("..++..");
    creator("++*+++");
    creator(".X4*++");
    creator("++*+++");
    creator("..++..");
    BasicBoard basic_board;
    creator.finished(&basic_board);
    PositionBoard position_board(basic_board);

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_TRUE(position_board.has_error());
  }

  // error removed (X to left of 4)
  {
    ASCIILevelCreator creator;
    creator("..++..");
    creator("++*+++");
    creator("..4*++");
    creator("++*+++");
    creator("..++..");
    BasicBoard basic_board;
    creator.finished(&basic_board);
    PositionBoard position_board(basic_board);

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_FALSE(position_board.has_error());
  }

  // solved
  {
    ASCIILevelCreator creator;
    creator("++++*");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator("*++++");
    BasicBoard basic_board;
    creator.finished(&basic_board);
    PositionBoard position_board(basic_board);

    EXPECT_TRUE(position_board.is_solved());
    EXPECT_FALSE(position_board.has_error());
  }
}

TEST(PositionBoardTest, add_bulbs1) {
  ASCIILevelCreator creator;
  creator(".....");
  creator(".....");
  creator("..4..");
  creator(".....");
  creator(".....");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(24, position_board.num_cells_needing_illumination());
  EXPECT_EQ(1, position_board.num_walls_with_deps());

  /// move 1
  {
    ASCIILevelCreator creator;
    creator("..+..");
    creator("++*++");
    creator("..4..");
    creator(".....");
    creator(".....");
    BasicBoard board_move1;
    creator.finished(&board_move1);

    position_board.add_bulb({1, 2});

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_EQ(board_move1, position_board.board());
    EXPECT_EQ(18, position_board.num_cells_needing_illumination());
  }

  /// move 2
  {
    ASCIILevelCreator creator;
    creator(".++..");
    creator("++*++");
    creator("+*4..");
    creator(".+...");
    creator(".+...");
    BasicBoard board_move2;
    creator.finished(&board_move2);

    position_board.add_bulb({2, 1});

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_EQ(board_move2, position_board.board());
    EXPECT_EQ(13, position_board.num_cells_needing_illumination());
  }

  /// move 3
  {
    ASCIILevelCreator creator;
    creator(".+++.");
    creator("++*++");
    creator("+*4*+");
    creator(".+.+.");
    creator(".+.+.");
    BasicBoard board_move3;
    creator.finished(&board_move3);

    position_board.add_bulb({2, 3});

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_EQ(board_move3, position_board.board());
    EXPECT_EQ(8, position_board.num_cells_needing_illumination());
  }

  /// move 4
  {
    ASCIILevelCreator creator;
    creator(".+++.");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator(".+++.");
    BasicBoard board_move4;
    creator.finished(&board_move4);

    position_board.add_bulb({3, 2});

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_EQ(board_move4, position_board.board());
    EXPECT_EQ(4, position_board.num_cells_needing_illumination());
  }

  /// move 5
  {
    ASCIILevelCreator creator;
    creator("*++++");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator("++++.");
    BasicBoard board_move5;
    creator.finished(&board_move5);

    position_board.add_bulb({0, 0});

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_EQ(board_move5, position_board.board());
    EXPECT_EQ(1, position_board.num_cells_needing_illumination());
  }

  /// move 6
  {
    ASCIILevelCreator creator;
    creator("*++++");
    creator("++*++");
    creator("+*4*+");
    creator("++*++");
    creator("++++*");
    BasicBoard board_move5;
    creator.finished(&board_move5);

    position_board.add_bulb({4, 4});

    EXPECT_TRUE(position_board.is_solved());
    EXPECT_EQ(board_move5, position_board.board());
    EXPECT_EQ(0, position_board.num_cells_needing_illumination());
  }
}

TEST(PositionBoardTest, add_bulbs2_to_invalid_state) {
  ASCIILevelCreator creator;
  creator(".....");
  creator(".....");
  creator("..4..");
  creator(".....");
  creator(".....");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  /// move 1
  {
    ASCIILevelCreator creator;
    creator("+*+++");
    creator(".+...");
    creator(".+4..");
    creator(".+...");
    creator(".+...");
    BasicBoard board_move1;
    creator.finished(&board_move1);

    position_board.add_bulb({0, 1});

    EXPECT_FALSE(position_board.is_solved());
    EXPECT_TRUE(position_board.has_error());
    EXPECT_EQ(board_move1, position_board.board());
    EXPECT_EQ(15, position_board.num_cells_needing_illumination());
  }
}

TEST(PositionBoardTest, add_bulbs3_to_invalid_state) {
  ASCIILevelCreator creator;
  creator(".....");
  creator(".....");
  creator("..3..");
  creator(".....");
  creator(".....");
  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  /// move 1
  position_board.add_bulb({0, 1});
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(15, position_board.num_cells_needing_illumination());

  /// move 2
  position_board.add_bulb({4, 3});
  EXPECT_TRUE(position_board.has_error());
  EXPECT_EQ(8, position_board.num_cells_needing_illumination());
}

TEST(PositionBoardTest, invalid_state_bulbs_see_each_other) {
  ASCIILevelCreator creator;
  creator("+..+");
  creator("*++*");
  creator("+..+");

  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_TRUE(position_board.has_error());
  EXPECT_EQ(4, position_board.num_cells_needing_illumination());
}

TEST(PositionBoardTest, adding_bulb_makes_wall_unsatisfiable) {
  ASCIILevelCreator creator;
  creator(".12.");
  creator("....");

  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(6, position_board.num_cells_needing_illumination());

  {
    PositionBoard board_copy1 = position_board;
    board_copy1.add_bulb({1, 0});
    EXPECT_TRUE(board_copy1.has_error());
    EXPECT_EQ(DecisionType::WALL_CANNOT_BE_SATISFIED,
              board_copy1.decision_type());
  }
  {
    PositionBoard board_copy2 = position_board;
    board_copy2.add_bulb({1, 3});
    EXPECT_TRUE(board_copy2.has_error());
    EXPECT_EQ(DecisionType::WALL_CANNOT_BE_SATISFIED,
              board_copy2.decision_type());
  }
  {
    PositionBoard board_copy3 = position_board;
    board_copy3.add_bulb({1, 1});
    EXPECT_TRUE(board_copy3.has_error());
    EXPECT_EQ(DecisionType::WALL_CANNOT_BE_SATISFIED,
              board_copy3.decision_type());
  }
}

TEST(PositionBoardTest, adding_bulb_to_nonempty_cell_fails) {
  ASCIILevelCreator creator;
  creator("+0.1.2+");
  creator("*+++0*+");
  creator("+.XX0+*");

  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(5, position_board.num_cells_needing_illumination());

  EXPECT_FALSE(position_board.add_bulb({0, 1}));
  EXPECT_FALSE(position_board.add_bulb({0, 3}));
  EXPECT_FALSE(position_board.add_bulb({0, 5}));
  EXPECT_FALSE(position_board.add_bulb({1, 0}));
  EXPECT_FALSE(position_board.add_bulb({2, 2}));
}

TEST(PositionBoardTest, adding_bulbs_cannot_see_each_other_through_walls) {
  ASCIILevelCreator creator;
  creator(".....");
  creator("..0..");
  creator(".3*4.");
  creator("..2..");
  creator(".....");

  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  // Above - same col as existing bulb
  position_board.add_bulb({0, 2});
  EXPECT_FALSE(position_board.has_error());

  // Below
  position_board.add_bulb({4, 2});
  EXPECT_FALSE(position_board.has_error());

  // From LEFT
  position_board.add_bulb({0, 2});
  EXPECT_FALSE(position_board.has_error());

  // From RIGHT
  position_board.add_bulb({2, 4});
  EXPECT_FALSE(position_board.has_error());
}

TEST(PositionBoardTest, load_board_ok_if_wall_between_bulbs) {
  ASCIILevelCreator creator;
  creator("..*..");
  creator("..0..");
  creator("*3*4*");
  creator("..2..");
  creator("..*..");

  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.has_error());
}

TEST(PositionBoardTest, add_bulb_adjacent_to_multiple_walls_with_deps) {
  ASCIILevelCreator creator;
  creator("010");
  creator("1.1");
  creator("010");

  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.is_solved());
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(1, position_board.num_cells_needing_illumination());
  EXPECT_EQ(4, position_board.num_walls_with_deps());

  position_board.add_bulb({1, 1});

  EXPECT_TRUE(position_board.is_solved());
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(0, position_board.num_cells_needing_illumination());
  EXPECT_EQ(0, position_board.num_walls_with_deps());
}

TEST(PositionBoardTest, add_mark1) {
  ASCIILevelCreator creator;
  creator("....");
  creator("..2.");
  creator("....");

  BasicBoard basic_board;
  creator.finished(&basic_board);
  PositionBoard position_board(basic_board);

  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(11, position_board.num_cells_needing_illumination());

  EXPECT_FALSE(position_board.add_mark({1, 2})); // on wall fails
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(11, position_board.num_cells_needing_illumination());

  // mark 1 ok - NOTE: marks don't decrease num cells needing illum
  position_board.add_mark({0, 2});
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(11, position_board.num_cells_needing_illumination());

  // mark 2 ok
  position_board.add_mark({2, 2});
  EXPECT_FALSE(position_board.has_error());
  EXPECT_EQ(11, position_board.num_cells_needing_illumination());

  // mark 3 error - wall unsatisfiable now
  position_board.add_mark({1, 1});
  EXPECT_TRUE(position_board.has_error()); //
  EXPECT_EQ(11, position_board.num_cells_needing_illumination());

  // final sanity check
  {
    ASCIILevelCreator creator;
    creator("..X.");
    creator(".X2.");
    creator("..X.");

    BasicBoard marked_board;
    creator.finished(&marked_board);

    EXPECT_EQ(marked_board, position_board.board());
  }
}

TEST(PositionBoardTest, add_wall1) {
  ASCIILevelCreator creator;
  creator("...");
  creator("...");
  creator("...");

  BasicBoard basic_board;
  creator.finished(&basic_board);

  PositionBoard board(basic_board);

  EXPECT_EQ(9, board.num_cells_needing_illumination());

  bool result = board.add_wall({1, 1}, CellState::WALL0);
  EXPECT_TRUE(result);
  EXPECT_EQ(0, board.num_walls_with_deps());
  EXPECT_EQ(8, board.num_cells_needing_illumination());
  EXPECT_EQ(CellState::WALL0, board.get_cell({1, 1}));
}

TEST(PositionBoardTest, add_wall2_blocks_bulb) {
  ASCIILevelCreator creator;
  creator(".+.");
  creator("+*+");
  creator(".+.");
  creator(".+.");

  BasicBoard basic_board;
  creator.finished(&basic_board);

  PositionBoard board(basic_board);

  EXPECT_EQ(0, board.num_walls_with_deps());
  EXPECT_EQ(6, board.num_cells_needing_illumination());

  bool result = board.add_wall({2, 1}, CellState::WALL0);
  EXPECT_TRUE(result);
  EXPECT_EQ(0, board.num_walls_with_deps());
  EXPECT_EQ(7, board.num_cells_needing_illumination());
  EXPECT_EQ(CellState::WALL0, board.get_cell({2, 1}));
  EXPECT_EQ(CellState::EMPTY, board.get_cell({3, 1}));
}

TEST(PositionBoardTest, add_wall3_blocks_bulb_with_crossbeams) {
  ASCIILevelCreator creator;
  creator("++...+");
  creator("+*++++");
  creator("++...+");
  creator("*+++++");
  creator("++...+");
  creator("+++++*");
  creator("++...+");

  BasicBoard basic_board;
  creator.finished(&basic_board);

  PositionBoard board(basic_board);
  EXPECT_EQ(12, board.num_cells_needing_illumination());
  bool result = board.add_wall({2, 1}, CellState::WALL0);
  EXPECT_TRUE(result);

  creator("++...+");
  creator("+*++++");
  creator("+0...+");
  creator("*+++++");
  creator("+....+");
  creator("+++++*");
  creator("+....+");
  BasicBoard expected;
  creator.finished(&expected);

  EXPECT_EQ(expected, board.board());

  EXPECT_EQ(0, board.num_walls_with_deps());
  EXPECT_EQ(14, board.num_cells_needing_illumination());
  EXPECT_EQ(CellState::WALL0, board.get_cell({2, 1}));
  EXPECT_EQ(CellState::ILLUM, board.get_cell({3, 1}));
  EXPECT_EQ(CellState::ILLUM, board.get_cell({5, 1}));

  EXPECT_EQ(CellState::EMPTY, board.get_cell({4, 1}));
  EXPECT_EQ(CellState::EMPTY, board.get_cell({6, 1}));
}

TEST(PositionBoardTest, add_wall4_has_deps) {
  ASCIILevelCreator creator;
  creator("...");
  creator("...");
  creator("...");
  creator("...");

  BasicBoard basic_board;
  creator.finished(&basic_board);

  PositionBoard board(basic_board);

  EXPECT_EQ(12, board.num_cells_needing_illumination());

  EXPECT_EQ(0, board.num_walls_with_deps());

  bool result = board.add_wall({0, 1}, CellState::WALL3);
  EXPECT_TRUE(result);
  result = board.add_wall({1, 2}, CellState::WALL2);
  EXPECT_TRUE(result);
  result = board.add_wall({2, 0}, CellState::WALL1);
  EXPECT_TRUE(result);

  EXPECT_EQ(9, board.num_cells_needing_illumination());
  EXPECT_EQ(CellState::WALL3, board.get_cell({0, 1}));
  EXPECT_EQ(CellState::WALL2, board.get_cell({1, 2}));
  EXPECT_EQ(CellState::WALL1, board.get_cell({2, 0}));

  EXPECT_EQ(3, board.num_walls_with_deps());

  creator(".3.");
  creator("..2");
  creator("1..");
  creator("...");

  BasicBoard expected;
  creator.finished(&expected);

  EXPECT_EQ(expected, board.board());
}

TEST(PositionBoardTest, set_cell1_simple) {
  PositionBoard     board(6, 6);
  ASCIILevelCreator creator;

  board.set_cell({0, 0}, CellState::BULB);
  creator("*+++++");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  BasicBoard expected;
  creator.finished(&expected);
  EXPECT_EQ(expected, board.board());

  EXPECT_FALSE(board.has_error());
  board.set_cell({3, 3}, CellState::WALL4);
  board.set_cell({3, 4}, CellState::WALL0);
  EXPECT_TRUE(board.has_error());

  board.set_cell({0, 0}, CellState::BULB);
  creator("*+++++");
  creator("+.....");
  creator("+.....");
  creator("+..40."); // cannot be satisfied
  creator("+.....");
  creator("+.....");

  // "fix" the error
  board.set_cell({3, 4}, CellState::BULB);

  board.set_cell({0, 0}, CellState::BULB);
  creator("*+++++");
  creator("+...+.");
  creator("+...+.");
  creator("+..4*+"); // Fixed?
  creator("+...+.");
  creator("+...+.");

  EXPECT_FALSE(board.has_error());
}

TEST(PositionBoardTest, set_cell2_more_complex) {
  PositionBoard     board(6, 6);
  ASCIILevelCreator creator;
  board.set_cell({0, 0}, CellState::BULB);
  creator("*+++++");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  BasicBoard expected;
  creator.finished(&expected);
  EXPECT_EQ(expected, board.board());

  board.set_cell({0, 1}, CellState::WALL0);
  board.set_cell({0, 0}, CellState::BULB);
  creator("*0....");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator.finished(&expected);
  EXPECT_EQ(expected, board.board());
}

TEST(PositionBoardTest, set_cell3_fix_bulbs_see_eachother) {
  PositionBoard     board(6, 6);
  ASCIILevelCreator creator;

  board.set_cell({0, 0}, CellState::BULB);
  board.set_cell({4, 0}, CellState::BULB);
  creator("*+++++");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("*+++++");
  creator("+.....");
  BasicBoard expected;
  creator.finished(&expected);
  EXPECT_EQ(expected, board.board());

  EXPECT_TRUE(board.has_error());
  EXPECT_EQ(DecisionType::BULBS_SEE_EACH_OTHER, board.decision_type());

  EXPECT_EQ(0, board.num_walls_with_deps());

  board.set_cell({3, 0}, CellState::WALL2);

  creator("*+++++");
  creator("+.....");
  creator("+.....");
  creator("2.....");
  creator("*+++++");
  creator("+.....");
  creator.finished(&expected);
  EXPECT_EQ(expected, board.board());

  EXPECT_FALSE(board.has_error());
  EXPECT_EQ(1, board.num_walls_with_deps());
}

TEST(PositionBoardTest, set_cell4_make_batch_changes) {
  PositionBoard     board(6, 6);
  ASCIILevelCreator creator;

  board.set_cell({0, 0},
                 CellState::BULB,
                 PositionBoard::SetCellPolicy::NO_REEVALUATE_BOARD);

  // bulbs "see each other" but board evaluation is disabled so it shouldn't
  // notice.

  board.set_cell({4, 0},
                 CellState::BULB,
                 PositionBoard::SetCellPolicy::NO_REEVALUATE_BOARD);
  creator("*.....");
  creator("......");
  creator("......");
  creator("......");
  creator("*.....");
  creator("......");
  BasicBoard expected;
  creator.finished(&expected);
  EXPECT_EQ(expected, board.board());

  EXPECT_FALSE(board.has_error());
  board.reevaluate_board_state();
  EXPECT_TRUE(board.has_error());

  creator("*+++++");
  creator("+.....");
  creator("+.....");
  creator("+.....");
  creator("*+++++");
  creator("+.....");
  creator.finished(&expected);
  EXPECT_EQ(expected, board.board());
}

} // namespace solver::test
