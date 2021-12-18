#include "CellState.hpp"
#include <fmt/core.h>
#include <gtest/gtest.h>
#include <stdexcept>

namespace model::test {

using enum CellState;

TEST(CellStateTest, to_from_char) {
  EXPECT_EQ(BULB, get_state_from_char(to_char(BULB)));
  EXPECT_EQ(EMPTY, get_state_from_char(to_char(EMPTY)));
  EXPECT_EQ(ILLUM, get_state_from_char(to_char(ILLUM)));
  EXPECT_EQ(MARK, get_state_from_char(to_char(MARK)));
  EXPECT_EQ(WALL0, get_state_from_char(to_char(WALL0)));
  EXPECT_EQ(WALL1, get_state_from_char(to_char(WALL1)));
  EXPECT_EQ(WALL2, get_state_from_char(to_char(WALL2)));
  EXPECT_EQ(WALL3, get_state_from_char(to_char(WALL3)));
  EXPECT_EQ(WALL4, get_state_from_char(to_char(WALL4)));
}

TEST(CellStateTest, to_from_string) {
  EXPECT_EQ(BULB, model::get_state_from_string(to_string(BULB)));
  EXPECT_EQ(EMPTY, get_state_from_string(to_string(EMPTY)));
  EXPECT_EQ(ILLUM, model::get_state_from_string(to_string(ILLUM)));
  EXPECT_EQ(MARK, model::get_state_from_string(to_string(MARK)));
  EXPECT_EQ(WALL0, get_state_from_string(to_string(WALL0)));
  EXPECT_EQ(WALL1, get_state_from_string(to_string(WALL1)));
  EXPECT_EQ(WALL2, get_state_from_string(to_string(WALL2)));
  EXPECT_EQ(WALL3, get_state_from_string(to_string(WALL3)));
  EXPECT_EQ(WALL4, get_state_from_string(to_string(WALL4)));
}

TEST(CellStateTest, bitwise) {
  CellState stateSet = EMPTY | WALL0 | MARK;
  EXPECT_EQ(EMPTY, stateSet & EMPTY);
  EXPECT_EQ(MARK, stateSet & MARK);
  EXPECT_EQ(WALL0, stateSet & WALL0);
  EXPECT_EQ(CellState{}, stateSet & WALL3);
}

TEST(CellStateTest, is_illuminable_test) {
  EXPECT_TRUE(is_illuminable(EMPTY));
  EXPECT_TRUE(is_illuminable(MARK));

  EXPECT_FALSE(is_illuminable(WALL0));
  EXPECT_FALSE(is_illuminable(WALL1));
  EXPECT_FALSE(is_illuminable(WALL2));
  EXPECT_FALSE(is_illuminable(WALL3));
  EXPECT_FALSE(is_illuminable(WALL4));
  EXPECT_FALSE(is_illuminable(BULB));
}

TEST(CellStateTest, wall_with_deps_test) {
  EXPECT_EQ(WALL0, wall_with_deps(0));
  EXPECT_EQ(WALL1, wall_with_deps(1));
  EXPECT_EQ(WALL2, wall_with_deps(2));
  EXPECT_EQ(WALL3, wall_with_deps(3));
  EXPECT_EQ(WALL4, wall_with_deps(4));

  EXPECT_THROW(wall_with_deps(-1), std::logic_error);
  EXPECT_THROW(wall_with_deps(5), std::logic_error);
}

TEST(CellStateTest, num_wall_deps_test) {
  EXPECT_EQ(0, num_wall_deps(EMPTY));
  EXPECT_EQ(0, num_wall_deps(MARK));
  EXPECT_EQ(0, num_wall_deps(WALL0));
  EXPECT_EQ(1, num_wall_deps(WALL1));
  EXPECT_EQ(2, num_wall_deps(WALL2));
  EXPECT_EQ(3, num_wall_deps(WALL3));
  EXPECT_EQ(4, num_wall_deps(WALL4));
  EXPECT_EQ(0, num_wall_deps(BULB));
}

TEST(CellStateTest, remove_wall_dep_test) {
  EXPECT_EQ(EMPTY, remove_wall_dep(EMPTY));
  EXPECT_EQ(MARK, remove_wall_dep(MARK));
  EXPECT_EQ(WALL0, remove_wall_dep(WALL0));
  EXPECT_EQ(WALL0, remove_wall_dep(WALL1));
  EXPECT_EQ(WALL1, remove_wall_dep(WALL2));
  EXPECT_EQ(WALL2, remove_wall_dep(WALL3));
  EXPECT_EQ(WALL3, remove_wall_dep(WALL4));
  EXPECT_EQ(BULB, remove_wall_dep(BULB));
}

TEST(CellStateTest, add_wall_dep_test) {
  EXPECT_EQ(EMPTY, add_wall_dep(EMPTY));
  EXPECT_EQ(MARK, add_wall_dep(MARK));
  EXPECT_EQ(WALL1, add_wall_dep(WALL0));
  EXPECT_EQ(WALL2, add_wall_dep(WALL1));
  EXPECT_EQ(WALL3, add_wall_dep(WALL2));
  EXPECT_EQ(WALL4, add_wall_dep(WALL3));
  EXPECT_EQ(WALL4, add_wall_dep(WALL4));
  EXPECT_EQ(BULB, add_wall_dep(BULB));
}

TEST(CellStateTest, is_playable) {
  EXPECT_TRUE(is_playable(BULB));
  EXPECT_TRUE(is_playable(MARK));
  EXPECT_FALSE(is_playable(EMPTY));
  EXPECT_FALSE(is_playable(ILLUM));
  EXPECT_FALSE(is_playable(WALL0));
  EXPECT_FALSE(is_playable(WALL1));
  EXPECT_FALSE(is_playable(WALL2));
  EXPECT_FALSE(is_playable(WALL3));
  EXPECT_FALSE(is_playable(WALL4));
}

TEST(CellStateTest, is_dynamic_entity) {
  EXPECT_TRUE(is_dynamic_entity(BULB));
  EXPECT_TRUE(is_dynamic_entity(MARK));
  EXPECT_TRUE(is_dynamic_entity(EMPTY));
  EXPECT_TRUE(is_dynamic_entity(ILLUM));
  EXPECT_FALSE(is_dynamic_entity(WALL0));
  EXPECT_FALSE(is_dynamic_entity(WALL1));
  EXPECT_FALSE(is_dynamic_entity(WALL2));
  EXPECT_FALSE(is_dynamic_entity(WALL3));
  EXPECT_FALSE(is_dynamic_entity(WALL4));
}

TEST(CellStateTest, is_illuminable) {
  EXPECT_TRUE(is_illuminable(MARK));
  EXPECT_TRUE(is_illuminable(EMPTY));

  EXPECT_FALSE(is_illuminable(BULB));
  EXPECT_FALSE(is_illuminable(ILLUM));
  EXPECT_FALSE(is_illuminable(WALL0));
  EXPECT_FALSE(is_illuminable(WALL1));
  EXPECT_FALSE(is_illuminable(WALL2));
  EXPECT_FALSE(is_illuminable(WALL3));
  EXPECT_FALSE(is_illuminable(WALL4));
}

TEST(CellStateTest, is_translucent) {
  EXPECT_TRUE(is_translucent(MARK));
  EXPECT_TRUE(is_translucent(EMPTY));
  EXPECT_TRUE(is_translucent(ILLUM));

  EXPECT_FALSE(is_translucent(BULB));
  EXPECT_FALSE(is_translucent(WALL0));
  EXPECT_FALSE(is_translucent(WALL1));
  EXPECT_FALSE(is_translucent(WALL2));
  EXPECT_FALSE(is_translucent(WALL3));
  EXPECT_FALSE(is_translucent(WALL4));
}

TEST(CellStateTest, is_empty) {
  EXPECT_TRUE(is_empty(EMPTY));
  EXPECT_FALSE(is_empty(MARK));
  EXPECT_FALSE(is_empty(ILLUM));
  EXPECT_FALSE(is_empty(BULB));
  EXPECT_FALSE(is_empty(WALL0));
  EXPECT_FALSE(is_empty(WALL1));
  EXPECT_FALSE(is_empty(WALL2));
  EXPECT_FALSE(is_empty(WALL3));
  EXPECT_FALSE(is_empty(WALL4));
}

TEST(CellStateTest, is_mark) {
  EXPECT_TRUE(is_mark(MARK));
  EXPECT_FALSE(is_mark(EMPTY));
  EXPECT_FALSE(is_mark(ILLUM));
  EXPECT_FALSE(is_mark(BULB));
  EXPECT_FALSE(is_mark(WALL0));
  EXPECT_FALSE(is_mark(WALL1));
  EXPECT_FALSE(is_mark(WALL2));
  EXPECT_FALSE(is_mark(WALL3));
  EXPECT_FALSE(is_mark(WALL4));
}

TEST(CellStateTest, is_bulb) {
  EXPECT_TRUE(is_bulb(BULB));
  EXPECT_FALSE(is_bulb(EMPTY));
  EXPECT_FALSE(is_bulb(MARK));
  EXPECT_FALSE(is_bulb(ILLUM));
  EXPECT_FALSE(is_bulb(WALL0));
  EXPECT_FALSE(is_bulb(WALL1));
  EXPECT_FALSE(is_bulb(WALL2));
  EXPECT_FALSE(is_bulb(WALL3));
  EXPECT_FALSE(is_bulb(WALL4));
}

TEST(CellStateTest, is_wall) {
  EXPECT_TRUE(is_wall(WALL0));
  EXPECT_TRUE(is_wall(WALL1));
  EXPECT_TRUE(is_wall(WALL2));
  EXPECT_TRUE(is_wall(WALL3));
  EXPECT_TRUE(is_wall(WALL4));
  EXPECT_FALSE(is_wall(BULB));
  EXPECT_FALSE(is_wall(EMPTY));
  EXPECT_FALSE(is_wall(MARK));
  EXPECT_FALSE(is_wall(ILLUM));
}

TEST(CellStateTest, is_wall_with_dep) {
  EXPECT_TRUE(is_wall_with_deps(WALL1));
  EXPECT_TRUE(is_wall_with_deps(WALL2));
  EXPECT_TRUE(is_wall_with_deps(WALL3));
  EXPECT_TRUE(is_wall_with_deps(WALL4));

  EXPECT_FALSE(is_wall_with_deps(WALL0));
  EXPECT_FALSE(is_wall_with_deps(BULB));
  EXPECT_FALSE(is_wall_with_deps(EMPTY));
  EXPECT_FALSE(is_wall_with_deps(MARK));
  EXPECT_FALSE(is_wall_with_deps(ILLUM));
}

TEST(CellStateTest, fmt_default) {
  CellState cell = CellState::WALL3;
  auto      fmt1 = fmt::format("{}", cell);
  EXPECT_EQ("WALL3", fmt1);
}

TEST(CellStateTest, fmt_s) {
  CellState cell = CellState::WALL3;
  auto      fmt2 = fmt::format("{:s}", cell);
  EXPECT_EQ("WALL3", fmt2);
}

TEST(CellStateTest, fmt_c) {
  CellState cell = CellState::WALL3;
  auto      fmt3 = fmt::format("{:c}", cell);
  EXPECT_EQ("3", fmt3);
}

} // namespace model::test
