#include "Action.hpp"
#include "gtest/gtest.h"

namespace model::test {

using namespace std::literals;
using enum Action;

TEST(ActionTest, action_to_from_string_test) {
  ASSERT_EQ(Add, get_action_from_string(to_string(Add)));
  ASSERT_EQ(Remove, get_action_from_string(to_string(Remove)));
  ASSERT_EQ(ResetGame, get_action_from_string(to_string(ResetGame)));
  ASSERT_EQ(StartGame, get_action_from_string(to_string(StartGame)));
}

TEST(ActionTest, to_from_char) {
  ASSERT_EQ(Add, get_action_from_char(to_char(Add)));
  ASSERT_EQ(Remove, get_action_from_char(to_char(Remove)));
  ASSERT_EQ(ResetGame, get_action_from_char(to_char(ResetGame)));
  ASSERT_EQ(StartGame, get_action_from_char(to_char(StartGame)));
}

} // namespace model::test
