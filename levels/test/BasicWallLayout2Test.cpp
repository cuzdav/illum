#include "BasicWallLayout2.hpp"
#include "solver/Solver.hpp"
#include <iostream>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

namespace levels::test {

using namespace ::testing;

TEST(BasicWallLayout2Test, create_simple_8x8) {
  std::mt19937    twister;
  BasicWallLayout2 layout;

  auto result = layout.create(twister, 8, 8);
  std::cout << "EXITING\n" << std::endl;
}


#if 0

TEST(BasicWallLayoutTest, create_single_8x8_regression_5509) {
  std::mt19937    twister;
  BasicWallLayout layout;

  // This seed was found to cause generation to fail. Now it's a regression
  // test.
  twister.seed(5509);
  auto model    = layout.create(twister, 8, 8);
  auto solution = solver::solve(model.get_underlying_board());
  EXPECT_THAT(solution.is_solved(), Eq(true));
}

TEST(BasicWallLayoutTest, create_single_8x8_regression_80549) {
  std::mt19937    twister;
  BasicWallLayout layout;

  // This seed was found to cause generation to fail. Now it's a regression
  // test.
  twister.seed(80549);
  auto model    = layout.create(twister, 8, 8);
  auto solution = solver::solve(model.get_underlying_board());
  EXPECT_THAT(solution.is_solved(), Eq(true));
}

TEST(BasicWallLayoutTest, create1K_16x16) {
  std::mt19937    twister;
  BasicWallLayout layout;

  for (int i = 0; i < 1000; ++i) {

    try {
      auto model = layout.create(twister, 16, 16);
      //    fmt::print("Unplayed board: {}\n", model.get_underlying_board());
      auto solution = solver::solve(model.get_underlying_board());
      EXPECT_THAT(solution.is_solved(), Eq(true));
    }
    catch (std::runtime_error & e) {
      std::cerr << "Failure with i = " << i << "\n";
      throw;
    }
  }
}

#endif 

} // namespace levels::test
