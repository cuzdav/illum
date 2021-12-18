#include "BasicBoard.hpp"
#include "ASCIILevelCreator.hpp"
#include "CellVisitorConcepts.hpp"
#include "Direction.hpp"
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <set>

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
  return SingleMove{Action::ADD, CellState::EMPTY, cell, coord};
}

auto
mk_move(Direction dir, Coord coord, CellState cell) {
  return SingleMoveDirection(dir, Action::ADD, CellState::EMPTY, cell, coord);
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
  EXPECT_FALSE(b.is_initialized());
  EXPECT_EQ(0, b.width());
  EXPECT_EQ(0, b.height());
}

TEST(BasicBoardTest, initialize) {
  BasicBoard b;
  b.reset(5, 6);
  EXPECT_TRUE(b.is_initialized());
  EXPECT_EQ(5, b.height());
  EXPECT_EQ(6, b.width());
}

TEST(BasicBoardTest, cell_accessors) {
  BasicBoard b;
  b.reset(5, 6);

  EXPECT_EQ(CellState::EMPTY, b.get_cell({0, 0}));
  EXPECT_EQ(CellState::EMPTY, b.get_cell({0, 1}));
  EXPECT_EQ(CellState::EMPTY, b.get_cell({1, 2}));
  EXPECT_EQ(CellState::EMPTY, b.get_cell({2, 3}));
  EXPECT_EQ(CellState::EMPTY, b.get_cell({4, 4}));

  b.set_cell({0, 0}, CellState::WALL0);
  b.set_cell({0, 1}, CellState::WALL1);
  b.set_cell({1, 2}, CellState::WALL2);
  b.set_cell({2, 3}, CellState::WALL3);
  b.set_cell({4, 4}, CellState::MARK);

  EXPECT_EQ(CellState::WALL0, b.get_cell({0, 0}));
  EXPECT_EQ(CellState::WALL1, b.get_cell({0, 1}));
  EXPECT_EQ(CellState::WALL2, b.get_cell({1, 2}));
  EXPECT_EQ(CellState::WALL3, b.get_cell({2, 3}));
  EXPECT_EQ(CellState::MARK, b.get_cell({4, 4}));

  EXPECT_TRUE(b.get_opt_cell({0, 0}).has_value());
  EXPECT_TRUE(b.get_opt_cell({0, 1}).has_value());
  EXPECT_TRUE(b.get_opt_cell({1, 2}).has_value());
  EXPECT_TRUE(b.get_opt_cell({2, 3}).has_value());
  EXPECT_TRUE(b.get_opt_cell({4, 4}).has_value());
  EXPECT_EQ(CellState::WALL0, *b.get_opt_cell({0, 0}));
  EXPECT_EQ(CellState::WALL1, *b.get_opt_cell({0, 1}));
  EXPECT_EQ(CellState::WALL2, *b.get_opt_cell({1, 2}));
  EXPECT_EQ(CellState::WALL3, *b.get_opt_cell({2, 3}));
  EXPECT_EQ(CellState::MARK, *b.get_opt_cell({4, 4}));

  auto not_wall1_or_wall2 = [](CellState cell) {
    return cell != CellState::WALL1 && cell != CellState::WALL2;
  };
  b.set_cell_if({0, 0}, CellState::EMPTY, not_wall1_or_wall2);
  b.set_cell_if({0, 1}, CellState::EMPTY, not_wall1_or_wall2);
  b.set_cell_if({1, 2}, CellState::EMPTY, not_wall1_or_wall2);
  b.set_cell_if({2, 3}, CellState::EMPTY, not_wall1_or_wall2);
  b.set_cell_if({4, 4}, CellState::EMPTY, not_wall1_or_wall2);

  EXPECT_EQ(CellState::EMPTY, b.get_cell({0, 0}));
  EXPECT_EQ(CellState::WALL1, b.get_cell({0, 1}));
  EXPECT_EQ(CellState::WALL2, b.get_cell({1, 2}));
  EXPECT_EQ(CellState::EMPTY, b.get_cell({2, 3}));
  EXPECT_EQ(CellState::EMPTY, b.get_cell({4, 4}));
}

