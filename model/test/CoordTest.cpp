#include "Coord.hpp"
#include <gtest/gtest.h>

namespace model::test {

TEST(CoordTest, is_in_range) {
  Coord coord{1, 5};

  ASSERT_TRUE(coord.in_range(2, 6));
  ASSERT_FALSE(coord.in_range(1, 6));
  ASSERT_FALSE(coord.in_range(2, 5));
}

} // namespace model::test
