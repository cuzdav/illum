#include "BasicBoard.hpp"
#include "LevelCreator.hpp"
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

namespace model::test {

using namespace ::testing;

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
  board.visit_board([&](int r, int c, CellState cell) {
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
  board.visit_board([&](int r, int c, CellState cell) {
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

auto
recorder(auto & moves) {
  return [&](int row, int col, CellState cell) {
    moves.push_back({Action::Add, cell, row, col});
  };
}

auto
recorder(auto & moves, CellState stop) {
  return [&moves, stop](int row, int col, CellState cell) {
    moves.push_back({Action::Add, cell, row, col});
    return cell != stop;
  };
}

auto
mk_move(int r, int c, CellState cell) {
  return SingleMove{Action::Add, cell, r, c};
}

auto
make_board() {
  BasicBoard   board;
  LevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X3");
  creator.finished(&board);
  return board;
}

using Moves = std::vector<SingleMove>;

TEST(BasicBoardTest, visit_left_row_some) {
  Moves moves;
  auto  b = make_board(); // r1: "12.00");

  b.visit_row_left_of(1, 4, recorder(moves, CellState::Empty));

  ASSERT_EQ((std::vector{mk_move(1, 3, CellState::Wall0),
                         mk_move(1, 2, CellState::Empty)}),
            moves);
}

TEST(BasicBoardTest, visit_left_row_some2) {
  Moves moves;
  auto  b = make_board(); // r1: "12.00");

  b.visit_row_left_of(1, 4, recorder(moves, CellState::Bulb));

  ASSERT_EQ((std::vector{mk_move(1, 3, CellState::Wall0),
                         mk_move(1, 2, CellState::Empty),
                         mk_move(1, 1, CellState::Wall2),
                         mk_move(1, 0, CellState::Wall1)}),
            moves);
}

TEST(BasicBoardTest, visit_left_row_some3_invalid) {
  Moves moves;
  auto  b = make_board();

  b.visit_row_left_of(1, 0, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);

  b.visit_row_left_of(-1, 0, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);

  b.visit_row_left_of(0, -4, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);

  b.visit_row_left_of(30, 0, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);
}

TEST(BasicBoardTest, visit_left_row_all) {
  Moves moves;
  make_board().visit_row_left_of(1, 4, recorder(moves));
  ASSERT_EQ((std::vector{mk_move(1, 3, CellState::Wall0),
                         mk_move(1, 2, CellState::Empty),
                         mk_move(1, 1, CellState::Wall2),
                         mk_move(1, 0, CellState::Wall1)}),
            moves);
}

TEST(BasicBoardTest, visit_right_row_some) {
  Moves moves;
  make_board().visit_row_right_of(2, 0, recorder(moves, CellState::Mark));
  // NOTE r2: "...X3"
  ASSERT_EQ((std::vector{mk_move(2, 1, CellState::Empty),
                         mk_move(2, 2, CellState::Empty),
                         mk_move(2, 3, CellState::Mark)}),
            moves);
}

TEST(BasicBoardTest, visit_right_row_all) {
  Moves moves;
  make_board().visit_row_right_of(2, 0, recorder(moves));
  // NOTE r2: "...X3"
  ASSERT_EQ((std::vector{mk_move(2, 1, CellState::Empty),
                         mk_move(2, 2, CellState::Empty),
                         mk_move(2, 3, CellState::Mark),
                         mk_move(2, 4, CellState::Wall3)}),
            moves);
}

TEST(BasicBoardTest, visit_above_col_some) {
  BasicBoard   board;
  LevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...X*");
  creator("...X3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_above(4, 4, recorder(moves, CellState::Wall0));

  ASSERT_EQ((std::vector{
                mk_move(3, 4, CellState::Bulb),
                mk_move(2, 4, CellState::Illum),
                mk_move(1, 4, CellState::Wall0),
            }),
            moves);
}

TEST(BasicBoardTest, visit_above_col_all) {
  BasicBoard   board;
  LevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...X*");
  creator("...X3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_above(4, 4, recorder(moves));

  ASSERT_EQ((std::vector{
                mk_move(3, 4, CellState::Bulb),
                mk_move(2, 4, CellState::Illum),
                mk_move(1, 4, CellState::Wall0),
                mk_move(0, 4, CellState::Empty),
            }),
            moves);
}

TEST(BasicBoardTest, visit_below_col_some) {
  BasicBoard   board;
  LevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...4*");
  creator("...X3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_below(0, 3, recorder(moves, CellState::Wall4));

  ASSERT_EQ((std::vector{
                mk_move(1, 3, CellState::Wall0),
                mk_move(2, 3, CellState::Mark),
                mk_move(3, 3, CellState::Wall4),
            }),
            moves);
}

TEST(BasicBoardTest, visit_below_col_all) {
  BasicBoard   board;
  LevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...4*");
  creator("...*3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_below(0, 3, recorder(moves));

  ASSERT_EQ((std::vector{
                mk_move(1, 3, CellState::Wall0),
                mk_move(2, 3, CellState::Mark),
                mk_move(3, 3, CellState::Wall4),
                mk_move(4, 3, CellState::Bulb),
            }),
            moves);
}

TEST(BasicBoardTest, visit_adjacent_some) {
  BasicBoard   board;
  LevelCreator creator;
  creator("012");
  creator("34*");
  creator("X+.");
  creator.finished(&board);

  // Top Left corner
  Moves moves;
  board.visit_adjacent(0, 0, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(0, 1, CellState::Wall1)),
                                   Eq(mk_move(1, 0, CellState::Wall3))));

  // Top Mid edge
  moves.clear();
  board.visit_adjacent(0, 1, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(0, 0, CellState::Wall0)),
                                   Eq(mk_move(1, 1, CellState::Wall4)),
                                   Eq(mk_move(0, 2, CellState::Wall2))));

  // Top Right corner
  moves.clear();
  board.visit_adjacent(0, 2, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(0, 1, CellState::Wall1)),
                                   Eq(mk_move(1, 2, CellState::Bulb))));

  // Left Mid edge
  moves.clear();
  board.visit_adjacent(1, 0, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(0, 0, CellState::Wall0)),
                                   Eq(mk_move(1, 1, CellState::Wall4)),
                                   Eq(mk_move(2, 0, CellState::Mark))));

  // Center
  moves.clear();
  board.visit_adjacent(1, 1, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(0, 1, CellState::Wall1)),
                                   Eq(mk_move(1, 0, CellState::Wall3)),
                                   Eq(mk_move(1, 2, CellState::Bulb)),
                                   Eq(mk_move(2, 1, CellState::Illum))));

  // Right Mid edge
  moves.clear();
  board.visit_adjacent(1, 2, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(0, 2, CellState::Wall2)),
                                   Eq(mk_move(1, 1, CellState::Wall4)),
                                   Eq(mk_move(2, 2, CellState::Empty))));

  // Bottom Left corner
  moves.clear();
  board.visit_adjacent(2, 0, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(1, 0, CellState::Wall3)),
                                   Eq(mk_move(2, 1, CellState::Illum))));

  // Bottom Mid edge
  moves.clear();
  board.visit_adjacent(2, 1, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(2, 0, CellState::Mark)),
                                   Eq(mk_move(1, 1, CellState::Wall4)),
                                   Eq(mk_move(2, 2, CellState::Empty))));

  // Bottom Right corner
  moves.clear();
  board.visit_adjacent(2, 2, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move(1, 2, CellState::Bulb)),
                                   Eq(mk_move(2, 1, CellState::Illum))));
}

TEST(BasicBoardTest, visit_adjacent_all) {}

} // namespace model::test