TEST(BasicBoardTest, bad_cell_access) {
  BasicBoard b;
  b.reset(5, 6);

  EXPECT_THROW(b.get_cell({-1, 0}), std::range_error);
  EXPECT_THROW(b.get_cell({0, -1}), std::range_error);
  EXPECT_THROW(b.get_cell({0, -1}), std::range_error);
  EXPECT_THROW(b.get_cell({0, 6}), std::range_error);

  EXPECT_EQ(std::nullopt, b.get_opt_cell({-1, 0}));
  EXPECT_EQ(std::nullopt, b.get_opt_cell({0, -1}));
  EXPECT_EQ(std::nullopt, b.get_opt_cell({0, -1}));
  EXPECT_EQ(std::nullopt, b.get_opt_cell({0, 6}));
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
    EXPECT_EQ(EMPTY, board.get_cell({0, 0}));
    EXPECT_EQ(EMPTY, board.get_cell({0, 1}));
    EXPECT_EQ(BULB, board.get_cell({0, 2}));

    // Mid row
    EXPECT_EQ(WALL1, board.get_cell({1, 0}));
    EXPECT_EQ(WALL2, board.get_cell({1, 1}));
    EXPECT_EQ(WALL3, board.get_cell({1, 2}));

    // Bot row
    EXPECT_EQ(WALL0, board.get_cell({2, 0}));
    EXPECT_EQ(MARK, board.get_cell({2, 1}));
    EXPECT_EQ(WALL4, board.get_cell({2, 2}));
  };

  board2 = board1;
  test(board1);
  test(board2);

  EXPECT_EQ(board1, board2);
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
  EXPECT_EQ(9, count);
  EXPECT_EQ(board, board2);
}

TEST(BasicBoardTest, visit_empty) {
  BasicBoard board;
  {
    ASCIILevelCreator creator;
    creator("......0.");
    creator("00.....0");
    creator("0....00.");
    creator("0..00...");
    creator("......0.");
    creator("..0...00");
    creator.finished(&board);
  }

  std::set<Coord> coords;
  board.visit_empty([&](Coord coord, CellState cell) {
    coords.insert(coord);
    ASSERT_EQ(CellState::EMPTY, cell);
    ASSERT_EQ(CellState::EMPTY, board.get_cell(coord));
  });
  ASSERT_EQ(34, coords.size());
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
    return cell != CellState::WALL2 ? KEEP_VISITING : STOP_VISITING;
  });
  EXPECT_EQ(5, count);

  BasicBoard expected;
  {
    ASCIILevelCreator creator;
    creator("..*");
    creator("12.");
    creator("...");
    creator.finished(&expected);
  }
  EXPECT_EQ(expected, board2);
}

TEST(BasicBoardTest, visit_left_row_some) {
  Moves moves;
  auto  b = make_board(); // r1: "12.00");

  b.visit_row_left_of({1, 4}, recorder(moves, CellState::EMPTY));

  // stops at wall to left of cooord
  EXPECT_EQ((std::vector{mk_move({1, 3}, CellState::WALL0)}), moves);

  moves.clear();
  b.visit_row_left_of({1, 3}, recorder(moves, CellState::EMPTY));

  // stops because it sees EMPTY
  EXPECT_EQ((std::vector{mk_move({1, 2}, CellState::EMPTY)}), moves);
}

TEST(BasicBoardTest, visit_directional_left_row_some) {
  DirectionMoves moves;
  auto           b = make_board(); // r1: "12.00");

  b.visit_row_left_of({1, 4}, recorder(moves, CellState::EMPTY));

  // stops at wall to left of cooord
  EXPECT_EQ((std::vector{mk_move(LEFT, {1, 3}, CellState::WALL0)}), moves);

  moves.clear();
  b.visit_row_left_of({1, 3}, recorder(moves, CellState::EMPTY));

  // stops because it sees EMPTY
  EXPECT_EQ((std::vector{mk_move(LEFT, {1, 2}, CellState::EMPTY)}), moves);
}

TEST(BasicBoardTest, visit_left_row_some2) {
  Moves moves;
  auto  b = make_board(); // row0: "..*.."

  b.visit_row_left_of({0, 4}, recorder(moves, CellState::BULB));

  EXPECT_EQ((std::vector{mk_move({0, 3}, CellState::EMPTY),
                         mk_move({0, 2}, CellState::BULB)}),

            moves);
}

TEST(BasicBoardTest, visit_left_row_some3_invalid) {
  Moves moves;
  auto  b = make_board();
  b.visit_row_left_of({1, 0}, recorder(moves, CellState::BULB));
  EXPECT_EQ(Moves{}, moves);

  b.visit_row_left_of({-1, 0}, recorder(moves, CellState::BULB));
  EXPECT_EQ(Moves{}, moves);

  b.visit_row_left_of({0, -4}, recorder(moves, CellState::BULB));
  EXPECT_EQ(Moves{}, moves);

  b.visit_row_left_of({30, 0}, recorder(moves, CellState::BULB));
  EXPECT_EQ(Moves{}, moves);
}

TEST(BasicBoardTest, visit_left_row_all) {
  Moves moves;
  // r2: "...X3"
  make_board().visit_row_left_of({2, 4}, recorder(moves));
  EXPECT_EQ((std::vector{mk_move({2, 3}, CellState::MARK),
                         mk_move({2, 2}, CellState::EMPTY),
                         mk_move({2, 1}, CellState::EMPTY),
                         mk_move({2, 0}, CellState::EMPTY)}),
            moves);
}

