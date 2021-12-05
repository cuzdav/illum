#include "CellState.hpp"
#include <fmt/core.h>
#include <gtest/gtest.h>
#include <stdexcept>

namespace model::test {

using enum CellState;

TEST(CellStateTest, to_from_char) {
  EXPECT_EQ(Bulb, get_state_from_char(to_char(Bulb)));
  EXPECT_EQ(Empty, get_state_from_char(to_char(Empty)));
  EXPECT_EQ(Illum, get_state_from_char(to_char(Illum)));
  EXPECT_EQ(Mark, get_state_from_char(to_char(Mark)));
  EXPECT_EQ(Wall0, get_state_from_char(to_char(Wall0)));
  EXPECT_EQ(Wall1, get_state_from_char(to_char(Wall1)));
  EXPECT_EQ(Wall2, get_state_from_char(to_char(Wall2)));
  EXPECT_EQ(Wall3, get_state_from_char(to_char(Wall3)));
  EXPECT_EQ(Wall4, get_state_from_char(to_char(Wall4)));
}

TEST(CellStateTest, to_from_string) {
  EXPECT_EQ(Bulb, model::get_state_from_string(to_string(Bulb)));
  EXPECT_EQ(Empty, get_state_from_string(to_string(Empty)));
  EXPECT_EQ(Illum, model::get_state_from_string(to_string(Illum)));
  EXPECT_EQ(Mark, model::get_state_from_string(to_string(Mark)));
  EXPECT_EQ(Wall0, get_state_from_string(to_string(Wall0)));
  EXPECT_EQ(Wall1, get_state_from_string(to_string(Wall1)));
  EXPECT_EQ(Wall2, get_state_from_string(to_string(Wall2)));
  EXPECT_EQ(Wall3, get_state_from_string(to_string(Wall3)));
  EXPECT_EQ(Wall4, get_state_from_string(to_string(Wall4)));
}

TEST(CellStateTest, bitwise) {
  CellState stateSet = Empty | Wall0 | Mark;
  EXPECT_EQ(Empty, stateSet & Empty);
  EXPECT_EQ(Mark, stateSet & Mark);
  EXPECT_EQ(Wall0, stateSet & Wall0);
  EXPECT_EQ(CellState{}, stateSet & Wall3);
}

TEST(CellStateTest, is_illuminable_test) {
  EXPECT_TRUE(is_illuminable(Empty));
  EXPECT_TRUE(is_illuminable(Mark));

  EXPECT_FALSE(is_illuminable(Wall0));
  EXPECT_FALSE(is_illuminable(Wall1));
  EXPECT_FALSE(is_illuminable(Wall2));
  EXPECT_FALSE(is_illuminable(Wall3));
  EXPECT_FALSE(is_illuminable(Wall4));
  EXPECT_FALSE(is_illuminable(Bulb));
}

TEST(CellStateTest, wall_with_deps_test) {
  EXPECT_EQ(Wall0, wall_with_deps(0));
  EXPECT_EQ(Wall1, wall_with_deps(1));
  EXPECT_EQ(Wall2, wall_with_deps(2));
  EXPECT_EQ(Wall3, wall_with_deps(3));
  EXPECT_EQ(Wall4, wall_with_deps(4));

  EXPECT_THROW(wall_with_deps(-1), std::logic_error);
  EXPECT_THROW(wall_with_deps(5), std::logic_error);
}

TEST(CellStateTest, num_wall_deps_test) {
  EXPECT_EQ(0, num_wall_deps(Empty));
  EXPECT_EQ(0, num_wall_deps(Mark));
  EXPECT_EQ(0, num_wall_deps(Wall0));
  EXPECT_EQ(1, num_wall_deps(Wall1));
  EXPECT_EQ(2, num_wall_deps(Wall2));
  EXPECT_EQ(3, num_wall_deps(Wall3));
  EXPECT_EQ(4, num_wall_deps(Wall4));
  EXPECT_EQ(0, num_wall_deps(Bulb));
}

TEST(CellStateTest, remove_wall_dep_test) {
  EXPECT_EQ(Empty, remove_wall_dep(Empty));
  EXPECT_EQ(Mark, remove_wall_dep(Mark));
  EXPECT_EQ(Wall0, remove_wall_dep(Wall0));
  EXPECT_EQ(Wall0, remove_wall_dep(Wall1));
  EXPECT_EQ(Wall1, remove_wall_dep(Wall2));
  EXPECT_EQ(Wall2, remove_wall_dep(Wall3));
  EXPECT_EQ(Wall3, remove_wall_dep(Wall4));
  EXPECT_EQ(Bulb, remove_wall_dep(Bulb));
}

TEST(CellStateTest, add_wall_dep_test) {
  EXPECT_EQ(Empty, add_wall_dep(Empty));
  EXPECT_EQ(Mark, add_wall_dep(Mark));
  EXPECT_EQ(Wall1, add_wall_dep(Wall0));
  EXPECT_EQ(Wall2, add_wall_dep(Wall1));
  EXPECT_EQ(Wall3, add_wall_dep(Wall2));
  EXPECT_EQ(Wall4, add_wall_dep(Wall3));
  EXPECT_EQ(Wall4, add_wall_dep(Wall4));
  EXPECT_EQ(Bulb, add_wall_dep(Bulb));
}

TEST(CellStateTest, is_playable) {
  EXPECT_TRUE(is_playable(Bulb));
  EXPECT_TRUE(is_playable(Mark));
  EXPECT_FALSE(is_playable(Empty));
  EXPECT_FALSE(is_playable(Illum));
  EXPECT_FALSE(is_playable(Wall0));
  EXPECT_FALSE(is_playable(Wall1));
  EXPECT_FALSE(is_playable(Wall2));
  EXPECT_FALSE(is_playable(Wall3));
  EXPECT_FALSE(is_playable(Wall4));
}

TEST(CellStateTest, is_dynamic_entity) {
  EXPECT_TRUE(is_dynamic_entity(Bulb));
  EXPECT_TRUE(is_dynamic_entity(Mark));
  EXPECT_TRUE(is_dynamic_entity(Empty));
  EXPECT_TRUE(is_dynamic_entity(Illum));
  EXPECT_FALSE(is_dynamic_entity(Wall0));
  EXPECT_FALSE(is_dynamic_entity(Wall1));
  EXPECT_FALSE(is_dynamic_entity(Wall2));
  EXPECT_FALSE(is_dynamic_entity(Wall3));
  EXPECT_FALSE(is_dynamic_entity(Wall4));
}

TEST(CellStateTest, is_illuminable) {
  EXPECT_TRUE(is_illuminable(Mark));
  EXPECT_TRUE(is_illuminable(Empty));

  EXPECT_FALSE(is_illuminable(Bulb));
  EXPECT_FALSE(is_illuminable(Illum));
  EXPECT_FALSE(is_illuminable(Wall0));
  EXPECT_FALSE(is_illuminable(Wall1));
  EXPECT_FALSE(is_illuminable(Wall2));
  EXPECT_FALSE(is_illuminable(Wall3));
  EXPECT_FALSE(is_illuminable(Wall4));
}

TEST(CellStateTest, is_translucent) {
  EXPECT_TRUE(is_translucent(Mark));
  EXPECT_TRUE(is_translucent(Empty));
  EXPECT_TRUE(is_translucent(Illum));

  EXPECT_FALSE(is_translucent(Bulb));
  EXPECT_FALSE(is_translucent(Wall0));
  EXPECT_FALSE(is_translucent(Wall1));
  EXPECT_FALSE(is_translucent(Wall2));
  EXPECT_FALSE(is_translucent(Wall3));
  EXPECT_FALSE(is_translucent(Wall4));
}

TEST(CellStateTest, is_empty) {
  EXPECT_TRUE(is_empty(Empty));
  EXPECT_FALSE(is_empty(Mark));
  EXPECT_FALSE(is_empty(Illum));
  EXPECT_FALSE(is_empty(Bulb));
  EXPECT_FALSE(is_empty(Wall0));
  EXPECT_FALSE(is_empty(Wall1));
  EXPECT_FALSE(is_empty(Wall2));
  EXPECT_FALSE(is_empty(Wall3));
  EXPECT_FALSE(is_empty(Wall4));
}

TEST(CellStateTest, is_mark) {
  EXPECT_TRUE(is_mark(Mark));
  EXPECT_FALSE(is_mark(Empty));
  EXPECT_FALSE(is_mark(Illum));
  EXPECT_FALSE(is_mark(Bulb));
  EXPECT_FALSE(is_mark(Wall0));
  EXPECT_FALSE(is_mark(Wall1));
  EXPECT_FALSE(is_mark(Wall2));
  EXPECT_FALSE(is_mark(Wall3));
  EXPECT_FALSE(is_mark(Wall4));
}

TEST(CellStateTest, is_bulb) {
  EXPECT_TRUE(is_bulb(Bulb));
  EXPECT_FALSE(is_bulb(Empty));
  EXPECT_FALSE(is_bulb(Mark));
  EXPECT_FALSE(is_bulb(Illum));
  EXPECT_FALSE(is_bulb(Wall0));
  EXPECT_FALSE(is_bulb(Wall1));
  EXPECT_FALSE(is_bulb(Wall2));
  EXPECT_FALSE(is_bulb(Wall3));
  EXPECT_FALSE(is_bulb(Wall4));
}

TEST(CellStateTest, is_wall) {
  EXPECT_TRUE(is_wall(Wall0));
  EXPECT_TRUE(is_wall(Wall1));
  EXPECT_TRUE(is_wall(Wall2));
  EXPECT_TRUE(is_wall(Wall3));
  EXPECT_TRUE(is_wall(Wall4));
  EXPECT_FALSE(is_wall(Bulb));
  EXPECT_FALSE(is_wall(Empty));
  EXPECT_FALSE(is_wall(Mark));
  EXPECT_FALSE(is_wall(Illum));
}

TEST(CellStateTest, is_wall_with_dep) {
  EXPECT_TRUE(is_wall_with_deps(Wall1));
  EXPECT_TRUE(is_wall_with_deps(Wall2));
  EXPECT_TRUE(is_wall_with_deps(Wall3));
  EXPECT_TRUE(is_wall_with_deps(Wall4));

  EXPECT_FALSE(is_wall_with_deps(Wall0));
  EXPECT_FALSE(is_wall_with_deps(Bulb));
  EXPECT_FALSE(is_wall_with_deps(Empty));
  EXPECT_FALSE(is_wall_with_deps(Mark));
  EXPECT_FALSE(is_wall_with_deps(Illum));
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
