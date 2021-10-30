#include "LevelCreator.hpp"
#include "TestStateChangeHandler.hpp"
#include "gtest/gtest.h"

#include "BoardModel.hpp"

namespace model::test {

using namespace ::testing;
using enum CellState;
using enum Action;
using model::test::TestStateChangeHandler;

TEST(BM, construct) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler = *model.get_handler();

  ASSERT_EQ(0, model.height());
  ASSERT_EQ(0, model.width());
  ASSERT_EQ(0, model.num_moves());
  ASSERT_FALSE(model.started());
}

TEST(BM, reset_game) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler =
      *dynamic_cast<TestStateChangeHandler const *>(model.get_handler());

  model.reset_game(3, 5);
  ASSERT_EQ(3, model.height());
  ASSERT_EQ(5, model.width());
  ASSERT_EQ(1, model.num_moves()); // reset_game is the first move (always)
  ASSERT_FALSE(model.started());

  auto const & moves = handler.cur().moves_;
  ASSERT_EQ(1, moves.size());

  ASSERT_EQ(ResetGame, moves[0].action_);
  ASSERT_EQ(3, moves[0].row_); // height
  ASSERT_EQ(5, moves[0].col_); // width
}

TEST(BM, cannot_add_pieces_before_resetting_game) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler = *model.get_handler();

  // must not add/remove before calling reset()
  ASSERT_THROW(model.add(Wall0, 0, 0), std::runtime_error);
  ASSERT_THROW(model.remove(0, 0), std::runtime_error);
}

TEST(BM, reset_game_again) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler =
      *dynamic_cast<TestStateChangeHandler const *>(model.get_handler());

  ASSERT_EQ(0, handler.num_games());

  model.reset_game(3, 5);
  ASSERT_EQ(1, handler.num_games());

  model.add(Wall0, 1, 3);

  auto const & moves = handler.cur().moves_;
  ASSERT_EQ(2, moves.size());

  ASSERT_EQ(Add, moves[1].action_);
  ASSERT_EQ(Wall0, moves[1].state_);
  ASSERT_EQ(1, moves[1].row_);
  ASSERT_EQ(3, moves[1].col_);

  model.reset_game(6, 7);
  ASSERT_EQ(2, handler.num_games());

  ASSERT_EQ(6, model.height());
  ASSERT_EQ(7, model.width());
}

TEST(BM, start_game) {
  BoardModel model(std::make_unique<TestStateChangeHandler>());

  LevelCreator creator;
  creator("00000");
  creator("0..10");
  creator("00000");
  creator.finished(&model);

  ASSERT_EQ(3, model.height());
  ASSERT_EQ(5, model.width());

  ASSERT_EQ(Wall0, model.get_cell(0, 0));
  ASSERT_EQ(Wall0, model.get_cell(0, 1));
  ASSERT_EQ(Wall0, model.get_cell(0, 2));
  ASSERT_EQ(Wall0, model.get_cell(0, 3));
  ASSERT_EQ(Wall0, model.get_cell(0, 4));
  ASSERT_EQ(Wall0, model.get_cell(1, 0));
  ASSERT_EQ(Wall0, model.get_cell(1, 4));
  ASSERT_EQ(Wall0, model.get_cell(2, 0));
  ASSERT_EQ(Wall0, model.get_cell(2, 1));
  ASSERT_EQ(Wall0, model.get_cell(2, 2));
  ASSERT_EQ(Wall0, model.get_cell(2, 3));
  ASSERT_EQ(Wall0, model.get_cell(2, 4));

  ASSERT_EQ(Empty, model.get_cell(1, 1));
  ASSERT_EQ(Empty, model.get_cell(1, 2));

  ASSERT_EQ(Wall1, model.get_cell(1, 3));
}

TEST(BM, reset_from_board) {
  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  LevelCreator creator;
  creator("00000");
  creator("0..10");
  creator("00000");
  creator.finished(&model);

  ASSERT_EQ(3, model.height());
  ASSERT_EQ(5, model.width());

  BasicBoard   board;
  BoardModel   model2(std::make_unique<TestStateChangeHandler>());
  LevelCreator creator2;
  creator2("1**.");
  creator2("*2*.");
  creator2("**3.");
  creator2.finished(&board);
  creator2.finished(&model2);

  ASSERT_EQ(3, board.height());
  ASSERT_EQ(4, board.width());

  ASSERT_EQ(3, model2.height());
  ASSERT_EQ(4, model2.width());

  model.reset_game(board);
  // (test) LevelCreator "starts" game automatically so we must too, before
  // we can compare them.
  model.start_game();

  ASSERT_EQ(3, model.height());
  ASSERT_EQ(4, model.width());

  ASSERT_EQ(model, model2);
}

} // namespace model::test
