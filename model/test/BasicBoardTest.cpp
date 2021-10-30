#include "BasicBoard.hpp"
#include "LevelCreator.hpp"
#include <gtest/gtest.h>

namespace model::test {

TEST(BasicBoardTest, constructor) {
  BasicBoard b;
  ASSERT_FALSE(b.is_initialized());
  ASSERT_EQ(0, b.width());
  ASSERT_EQ(0, b.height());
}

TEST(BasicBoardTest, initialize) {
  BasicBoard b;
  b.reset(5, 6);
  ASSERT_TRUE(b.is_initialized());
  ASSERT_EQ(5, b.height());
  ASSERT_EQ(6, b.width());
}

TEST(BasicBoardTest, cell_accessors) {
  BasicBoard b;
  b.reset(5, 6);

  ASSERT_EQ(CellState::Empty, b.get_cell(0, 0));
  ASSERT_EQ(CellState::Empty, b.get_cell(0, 1));
  ASSERT_EQ(CellState::Empty, b.get_cell(1, 2));
  ASSERT_EQ(CellState::Empty, b.get_cell(2, 3));
  ASSERT_EQ(CellState::Empty, b.get_cell(4, 4));

  b.set_cell(0, 0, CellState::Wall0);
  b.set_cell(0, 1, CellState::Wall1);
  b.set_cell(1, 2, CellState::Wall2);
  b.set_cell(2, 3, CellState::Wall3);
  b.set_cell(4, 4, CellState::Mark);

  ASSERT_EQ(CellState::Wall0, b.get_cell(0, 0));
  ASSERT_EQ(CellState::Wall1, b.get_cell(0, 1));
  ASSERT_EQ(CellState::Wall2, b.get_cell(1, 2));
  ASSERT_EQ(CellState::Wall3, b.get_cell(2, 3));
  ASSERT_EQ(CellState::Mark, b.get_cell(4, 4));

  ASSERT_TRUE(b.get_opt_cell(0, 0).has_value());
  ASSERT_TRUE(b.get_opt_cell(0, 1).has_value());
  ASSERT_TRUE(b.get_opt_cell(1, 2).has_value());
  ASSERT_TRUE(b.get_opt_cell(2, 3).has_value());
  ASSERT_TRUE(b.get_opt_cell(4, 4).has_value());
  ASSERT_EQ(CellState::Wall0, *b.get_opt_cell(0, 0));
  ASSERT_EQ(CellState::Wall1, *b.get_opt_cell(0, 1));
  ASSERT_EQ(CellState::Wall2, *b.get_opt_cell(1, 2));
  ASSERT_EQ(CellState::Wall3, *b.get_opt_cell(2, 3));
  ASSERT_EQ(CellState::Mark, *b.get_opt_cell(4, 4));

  auto not_wall1_or_wall2 = [](CellState cell) {
    return cell != CellState::Wall1 && cell != CellState::Wall2;
  };
  b.set_cell_if(0, 0, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if(0, 1, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if(1, 2, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if(2, 3, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if(4, 4, CellState::Empty, not_wall1_or_wall2);

  ASSERT_EQ(CellState::Empty, b.get_cell(0, 0));
  ASSERT_EQ(CellState::Wall1, b.get_cell(0, 1));
  ASSERT_EQ(CellState::Wall2, b.get_cell(1, 2));
  ASSERT_EQ(CellState::Empty, b.get_cell(2, 3));
  ASSERT_EQ(CellState::Empty, b.get_cell(4, 4));
}

TEST(BasicBoardTest, bad_cell_access) {
  BasicBoard b;
  b.reset(5, 6);

  ASSERT_THROW(b.get_cell(-1, 0), std::range_error);
  ASSERT_THROW(b.get_cell(0, -1), std::range_error);
  ASSERT_THROW(b.get_cell(0, -1), std::range_error);
  ASSERT_THROW(b.get_cell(0, 6), std::range_error);

  ASSERT_EQ(std::nullopt, b.get_opt_cell(-1, 0));
  ASSERT_EQ(std::nullopt, b.get_opt_cell(0, -1));
  ASSERT_EQ(std::nullopt, b.get_opt_cell(0, -1));
  ASSERT_EQ(std::nullopt, b.get_opt_cell(0, 6));
}

TEST(BasicBoardTest, equality) {
  BasicBoard   board1, board2;
  LevelCreator creator;
  creator("..*");
  creator("123");
  creator("0X4");
  creator.finished(&board1);

  using enum CellState;

  auto test = [&](BasicBoard const & board) {
    // Top row
    ASSERT_EQ(Empty, board.get_cell(0, 0));
    ASSERT_EQ(Empty, board.get_cell(0, 1));
    ASSERT_EQ(Bulb, board.get_cell(0, 2));

    // Mid row
    ASSERT_EQ(Wall1, board.get_cell(1, 0));
    ASSERT_EQ(Wall2, board.get_cell(1, 1));
    ASSERT_EQ(Wall3, board.get_cell(1, 2));

    // Bot row
    ASSERT_EQ(Wall0, board.get_cell(2, 0));
    ASSERT_EQ(Mark, board.get_cell(2, 1));
    ASSERT_EQ(Wall4, board.get_cell(2, 2));
  };

  board2 = board1;
  test(board1);
  test(board2);

  ASSERT_EQ(board1, board2);
}

TEST(BasicBoardTest, visit_all) {
  BasicBoard   board;
  LevelCreator creator;
  creator("..*");
  creator("123");
  creator("0X4");
  creator.finished(&board);

  BasicBoard board2;
  board2.reset(board.height(), board.width());

  int count = 0;
  board.visit_cells([&](int r, int c, CellState cell) {
    ++count;
    board2.set_cell(r, c, cell);
  });
  ASSERT_EQ(9, count);
  ASSERT_EQ(board, board2);
}

TEST(BasicBoardTest, visit_some) {
  BasicBoard board;
  {
    LevelCreator creator;
    creator("..*");
    creator("123");
    creator("0X4");
    creator.finished(&board);
  }

  BasicBoard board2;
  board2.reset(board.height(), board.width());

  int count = 0;
  board.visit_cells([&](int r, int c, CellState cell) {
    ++count;
    board2.set_cell(r, c, cell);
    return cell != CellState::Wall2;
  });
  ASSERT_EQ(5, count);

  BasicBoard expected;
  {
    LevelCreator creator;
    creator("..*");
    creator("12.");
    creator("...");
    creator.finished(&expected);
  }
  ASSERT_EQ(expected, board2);
}

} // namespace model::test
