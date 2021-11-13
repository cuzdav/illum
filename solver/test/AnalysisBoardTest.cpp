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

} // namespace solver::test
