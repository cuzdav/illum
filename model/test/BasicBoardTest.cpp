#include "BasicBoard.hpp"
#include "ASCIILevelCreator.hpp"
#include "CellVisitorConcepts.hpp"
#include "Direction.hpp"
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

namespace model::test {

using enum Direction;

struct SingleMoveDirection : SingleMove {
  Direction direction_;
  bool      operator==(SingleMoveDirection const &) const = default;

  SingleMoveDirection(
      Direction dir, Action action, CellState from, CellState to, Coord coord)
      : SingleMove{action, from, to, coord}, direction_(dir) {}
};

std::ostream &
operator<<(std::ostream & os, SingleMoveDirection const & smd) {
  os << "[Direction: " << smd.direction_ << ", "
     << static_cast<SingleMove const &>(smd) << "]";
  return os;
}

using Moves          = std::vector<SingleMove>;
using DirectionMoves = std::vector<SingleMoveDirection>;

auto
mk_move(Coord coord, CellState cell) {
  return SingleMove{Action::Add, CellState::Empty, cell, coord};
}

auto
mk_move(Direction dir, Coord coord, CellState cell) {
  return SingleMoveDirection(dir, Action::Add, CellState::Empty, cell, coord);
}

auto
recorder(Moves & moves) {
  return [&](Coord coord, CellState cell) {
    moves.push_back(mk_move(coord, cell));
  };
}

auto
recorder(Moves & moves, CellState stop_if_cell_is) {
  return [&moves, stop_if_cell_is](Coord coord, CellState cell) {
    moves.push_back(mk_move(coord, cell));
    return cell != stop_if_cell_is ? KEEP_VISITING : STOP_VISITING;
  };
}

auto
recorder(DirectionMoves & moves) {
  return [&](Direction dir, Coord coord, CellState cell) {
    moves.push_back(mk_move(dir, coord, cell));
  };
}

auto
recorder(DirectionMoves & moves, CellState stop_if_cell_is) {
  return [&moves, stop_if_cell_is](Direction dir, Coord coord, CellState cell) {
    moves.push_back(mk_move(dir, coord, cell));
    return cell != stop_if_cell_is ? KEEP_VISITING : STOP_VISITING;
  };
}

auto
make_board() {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X3");
  creator.finished(&board);
  return board;
}

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

  ASSERT_EQ(CellState::Empty, b.get_cell({0, 0}));
  ASSERT_EQ(CellState::Empty, b.get_cell({0, 1}));
  ASSERT_EQ(CellState::Empty, b.get_cell({1, 2}));
  ASSERT_EQ(CellState::Empty, b.get_cell({2, 3}));
  ASSERT_EQ(CellState::Empty, b.get_cell({4, 4}));

  b.set_cell({0, 0}, CellState::Wall0);
  b.set_cell({0, 1}, CellState::Wall1);
  b.set_cell({1, 2}, CellState::Wall2);
  b.set_cell({2, 3}, CellState::Wall3);
  b.set_cell({4, 4}, CellState::Mark);

  ASSERT_EQ(CellState::Wall0, b.get_cell({0, 0}));
  ASSERT_EQ(CellState::Wall1, b.get_cell({0, 1}));
  ASSERT_EQ(CellState::Wall2, b.get_cell({1, 2}));
  ASSERT_EQ(CellState::Wall3, b.get_cell({2, 3}));
  ASSERT_EQ(CellState::Mark, b.get_cell({4, 4}));

  ASSERT_TRUE(b.get_opt_cell({0, 0}).has_value());
  ASSERT_TRUE(b.get_opt_cell({0, 1}).has_value());
  ASSERT_TRUE(b.get_opt_cell({1, 2}).has_value());
  ASSERT_TRUE(b.get_opt_cell({2, 3}).has_value());
  ASSERT_TRUE(b.get_opt_cell({4, 4}).has_value());
  ASSERT_EQ(CellState::Wall0, *b.get_opt_cell({0, 0}));
  ASSERT_EQ(CellState::Wall1, *b.get_opt_cell({0, 1}));
  ASSERT_EQ(CellState::Wall2, *b.get_opt_cell({1, 2}));
  ASSERT_EQ(CellState::Wall3, *b.get_opt_cell({2, 3}));
  ASSERT_EQ(CellState::Mark, *b.get_opt_cell({4, 4}));