TEST(BasicBoardTest, visit_directional_left_row_all) {
  DirectionMoves moves;
  // r2: "...X3"
  make_board().visit_row_left_of({2, 4}, recorder(moves));
  EXPECT_EQ((std::vector{mk_move(LEFT, {2, 3}, CellState::MARK),
                         mk_move(LEFT, {2, 2}, CellState::EMPTY),
                         mk_move(LEFT, {2, 1}, CellState::EMPTY),
                         mk_move(LEFT, {2, 0}, CellState::EMPTY)}),
            moves);
}

TEST(BasicBoardTest, visit_right_row_some) {
  Moves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves, CellState::MARK));
  // NOTE row 2 looks like: "...X3"
  EXPECT_EQ((std::vector{mk_move({2, 1}, CellState::EMPTY),
                         mk_move({2, 2}, CellState::EMPTY),
                         mk_move({2, 3}, CellState::MARK)}),
            moves);
}

TEST(BasicBoardTest, visit_directional_right_row_some) {
  DirectionMoves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves, CellState::MARK));
  // NOTE row 2 looks like: "...X3"
  EXPECT_EQ((std::vector{mk_move(RIGHT, {2, 1}, CellState::EMPTY),
                         mk_move(RIGHT, {2, 2}, CellState::EMPTY),
                         mk_move(RIGHT, {2, 3}, CellState::MARK)}),
            moves);
}

