#include "CellState.hpp"
#include <fmt/core.h>
#include <gtest/gtest.h>

namespace model::test {

using enum CellState;

TEST(CellStateTest, to_from_char) {
  ASSERT_EQ(Empty, get_state_from_char(to_char(Empty)));
  ASSERT_EQ(Wall0, get_state_from_char(to_char(Wall0)));
  ASSERT_EQ(Wall1, get_state_from_char(to_char(Wall1)));
  ASSERT_EQ(Wall2, get_state_from_char(to_char(Wall2)));
  ASSERT_EQ(Wall3, get_state_from_char(to_char(Wall3)));
  ASSERT_EQ(Wall4, get_state_from_char(to_char(Wall4)));
  ASSERT_EQ(Bulb, get_state_from_char(to_char(Bulb)));
  ASSERT_EQ(Mark, get_state_from_char(to_char(Mark)));
}

TEST(CellStateTest, to_from_string) {
  ASSERT_EQ(Empty, get_state_from_string(to_string(Empty)));
  ASSERT_EQ(Wall0, get_state_from_string(to_string(Wall0)));
  ASSERT_EQ(Wall1, get_state_from_string(to_string(Wall1)));
  ASSERT_EQ(Wall2, get_state_from_string(to_string(Wall2)));
  ASSERT_EQ(Wall3, get_state_from_string(to_string(Wall3)));
  ASSERT_EQ(Wall4, get_state_from_string(to_string(Wall4)));
  ASSERT_EQ(Bulb, model::get_state_from_string(to_string(Bulb)));
  ASSERT_EQ(Mark, model::get_state_from_string(to_string(Mark)));
}

TEST(CellStateTest, bitwise) {
  CellState stateSet = Empty | Wall0 | Mark;
  EXPECT_EQ(Empty, stateSet & Empty);
  EXPECT_EQ(Mark, stateSet & Mark);
  EXPECT_EQ(Wall0, stateSet & Wall0);
  EXPECT_EQ(CellState{}, stateSet & Wall3);
}

TEST(CellStateTest, is_illumable_test) {
  ASSERT_TRUE(is_illumable(Empty));
  ASSERT_TRUE(is_illumable(Mark));

  ASSERT_FALSE(is_illumable(Wall0));
  ASSERT_FALSE(is_illumable(Wall1));
  ASSERT_FALSE(is_illumable(Wall2));
  ASSERT_FALSE(is_illumable(Wall3));
  ASSERT_FALSE(is_illumable(Wall4));
  ASSERT_FALSE(is_illumable(Bulb));
}

TEST(CellStateTest, num_wall_deps_test) {
  ASSERT_EQ(0, num_wall_deps(Empty));
  ASSERT_EQ(0, num_wall_deps(Mark));
  ASSERT_EQ(0, num_wall_deps(Wall0));
  ASSERT_EQ(1, num_wall_deps(Wall1));
  ASSERT_EQ(2, num_wall_deps(Wall2));
  ASSERT_EQ(3, num_wall_deps(Wall3));
  ASSERT_EQ(4, num_wall_deps(Wall4));
  ASSERT_EQ(0, num_wall_deps(Bulb));
}

TEST(CellStateTest, remove_wall_dep_test) {
  ASSERT_EQ(Empty, remove_wall_dep(Empty));
  ASSERT_EQ(Mark, remove_wall_dep(Mark));
  ASSERT_EQ(Wall0, remove_wall_dep(Wall0));
  ASSERT_EQ(Wall0, remove_wall_dep(Wall1));
  ASSERT_EQ(Wall1, remove_wall_dep(Wall2));
  ASSERT_EQ(Wall2, remove_wall_dep(Wall3));
  ASSERT_EQ(Wall3, remove_wall_dep(Wall4));
  ASSERT_EQ(Bulb, remove_wall_dep(Bulb));
}

TEST(CellStateTest, add_wall_dep_test) {
  ASSERT_EQ(Empty, add_wall_dep(Empty));
  ASSERT_EQ(Mark, add_wall_dep(Mark));
  ASSERT_EQ(Wall1, add_wall_dep(Wall0));
  ASSERT_EQ(Wall2, add_wall_dep(Wall1));
  ASSERT_EQ(Wall3, add_wall_dep(Wall2));
  ASSERT_EQ(Wall4, add_wall_dep(Wall3));
  ASSERT_EQ(Wall4, add_wall_dep(Wall4));
  ASSERT_EQ(Bulb, add_wall_dep(Bulb));
}

TEST(CellStateTest, fmt_default) {
  CellState cell = CellState::Wall3;
  auto      fmt1 = fmt::format("{}", cell);
  EXPECT_EQ("Wall3", fmt1);
}

TEST(CellStateTest, fmt_s) {
  CellState cell = CellState::Wall3;
  auto      fmt2 = fmt::format("{:s}", cell);
  EXPECT_EQ("Wall3", fmt2);
}

TEST(CellStateTest, fmt_c) {
  CellState cell = CellState::Wall3;
  auto      fmt3 = fmt::format("{:c}", cell);
  EXPECT_EQ("3", fmt3);
}

} // namespace model::test