  auto not_wall1_or_wall2 = [](CellState cell) {
    return cell != CellState::Wall1 && cell != CellState::Wall2;
  };
  b.set_cell_if({0, 0}, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if({0, 1}, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if({1, 2}, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if({2, 3}, CellState::Empty, not_wall1_or_wall2);
  b.set_cell_if({4, 4}, CellState::Empty, not_wall1_or_wall2);

  ASSERT_EQ(CellState::Empty, b.get_cell({0, 0}));
  ASSERT_EQ(CellState::Wall1, b.get_cell({0, 1}));
  ASSERT_EQ(CellState::Wall2, b.get_cell({1, 2}));
  ASSERT_EQ(CellState::Empty, b.get_cell({2, 3}));
  ASSERT_EQ(CellState::Empty, b.get_cell({4, 4}));
}

TEST(BasicBoardTest, bad_cell_access) {
  BasicBoard b;
  b.reset(5, 6);

  ASSERT_THROW(b.get_cell({-1, 0}), std::range_error);
  ASSERT_THROW(b.get_cell({0, -1}), std::range_error);
  ASSERT_THROW(b.get_cell({0, -1}), std::range_error);
  ASSERT_THROW(b.get_cell({0, 6}), std::range_error);

  ASSERT_EQ(std::nullopt, b.get_opt_cell({-1, 0}));
  ASSERT_EQ(std::nullopt, b.get_opt_cell({0, -1}));
  ASSERT_EQ(std::nullopt, b.get_opt_cell({0, -1}));
  ASSERT_EQ(std::nullopt, b.get_opt_cell({0, 6}));
}

TEST(BasicBoardTest, equality) {
  BasicBoard        board1, board2;
  ASCIILevelCreator creator;
  creator("..*");
  creator("123");
  creator("0X4");
  creator.finished(&board1);

  using enum CellState;

  auto test = [&](BasicBoard const & board) {
    // Top row
    ASSERT_EQ(Empty, board.get_cell({0, 0}));
    ASSERT_EQ(Empty, board.get_cell({0, 1}));
    ASSERT_EQ(Bulb, board.get_cell({0, 2}));

    // Mid row
    ASSERT_EQ(Wall1, board.get_cell({1, 0}));
    ASSERT_EQ(Wall2, board.get_cell({1, 1}));
    ASSERT_EQ(Wall3, board.get_cell({1, 2}));

    // Bot row
    ASSERT_EQ(Wall0, board.get_cell({2, 0}));
    ASSERT_EQ(Mark, board.get_cell({2, 1}));
    ASSERT_EQ(Wall4, board.get_cell({2, 2}));
  };

  board2 = board1;
  test(board1);
  test(board2);

  ASSERT_EQ(board1, board2);
}

TEST(BasicBoardTest, visit_all) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*");
  creator("123");
  creator("0X4");
  creator.finished(&board);

  BasicBoard board2;
  board2.reset(board.height(), board.width());

  int count = 0;
  board.visit_board([&](Coord coord, CellState cell) {
    ++count;
    board2.set_cell(coord, cell);
  });
  ASSERT_EQ(9, count);
  ASSERT_EQ(board, board2);
}

TEST(BasicBoardTest, visit_some) {
  BasicBoard board;
  {
    ASCIILevelCreator creator;
    creator("..*");
    creator("123");
    creator("0X4");
    creator.finished(&board);
  }

  BasicBoard board2;
  board2.reset(board.height(), board.width());

  int count = 0;
  board.visit_board([&](Coord coord, CellState cell) {
    ++count;
    board2.set_cell(coord, cell);
    return cell != CellState::Wall2 ? KEEP_VISITING : STOP_VISITING;
  });
  ASSERT_EQ(5, count);

  BasicBoard expected;
  {
    ASCIILevelCreator creator;
    creator("..*");
    creator("12.");
    creator("...");
    creator.finished(&expected);
  }
  ASSERT_EQ(expected, board2);
}

TEST(BasicBoardTest, visit_left_row_some) {
  Moves moves;
  auto  b = make_board(); // r1: "12.00");

  b.visit_row_left_of({1, 4}, recorder(moves, CellState::Empty));

  // stops at wall to left of cooord
  ASSERT_EQ((std::vector{mk_move({1, 3}, CellState::Wall0)}), moves);

  moves.clear();
  b.visit_row_left_of({1, 3}, recorder(moves, CellState::Empty));

  // stops because it sees Empty
  ASSERT_EQ((std::vector{mk_move({1, 2}, CellState::Empty)}), moves);
}

TEST(BasicBoardTest, visit_directional_left_row_some) {
  DirectionMoves moves;
  auto           b = make_board(); // r1: "12.00");

  b.visit_row_left_of({1, 4}, recorder(moves, CellState::Empty));

  // stops at wall to left of cooord
  ASSERT_EQ((std::vector{mk_move(Left, {1, 3}, CellState::Wall0)}), moves);

  moves.clear();
  b.visit_row_left_of({1, 3}, recorder(moves, CellState::Empty));

  // stops because it sees Empty
  ASSERT_EQ((std::vector{mk_move(Left, {1, 2}, CellState::Empty)}), moves);
}

TEST(BasicBoardTest, visit_left_row_some2) {
  Moves moves;
  auto  b = make_board(); // row0: "..*.."

  b.visit_row_left_of({0, 4}, recorder(moves, CellState::Bulb));

  ASSERT_EQ((std::vector{mk_move({0, 3}, CellState::Empty),
                         mk_move({0, 2}, CellState::Bulb)}),

            moves);
}

TEST(BasicBoardTest, visit_left_row_some3_invalid) {
  Moves moves;
  auto  b = make_board();

  b.visit_row_left_of({1, 0}, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);

  b.visit_row_left_of({-1, 0}, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);

  b.visit_row_left_of({0, -4}, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);

  b.visit_row_left_of({30, 0}, recorder(moves, CellState::Bulb));
  ASSERT_EQ(Moves{}, moves);
}

TEST(BasicBoardTest, visit_left_row_all) {
  Moves moves;
  // r2: "...X3"
  make_board().visit_row_left_of({2, 4}, recorder(moves));
  ASSERT_EQ((std::vector{mk_move({2, 3}, CellState::Mark),
                         mk_move({2, 2}, CellState::Empty),
                         mk_move({2, 1}, CellState::Empty),
                         mk_move({2, 0}, CellState::Empty)}),
            moves);
}

TEST(BasicBoardTest, visit_directional_left_row_all) {
  DirectionMoves moves;
  // r2: "...X3"
  make_board().visit_row_left_of({2, 4}, recorder(moves));
  ASSERT_EQ((std::vector{mk_move(Left, {2, 3}, CellState::Mark),
                         mk_move(Left, {2, 2}, CellState::Empty),
                         mk_move(Left, {2, 1}, CellState::Empty),
                         mk_move(Left, {2, 0}, CellState::Empty)}),
            moves);
}

TEST(BasicBoardTest, visit_right_row_some) {
  Moves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves, CellState::Mark));
  // NOTE row 2 looks like: "...X3"
  ASSERT_EQ((std::vector{mk_move({2, 1}, CellState::Empty),
                         mk_move({2, 2}, CellState::Empty),
                         mk_move({2, 3}, CellState::Mark)}),
            moves);
}

