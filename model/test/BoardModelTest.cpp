#include "ASCIILevelCreator.hpp"
#include "TestStateChangeHandler.hpp"
#include "gtest/gtest.h"

#include "BoardModel.hpp"

namespace model::test {

using namespace ::testing;
using enum CellState;
using enum Action;
using model::test::TestStateChangeHandler;

TEST(BM, construct) {
  BoardModel model;

  ASSERT_EQ(0, model.height());
  ASSERT_EQ(0, model.width());
  ASSERT_EQ(0, model.num_total_moves());
  ASSERT_FALSE(model.started());
}

TEST(BM, reset_game) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler =
      *dynamic_cast<TestStateChangeHandler const *>(model.get_handler());

  model.reset_game(3, 5);
  ASSERT_EQ(3, model.height());
  ASSERT_EQ(5, model.width());
  ASSERT_EQ(1,
            model.num_total_moves()); // reset_game is the first move (always)
  ASSERT_FALSE(model.started());

  auto const & moves = handler.cur().moves_;
  ASSERT_EQ(1, moves.size());

  ASSERT_EQ(RESETGame, moves[0].action_);
  ASSERT_EQ(3, moves[0].coord_.row_); // height
  ASSERT_EQ(5, moves[0].coord_.col_); // width
}

TEST(BM, cannot_add_pieces_before_resetting_game) {
  BoardModel model;

  // must not add/remove before calling reset()
  ASSERT_THROW(model.add(WALL0, {0, 0}), std::runtime_error);
  ASSERT_THROW(model.remove({0, 0}), std::runtime_error);
}

TEST(BM, reset_game_again) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler =
      *dynamic_cast<TestStateChangeHandler const *>(model.get_handler());

  ASSERT_EQ(0, handler.num_games());

  model.reset_game(3, 5);
  ASSERT_EQ(1, handler.num_games());
  ASSERT_EQ(3, model.height());
  ASSERT_EQ(5, model.width());

  model.add(WALL0, {1, 3});

  auto const & moves = handler.cur().moves_;
  ASSERT_EQ(2, moves.size());

  ASSERT_EQ(ADD, moves[1].action_);
  ASSERT_EQ(WALL0, moves[1].to_);
  ASSERT_EQ(1, moves[1].coord_.row_);
  ASSERT_EQ(3, moves[1].coord_.col_);

  model.reset_game(6, 7);
  ASSERT_EQ(2, handler.num_games());

  ASSERT_EQ(6, model.height());
  ASSERT_EQ(7, model.width());
}

TEST(BM, start_game) {
  BoardModel model;

  ASCIILevelCreator creator;
  creator("00000");
  creator("0..10");
  creator("00000");
  creator.finished(&model);

  ASSERT_EQ(3, model.height());
  ASSERT_EQ(5, model.width());

  ASSERT_EQ(WALL0, model.get_cell({0, 0}));
  ASSERT_EQ(WALL0, model.get_cell({0, 1}));
  ASSERT_EQ(WALL0, model.get_cell({0, 2}));
  ASSERT_EQ(WALL0, model.get_cell({0, 3}));
  ASSERT_EQ(WALL0, model.get_cell({0, 4}));
  ASSERT_EQ(WALL0, model.get_cell({1, 0}));
  ASSERT_EQ(WALL0, model.get_cell({1, 4}));
  ASSERT_EQ(WALL0, model.get_cell({2, 0}));
  ASSERT_EQ(WALL0, model.get_cell({2, 1}));
  ASSERT_EQ(WALL0, model.get_cell({2, 2}));
  ASSERT_EQ(WALL0, model.get_cell({2, 3}));
  ASSERT_EQ(WALL0, model.get_cell({2, 4}));

  ASSERT_EQ(EMPTY, model.get_cell({1, 1}));
  ASSERT_EQ(EMPTY, model.get_cell({1, 2}));

  ASSERT_EQ(WALL1, model.get_cell({1, 3}));
}

