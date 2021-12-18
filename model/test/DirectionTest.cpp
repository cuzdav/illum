#include "Direction.hpp"
#include "gtest/gtest.h"

namespace model::test {
using enum Direction;
using namespace directiongroups;

TEST(DirectionTest, bitops) {
  EXPECT_EQ(15, +all);
  EXPECT_TRUE(contains_all(all, UP));
  EXPECT_TRUE(contains_all(all, DOWN));
  EXPECT_TRUE(contains_all(all, RIGHT));
  EXPECT_TRUE(contains_all(all, LEFT));
  EXPECT_FALSE(contains_all(UP, all));
  EXPECT_FALSE(contains_all(DOWN, all));
  EXPECT_FALSE(contains_all(RIGHT, all));
  EXPECT_FALSE(contains_all(LEFT, all));
  EXPECT_EQ(UP | DOWN, vertical);
  EXPECT_EQ(LEFT | RIGHT, horizontal);
  EXPECT_EQ(vertical | horizontal, all);
}

TEST(DirectionTest, to_string) {
  EXPECT_EQ("NONE", to_string(NONE));
  EXPECT_EQ("UP", to_string(UP));
  EXPECT_EQ("DOWN", to_string(DOWN));
  EXPECT_EQ("LEFT", to_string(LEFT));
  EXPECT_EQ("RIGHT", to_string(RIGHT));
}

TEST(DirectionTest, thats_a_rotate) {
  EXPECT_EQ(LEFT, rotate90_left(UP));
  EXPECT_EQ(DOWN, rotate90_left(LEFT));
  EXPECT_EQ(RIGHT, rotate90_left(DOWN));
  EXPECT_EQ(UP, rotate90_left(RIGHT));
  EXPECT_EQ(NONE, rotate90_left(NONE));
}

TEST(DirectionTest, flip) {
  EXPECT_EQ(LEFT, flip(RIGHT));
  EXPECT_EQ(DOWN, flip(UP));
  EXPECT_EQ(RIGHT, flip(LEFT));
  EXPECT_EQ(UP, flip(DOWN));
  EXPECT_EQ(NONE, flip(NONE));
}

} // namespace model::test