TEST(BasicBoardTest, visit_directional_right_row_some) {
  DirectionMoves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves, CellState::Mark));
  // NOTE row 2 looks like: "...X3"
  ASSERT_EQ((std::vector{mk_move(Right, {2, 1}, CellState::Empty),
                         mk_move(Right, {2, 2}, CellState::Empty),
                         mk_move(Right, {2, 3}, CellState::Mark)}),
            moves);
}

TEST(BasicBoardTest, visit_directional_right_row_all) {
  DirectionMoves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves));
  // NOTE r2 looks like: "...X3"
  ASSERT_EQ((std::vector{mk_move(Right, {2, 1}, CellState::Empty),
                         mk_move(Right, {2, 2}, CellState::Empty),
                         mk_move(Right, {2, 3}, CellState::Mark),
                         mk_move(Right, {2, 4}, CellState::Wall3)}),
            moves);
}
TEST(BasicBoardTest, visit_right_row_all) {
  Moves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves));
  // NOTE r2 looks like: "...X3"
  ASSERT_EQ((std::vector{mk_move({2, 1}, CellState::Empty),
                         mk_move({2, 2}, CellState::Empty),
                         mk_move({2, 3}, CellState::Mark),
                         mk_move({2, 4}, CellState::Wall3)}),
            moves);
}