TEST(BM, reset_from_board) {
  BoardModel        model;
  ASCIILevelCreator creator;
  creator("00000");
  creator("0..10");
  creator("00000");
  creator.finished(&model);

  ASSERT_EQ(3, model.height());
  ASSERT_EQ(5, model.width());

  BasicBoard        board;
  BoardModel        model2;
  ASCIILevelCreator creator2;
  creator2("1...");
  creator2(".2..");
  creator2("..3.");
  creator2.finished(&board, ASCIILevelCreator::ResetPolicy::DONT_RESET);
  creator2.finished(&model2);

  ASSERT_EQ(3, board.height());
  ASSERT_EQ(4, board.width());

  ASSERT_EQ(3, model2.height());
  ASSERT_EQ(4, model2.width());

  model.reset_game(board);
  // (test) ASCIILevelCreator "starts" game automatically so we must too, before
  // we can compare them.
  model.start_game();

  ASSERT_EQ(3, model.height());
  ASSERT_EQ(4, model.width());

  ASSERT_EQ(model, model2);
}

TEST(BM, visit_board) {
  BoardModel        model;
  ASCIILevelCreator creator;
  creator("00000");
  creator("0..10");
  creator("00000");
  creator.finished(&model);

  int count = 0;
  model.visit_board([&](Coord coord, CellState cell) {
    ++count;
    ASSERT_EQ(model.get_cell(coord), cell);
  });
  ASSERT_EQ(model.width() * model.height(), count);
}

TEST(BM, undo) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler =
      *dynamic_cast<TestStateChangeHandler const *>(model.get_handler());

  handler.set_trace(true);

  ASCIILevelCreator creator;
  creator("0.00.");
  creator("0..1.");
  creator("1....");
  creator.finished(&model, ASCIILevelCreator::StartPolicy::DONT_CALL_START);

  ASSERT_FALSE(model.undo()); // not started
  model.start_game();
  ASSERT_FALSE(model.undo());            // no user-played
  ASSERT_EQ(8, model.num_total_moves()); // counting reset,start,and setup walls
  ASSERT_EQ(0, model.num_played_moves()); // none yet

  // Move 1 (not a good move, maybe a user mistake)
  model.add(CellState::BULB, {1, 1});
  ASSERT_EQ(CellState::EMPTY, handler.last_move().from_);
  ASSERT_EQ(CellState::BULB, handler.last_move().to_);
  ASSERT_EQ(Coord(1, 1), handler.last_move().coord_);
  ASSERT_EQ(1, model.num_played_moves());

  // Move 2 (remove move 1)
  model.remove({1, 1});
  ASSERT_EQ(CellState::BULB, handler.last_move().from_);
  ASSERT_EQ(CellState::EMPTY, handler.cur().moves_.back().to_);
  ASSERT_EQ(Coord(1, 1), handler.cur().moves_.back().coord_);
  ASSERT_EQ(2, model.num_played_moves());

  // Move 3 (a good move, maybe prev mistake was a typo)
  model.add(CellState::BULB, {2, 1});
  ASSERT_EQ(CellState::EMPTY, handler.last_move().from_);
  ASSERT_EQ(CellState::BULB, handler.last_move().to_);
  ASSERT_EQ(Coord(2, 1), handler.last_move().coord_);
  ASSERT_EQ(3, model.num_played_moves());

  ASSERT_TRUE(model.undo()); // undo move 3
  // undoing an add looks like a remove to the callback
  ASSERT_EQ(Action::REMOVE, handler.last_move().action_);
  ASSERT_EQ(CellState::BULB, handler.last_move().from_);
  ASSERT_EQ(CellState::EMPTY, handler.last_move().to_);
  ASSERT_EQ(Coord(2, 1), handler.last_move().coord_);
  ASSERT_EQ(2, model.num_played_moves());

  ASSERT_TRUE(model.undo()); // undo move 2
  // undoing a remove looks like an add to the callback
  ASSERT_EQ(Action::ADD, handler.last_move().action_);
  ASSERT_EQ(CellState::EMPTY, handler.last_move().from_);
  ASSERT_EQ(CellState::BULB, handler.last_move().to_);
  ASSERT_EQ(Coord(1, 1), handler.last_move().coord_);
  ASSERT_EQ(1, model.num_played_moves());

  ASSERT_TRUE(model.undo()); // undo move 1
  ASSERT_EQ(Action::REMOVE, handler.last_move().action_);
  ASSERT_EQ(CellState::BULB, handler.last_move().from_);
  ASSERT_EQ(CellState::EMPTY, handler.last_move().to_);

  ASSERT_EQ(Coord(1, 1), handler.last_move().coord_);

  ASSERT_EQ(0, model.num_played_moves());
}

} // namespace model::test
