#include "Action.hpp"
#include "gtest/gtest.h"

namespace model::test {

using namespace std::literals;
using enum Action;

TEST(ActionTest, action_to_from_string_test) {
  EXPECT_EQ(ADD, get_action_from_string(to_string(ADD)));
  EXPECT_EQ(REMOVE, get_action_from_string(to_string(REMOVE)));
  EXPECT_EQ(RESETGame, get_action_from_string(to_string(RESETGame)));
  EXPECT_EQ(START_GAME, get_action_from_string(to_string(START_GAME)));
}

TEST(ActionTest, to_from_char) {
  EXPECT_EQ(ADD, get_action_from_char(to_char(ADD)));
  EXPECT_EQ(REMOVE, get_action_from_char(to_char(REMOVE)));
  EXPECT_EQ(RESETGame, get_action_from_char(to_char(RESETGame)));
  EXPECT_EQ(START_GAME, get_action_from_char(to_char(START_GAME)));
}

TEST(ActionTest, Format_char) {
  static_assert(enumutils::Stringable<Action>);
  static_assert(enumutils::Charable<Action>);

  EXPECT_EQ(to_string(ADD), fmt::format("{:s}", ADD));
  EXPECT_EQ(to_string(ADD), fmt::format("{}", ADD));

  EXPECT_EQ(std::string(1, to_char(ADD)), fmt::format("{:c}", ADD));

  EXPECT_EQ(std::to_string(+ADD), fmt::format("{:d}", ADD));
}

} // namespace model::test
