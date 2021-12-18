#include "AnalysisBoard.hpp"
#include "ASCIILevelCreator.hpp"
#include "BasicBoard.hpp"
#include "Direction.hpp"
#include "gtest/gtest.h"

namespace solver::test {

TEST(AnalysisBoardTest, no_adjusting_stack_while_visiting) {
  model::test::ASCIILevelCreator creator;
  creator("...");
  creator(".3.");
  creator("...");
  model::BasicBoard board;
  creator.finished(&board);
  AnalysisBoard aboard(board);

  EXPECT_EQ(0, aboard.get_visit_depth());

  aboard.clone_position();

  // visit_board
  {
    aboard.visit_board([&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_board_if
  {
    aboard.visit_board_if(
        [&](auto, auto) {
          ASSERT_THROW(aboard.clone_position(), std::runtime_error);
          ASSERT_THROW(aboard.pop(), std::runtime_error);
        },
        [](auto, auto) { return true; });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_adjacent
  {
    aboard.visit_adjacent({0, 0}, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_adj_flank
  {
    aboard.visit_adj_flank({0, 0}, model::Direction::DOWN, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_empty
  {
    aboard.visit_empty([&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_row_left_of
  {
    aboard.visit_row_left_of({0, 0}, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_row_right_of
  {
    aboard.visit_row_right_of({0, 0}, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_col_above
  {
    aboard.visit_col_above({0, 0}, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_col_below
  {
    aboard.visit_col_below({0, 0}, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_perpendicular
  {
    aboard.visit_perpendicular({0, 0}, model::Direction::DOWN, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }

  // visit_rows_cols_outward
  {
    aboard.visit_rows_cols_outward({0, 0}, [&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    aboard.clone_position();
    aboard.pop();
  }
}

TEST(AnalysisBoardTest, no_adjusting_stack_while_nested_visiting) {
  model::test::ASCIILevelCreator creator;
  creator("...");
  creator(".3.");
  creator("...");
  model::BasicBoard board;
  creator.finished(&board);
  AnalysisBoard aboard(board);

  EXPECT_EQ(0, aboard.get_visit_depth());

  aboard.clone_position();

  aboard.visit_board([&](auto, auto) {
    ASSERT_THROW(aboard.clone_position(), std::runtime_error);
    ASSERT_THROW(aboard.pop(), std::runtime_error);
    aboard.visit_board([&](auto, auto) {
      ASSERT_THROW(aboard.clone_position(), std::runtime_error);
      ASSERT_THROW(aboard.pop(), std::runtime_error);
    });
    // after returning from nested visit, we still cannot modify stack...
    ASSERT_THROW(aboard.clone_position(), std::runtime_error);
    ASSERT_THROW(aboard.pop(), std::runtime_error);
  });
  aboard.clone_position();
  aboard.pop();
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