TEST(BasicBoardTest, visit_directional_right_row_all) {
  DirectionMoves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves));
  // NOTE r2 looks like: "...X3"
  EXPECT_EQ((std::vector{mk_move(RIGHT, {2, 1}, CellState::EMPTY),
                         mk_move(RIGHT, {2, 2}, CellState::EMPTY),
                         mk_move(RIGHT, {2, 3}, CellState::MARK),
                         mk_move(RIGHT, {2, 4}, CellState::WALL3)}),
            moves);
}
TEST(BasicBoardTest, visit_right_row_all) {
  Moves moves;
  make_board().visit_row_right_of({2, 0}, recorder(moves));
  // NOTE r2 looks like: "...X3"
  EXPECT_EQ((std::vector{mk_move({2, 1}, CellState::EMPTY),
                         mk_move({2, 2}, CellState::EMPTY),
                         mk_move({2, 3}, CellState::MARK),
                         mk_move({2, 4}, CellState::WALL3)}),
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
  board.visit_col_above({4, 4}, recorder(moves, CellState::WALL0));

  EXPECT_EQ((std::vector{
                mk_move({3, 4}, CellState::BULB),
                mk_move({2, 4}, CellState::ILLUM),
                mk_move({1, 4}, CellState::WALL0),
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
  board.visit_col_above({4, 4}, recorder(moves, CellState::WALL0));

  EXPECT_EQ((std::vector{
                mk_move(UP, {3, 4}, CellState::BULB),
                mk_move(UP, {2, 4}, CellState::ILLUM),
                mk_move(UP, {1, 4}, CellState::WALL0),
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

  EXPECT_EQ((std::vector{
                mk_move({3, 4}, CellState::BULB),
                mk_move({2, 4}, CellState::ILLUM),
                mk_move({1, 4}, CellState::WALL0),
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

  EXPECT_EQ((std::vector{
                mk_move(UP, {3, 4}, CellState::BULB),
                mk_move(UP, {2, 4}, CellState::ILLUM),
                mk_move(UP, {1, 4}, CellState::WALL0),
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
  board.visit_col_below({0, 3}, recorder(moves, CellState::WALL4));

  EXPECT_EQ((std::vector{
                mk_move({1, 3}, CellState::MARK),
                mk_move({2, 3}, CellState::MARK),
                mk_move({3, 3}, CellState::WALL4),
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
  board.visit_col_below({0, 3}, recorder(moves, CellState::WALL4));

  EXPECT_EQ((std::vector{
                mk_move(DOWN, {1, 3}, CellState::MARK),
                mk_move(DOWN, {2, 3}, CellState::MARK),
                mk_move(DOWN, {3, 3}, CellState::WALL4),
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

  EXPECT_EQ((std::vector{
                mk_move({1, 2}, CellState::ILLUM),
                mk_move({2, 2}, CellState::ILLUM),
                mk_move({3, 2}, CellState::ILLUM),
                mk_move({4, 2}, CellState::ILLUM),
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

  EXPECT_EQ((std::vector{
                mk_move(DOWN, {1, 2}, CellState::ILLUM),
                mk_move(DOWN, {2, 2}, CellState::ILLUM),
                mk_move(DOWN, {3, 2}, CellState::ILLUM),
                mk_move(DOWN, {4, 2}, CellState::ILLUM),
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

  // Top LEFT corner
  Moves moves;
  board.visit_adjacent({0, 0}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({1, 0}, CellState::WALL3)),
                          Eq(mk_move({0, 1}, CellState::WALL1))));

  // Top Mid edge
  moves.clear();
  board.visit_adjacent({0, 1}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({0, 0}, CellState::WALL0)),
                          Eq(mk_move({1, 1}, CellState::WALL4)),
                          Eq(mk_move({0, 2}, CellState::WALL2))));

  // Top RIGHT corner
  moves.clear();
  board.visit_adjacent({0, 2}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({0, 1}, CellState::WALL1)),
                          Eq(mk_move({1, 2}, CellState::BULB))));

  // LEFT Mid edge
  moves.clear();
  board.visit_adjacent({1, 0}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({0, 0}, CellState::WALL0)),
                          Eq(mk_move({2, 0}, CellState::MARK)),
                          Eq(mk_move({1, 1}, CellState::WALL4))));

  // Center
  moves.clear();
  board.visit_adjacent({1, 1}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({0, 1}, CellState::WALL1)),
                          Eq(mk_move({1, 0}, CellState::WALL3)),
                          Eq(mk_move({2, 1}, CellState::ILLUM)),
                          Eq(mk_move({1, 2}, CellState::BULB))));

  // RIGHT Mid edge
  moves.clear();
  board.visit_adjacent({1, 2}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({0, 2}, CellState::WALL2)),
                          Eq(mk_move({1, 1}, CellState::WALL4)),
                          Eq(mk_move({2, 2}, CellState::EMPTY))));

  // Bottom LEFT corner
  moves.clear();
  board.visit_adjacent({2, 0}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({1, 0}, CellState::WALL3)),
                          Eq(mk_move({2, 1}, CellState::ILLUM))));

  // Bottom Mid edge
  moves.clear();
  board.visit_adjacent({2, 1}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({1, 1}, CellState::WALL4)),
                          Eq(mk_move({2, 0}, CellState::MARK)),
                          Eq(mk_move({2, 2}, CellState::EMPTY))));

  // Bottom RIGHT corner
  moves.clear();
  board.visit_adjacent({2, 2}, recorder(moves));
  EXPECT_THAT(moves,
              ElementsAre(Eq(mk_move({1, 2}, CellState::BULB)),
                          Eq(mk_move({2, 1}, CellState::ILLUM))));
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
    board.visit_perpendicular({2, 0}, Direction::UP, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::RIGHT, {2, 1}, CellState::ILLUM)),
                    Eq(mk_move(Direction::RIGHT, {2, 2}, CellState::ILLUM)),
                    Eq(mk_move(Direction::RIGHT, {2, 3}, CellState::BULB)),
                    Eq(mk_move(Direction::RIGHT, {2, 4}, CellState::ILLUM))));
  }
  // If moving up from 1,2 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({1, 2}, Direction::UP, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::LEFT, {1, 0}, CellState::MARK)),
                    Eq(mk_move(Direction::LEFT, {1, 1}, CellState::EMPTY)),
                    Eq(mk_move(Direction::RIGHT, {1, 3}, CellState::ILLUM)),
                    Eq(mk_move(Direction::RIGHT, {1, 4}, CellState::WALL0))));
  }

  // If moving down from 2,0 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({2, 0}, Direction::DOWN, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::RIGHT, {2, 1}, CellState::ILLUM)),
                    Eq(mk_move(Direction::RIGHT, {2, 2}, CellState::ILLUM)),
                    Eq(mk_move(Direction::RIGHT, {2, 3}, CellState::BULB)),
                    Eq(mk_move(Direction::RIGHT, {2, 4}, CellState::ILLUM))));
  }
  // If moving down from 1,2 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({1, 2}, Direction::DOWN, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::LEFT, {1, 0}, CellState::MARK)),
                    Eq(mk_move(Direction::LEFT, {1, 1}, CellState::EMPTY)),
                    Eq(mk_move(Direction::RIGHT, {1, 3}, CellState::ILLUM)),
                    Eq(mk_move(Direction::RIGHT, {1, 4}, CellState::WALL0))));
  }

  // If moving right at 2,0 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({2, 0}, Direction::RIGHT, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::UP, {0, 0}, CellState::EMPTY)),
                    Eq(mk_move(Direction::UP, {1, 0}, CellState::MARK)),
                    Eq(mk_move(Direction::DOWN, {3, 0}, CellState::WALL1))));
  }
  // If moving right at 3,4 ...
  {
    DirectionMoves moves;
    board.visit_perpendicular({3, 4}, Direction::RIGHT, recorder(moves));
    EXPECT_THAT(moves,
                UnorderedElementsAre(
                    Eq(mk_move(Direction::UP, {2, 4}, CellState::ILLUM)),
                    Eq(mk_move(Direction::UP, {1, 4}, CellState::WALL0))));
  }
}

} // namespace model::test
