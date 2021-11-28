#include "Coord.hpp"
#include <gtest/gtest.h>

namespace model::test {

TEST(CoordTest, is_in_range) {
  Coord coord{1, 5};

  ASSERT_TRUE(coord.in_range(2, 6));
  ASSERT_FALSE(coord.in_range(1, 6));
  ASSERT_FALSE(coord.in_range(2, 5));
}

TEST(CoordTest, addition) {
  Coord c1{4, 5};
  Coord c2{-1, +1};

  ASSERT_EQ(Coord(3, 6), c1 + c2);
}

TEST(CoordTest, subtraction) {
  Coord c1{4, 5};
  Coord c2{-1, +1};

  ASSERT_EQ(Coord(5, 4), c1 - c2);
}

} // namespace model::test