TEST(BasicBoardTest, visit_above_col_some) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...X*");
  creator("...X3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_above({4, 4}, recorder(moves, CellState::Wall0));

  ASSERT_EQ((std::vector{
                mk_move({3, 4}, CellState::Bulb),
                mk_move({2, 4}, CellState::Illum),
                mk_move({1, 4}, CellState::Wall0),
            }),
            moves);
}

TEST(BasicBoardTest, visit_directional_above_col_some) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...X*");
  creator("...X3");
  creator.finished(&board);

  DirectionMoves moves;
  board.visit_col_above({4, 4}, recorder(moves, CellState::Wall0));

  ASSERT_EQ((std::vector{
                mk_move(Up, {3, 4}, CellState::Bulb),
                mk_move(Up, {2, 4}, CellState::Illum),
                mk_move(Up, {1, 4}, CellState::Wall0),
            }),
            moves);
}

TEST(BasicBoardTest, visit_above_col_all) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...X*");
  creator("...X3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_above({4, 4}, recorder(moves));

  ASSERT_EQ((std::vector{
                mk_move({3, 4}, CellState::Bulb),
                mk_move({2, 4}, CellState::Illum),
                mk_move({1, 4}, CellState::Wall0),
            }),
            moves);
}

TEST(BasicBoardTest, visit_directional_above_col_all) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*..");
  creator("12.00");
  creator("...X+");
  creator("...X*");
  creator("...X3");
  creator.finished(&board);

  DirectionMoves moves;
  board.visit_col_above({4, 4}, recorder(moves));

  ASSERT_EQ((std::vector{
                mk_move(Up, {3, 4}, CellState::Bulb),
                mk_move(Up, {2, 4}, CellState::Illum),
                mk_move(Up, {1, 4}, CellState::Wall0),
            }),
            moves);
}

TEST(BasicBoardTest, visit_below_col_some) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*..");
  creator("12.X0");
  creator("...X+");
  creator("...4*");
  creator("...X3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_below({0, 3}, recorder(moves, CellState::Wall4));

  ASSERT_EQ((std::vector{
                mk_move({1, 3}, CellState::Mark),
                mk_move({2, 3}, CellState::Mark),
                mk_move({3, 3}, CellState::Wall4),
            }),
            moves);
}

TEST(BasicBoardTest, visit_directional_below_col_some) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..*..");
  creator("12.X0");
  creator("...X+");
  creator("...4*");
  creator("...X3");
  creator.finished(&board);

  DirectionMoves moves;
  board.visit_col_below({0, 3}, recorder(moves, CellState::Wall4));

  ASSERT_EQ((std::vector{
                mk_move(Down, {1, 3}, CellState::Mark),
                mk_move(Down, {2, 3}, CellState::Mark),
                mk_move(Down, {3, 3}, CellState::Wall4),
            }),
            moves);
}

TEST(BasicBoardTest, visit_below_col_all) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("++*++");
  creator("12+00");
  creator("..+X+");
  creator("..+4*");
  creator("..+*3");
  creator.finished(&board);

  Moves moves;
  board.visit_col_below({0, 2}, recorder(moves));

  ASSERT_EQ((std::vector{
                mk_move({1, 2}, CellState::Illum),
                mk_move({2, 2}, CellState::Illum),
                mk_move({3, 2}, CellState::Illum),
                mk_move({4, 2}, CellState::Illum),
            }),
            moves);
}

TEST(BasicBoardTest, visit_directional_below_col_all) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("++*++");
  creator("12+00");
  creator("..+X+");
  creator("..+4*");
  creator("..+*3");
  creator.finished(&board);

  DirectionMoves moves;
  board.visit_col_below({0, 2}, recorder(moves));

  ASSERT_EQ((std::vector{
                mk_move(Down, {1, 2}, CellState::Illum),
                mk_move(Down, {2, 2}, CellState::Illum),
                mk_move(Down, {3, 2}, CellState::Illum),
                mk_move(Down, {4, 2}, CellState::Illum),
            }),
            moves);
}

