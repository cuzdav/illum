#include "BasicWallLayout.hpp"
#include "solver/Solver.hpp"
#include <iostream>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

namespace levels::test {

using namespace ::testing;

TEST(BasicWallLayoutTest, create10K_8x8) {
  std::mt19937    twister;
  BasicWallLayout layout;

  for (int i = 0; i < 10000; ++i) {

    auto model = layout.create(twister, 8, 8);
    //    fmt::print("Unplayed board: {}\n", model.get_underlying_board());

    auto solution = solver::solve(model.get_underlying_board());
    EXPECT_THAT(solution.is_solved(), Eq(true));
  }
}

TEST(BasicWallLayoutTest, create1K_16x16) {
  std::mt19937    twister;
  BasicWallLayout layout;

  for (int i = 0; i < 1000; ++i) {

    auto model = layout.create(twister, 16, 16);
    fmt::print("Unplayed board: {}\n", model.get_underlying_board());

    auto solution = solver::solve(model.get_underlying_board());
    EXPECT_THAT(solution.is_solved(), Eq(true));
  }
}

} // namespace levels::test
