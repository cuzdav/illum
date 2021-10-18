#include "CellState.hpp"
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

} // namespace model::test