TEST(BasicBoardTest, visit_adjacent_all) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("012");
  creator("34*");
  creator("X+.");
  creator.finished(&board);

  // Top Left corner
  Moves moves;
  board.visit_adjacent({0, 0}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({0, 1}, CellState::Wall1)),
                                   Eq(mk_move({1, 0}, CellState::Wall3))));

  // Top Mid edge
  moves.clear();
  board.visit_adjacent({0, 1}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({0, 0}, CellState::Wall0)),
                                   Eq(mk_move({1, 1}, CellState::Wall4)),
                                   Eq(mk_move({0, 2}, CellState::Wall2))));

  // Top Right corner
  moves.clear();
  board.visit_adjacent({0, 2}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({0, 1}, CellState::Wall1)),
                                   Eq(mk_move({1, 2}, CellState::Bulb))));

  // Left Mid edge
  moves.clear();
  board.visit_adjacent({1, 0}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({0, 0}, CellState::Wall0)),
                                   Eq(mk_move({1, 1}, CellState::Wall4)),
                                   Eq(mk_move({2, 0}, CellState::Mark))));

  // Center
  moves.clear();
  board.visit_adjacent({1, 1}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({0, 1}, CellState::Wall1)),
                                   Eq(mk_move({1, 0}, CellState::Wall3)),
                                   Eq(mk_move({1, 2}, CellState::Bulb)),
                                   Eq(mk_move({2, 1}, CellState::Illum))));

  // Right Mid edge
  moves.clear();
  board.visit_adjacent({1, 2}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({0, 2}, CellState::Wall2)),
                                   Eq(mk_move({1, 1}, CellState::Wall4)),
                                   Eq(mk_move({2, 2}, CellState::Empty))));

  // Bottom Left corner
  moves.clear();
  board.visit_adjacent({2, 0}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({1, 0}, CellState::Wall3)),
                                   Eq(mk_move({2, 1}, CellState::Illum))));

  // Bottom Mid edge
  moves.clear();
  board.visit_adjacent({2, 1}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({2, 0}, CellState::Mark)),
                                   Eq(mk_move({1, 1}, CellState::Wall4)),
                                   Eq(mk_move({2, 2}, CellState::Empty))));

  // Bottom Right corner
  moves.clear();
  board.visit_adjacent({2, 2}, recorder(moves));
  EXPECT_THAT(moves,
              UnorderedElementsAre(Eq(mk_move({1, 2}, CellState::Bulb)),
                                   Eq(mk_move({2, 1}, CellState::Illum))));
}

TEST(BasicBoardTest, visit_perpendiculars) {
  BasicBoard        board;
  ASCIILevelCreator creator;
  creator("..X+0");
  creator("X..+0");
  creator("+++*+");
  creator("1..+.");
  creator.finished(&board);

  // If moving up from 2,0 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({2, 0}, Direction::Up, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::Right, {2, 1}, CellState::Illum)),
                    Eq(mk_move(Direction::Right, {2, 2}, CellState::Illum)),
                    Eq(mk_move(Direction::Right, {2, 3}, CellState::Bulb)),
                    Eq(mk_move(Direction::Right, {2, 4}, CellState::Illum))));
  }
  // If moving up from 1,2 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({1, 2}, Direction::Up, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::Left, {1, 0}, CellState::Mark)),
                    Eq(mk_move(Direction::Left, {1, 1}, CellState::Empty)),
                    Eq(mk_move(Direction::Right, {1, 3}, CellState::Illum)),
                    Eq(mk_move(Direction::Right, {1, 4}, CellState::Wall0))));
  }

  // If moving down from 2,0 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({2, 0}, Direction::Down, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::Right, {2, 1}, CellState::Illum)),
                    Eq(mk_move(Direction::Right, {2, 2}, CellState::Illum)),
                    Eq(mk_move(Direction::Right, {2, 3}, CellState::Bulb)),
                    Eq(mk_move(Direction::Right, {2, 4}, CellState::Illum))));
  }
  // If moving down from 1,2 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({1, 2}, Direction::Down, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::Left, {1, 0}, CellState::Mark)),
                    Eq(mk_move(Direction::Left, {1, 1}, CellState::Empty)),
                    Eq(mk_move(Direction::Right, {1, 3}, CellState::Illum)),
                    Eq(mk_move(Direction::Right, {1, 4}, CellState::Wall0))));
  }

  // If moving right at 2,0 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({2, 0}, Direction::Right, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::Up, {0, 0}, CellState::Empty)),
                    Eq(mk_move(Direction::Up, {1, 0}, CellState::Mark)),
                    Eq(mk_move(Direction::Down, {3, 0}, CellState::Wall1))));
  }
  // If moving right at 3,4 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({3, 4}, Direction::Right, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::Up, {2, 4}, CellState::Illum)),
                    Eq(mk_move(Direction::Up, {1, 4}, CellState::Wall0))));
  }
}

} // namespace model::test
