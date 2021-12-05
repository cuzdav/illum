#include "Direction.hpp"
#include "gtest/gtest.h"

namespace model::test {
using enum Direction;
using namespace directiongroups;

TEST(DirectionTest, bitops) {
  EXPECT_EQ(15, +all);
  EXPECT_TRUE(contains_all(all, Up));
  EXPECT_TRUE(contains_all(all, Down));
  EXPECT_TRUE(contains_all(all, Right));
  EXPECT_TRUE(contains_all(all, Left));
  EXPECT_FALSE(contains_all(Up, all));
  EXPECT_FALSE(contains_all(Down, all));
  EXPECT_FALSE(contains_all(Right, all));
  EXPECT_FALSE(contains_all(Left, all));
  EXPECT_EQ(Up | Down, vertical);
  EXPECT_EQ(Left | Right, horizontal);
  EXPECT_EQ(vertical | horizontal, all);
}

TEST(DirectionTest, to_string) {
  EXPECT_EQ("None", to_string(None));
  EXPECT_EQ("Up", to_string(Up));
  EXPECT_EQ("Down", to_string(Down));
  EXPECT_EQ("Left", to_string(Left));
  EXPECT_EQ("Right", to_string(Right));
}

TEST(DirectionTest, thats_a_rotate) {
  EXPECT_EQ(Left, rotate90_left(Up));
  EXPECT_EQ(Down, rotate90_left(Left));
  EXPECT_EQ(Right, rotate90_left(Down));
  EXPECT_EQ(Up, rotate90_left(Right));
  EXPECT_EQ(None, rotate90_left(None));
}

TEST(DirectionTest, flip) {
  EXPECT_EQ(Left, flip(Right));
  EXPECT_EQ(Down, flip(Up));
  EXPECT_EQ(Right, flip(Left));
  EXPECT_EQ(Up, flip(Down));
  EXPECT_EQ(None, flip(None));
}

} // namespace model::test
