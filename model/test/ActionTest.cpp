#include "Action.hpp"
#include "gtest/gtest.h"

namespace model::test {

using namespace std::literals;
using enum Action;

TEST(ActionTest, action_to_from_string_test) {
  EXPECT_EQ(Add, get_action_from_string(to_string(Add)));
  EXPECT_EQ(Remove, get_action_from_string(to_string(Remove)));
  EXPECT_EQ(ResetGame, get_action_from_string(to_string(ResetGame)));
  EXPECT_EQ(StartGame, get_action_from_string(to_string(StartGame)));
}

TEST(ActionTest, to_from_char) {
  EXPECT_EQ(Add, get_action_from_char(to_char(Add)));
  EXPECT_EQ(Remove, get_action_from_char(to_char(Remove)));
  EXPECT_EQ(ResetGame, get_action_from_char(to_char(ResetGame)));
  EXPECT_EQ(StartGame, get_action_from_char(to_char(StartGame)));
}

TEST(ActionTest, Format_char) {
  static_assert(enumutils::Stringable<Action>);
  static_assert(enumutils::Charable<Action>);

  EXPECT_EQ(to_string(Add), fmt::format("{:s}", Add));
  EXPECT_EQ(to_string(Add), fmt::format("{}", Add));

  EXPECT_EQ(std::string(1, to_char(Add)), fmt::format("{:c}", Add));

  EXPECT_EQ(std::to_string(+Add), fmt::format("{:d}", Add));
}

} // namespace model::test
