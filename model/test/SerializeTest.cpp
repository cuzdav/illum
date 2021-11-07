#include "Serialize.hpp"
#include "BoardModel.hpp"
#include "LevelCreator.hpp"
#include "TestStateChangeHandler.hpp"
#include "gtest/gtest.h"
#include <sstream>
#include <string_view>

namespace model::serialize::test {

using enum CellState;
using enum Action;

using namespace ::testing;
using namespace ::model::test;

using namespace std::literals;

TEST(Serializer, to_ptree) {

  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler = *model.get_handler();

  ::model::test::LevelCreator creator;
  creator("00010");
  creator("0...1");
  creator("00010");
  creator.finished(&model);

  auto serialized = ::model::serialize::to_ptree(model);

  BoardModel   model2(std::make_unique<TestStateChangeHandler>());
  auto const & handler2 = *model.get_handler();

  ASSERT_NE(model, model2);
  ASSERT_NE(model.width(), model2.width());
  ASSERT_NE(model.height(), model2.height());
  ASSERT_NE(model.started(), model2.started());
  ASSERT_NE(model.num_moves(), model2.num_moves());

  // after this they should match
  ::model::serialize::setup_model_from_ptree(serialized, model2);

  ASSERT_EQ(model, model2);
  ASSERT_EQ(model.width(), model2.width());
  ASSERT_EQ(model.height(), model2.height());
  ASSERT_EQ(model.started(), model2.started());
  ASSERT_EQ(model.num_moves(), model2.num_moves());

  for (int r = 0; r < model.height(); ++r) {
    for (int c = 0; c < model.width(); ++c) {
      ASSERT_EQ(model.get_cell({r, c}), model2.get_cell({r, c}));
    }
  }
}

TEST(Serializer, to_from_json) {

  BoardModel   model(std::make_unique<TestStateChangeHandler>());
  auto const & handler = *model.get_handler();

  ::model::test::LevelCreator creator;
  creator("0100010");
  creator("1..0..1");
  creator("010.010");
  creator("0..0..0");
  creator("1..0..1");
  creator("0000000");
  creator.finished(&model);

  std::ostringstream oss;
  serialize::to_json_stream(oss, model);

  std::string streamed = oss.str();

  std::istringstream iss(streamed);

  BoardModel   model2(std::make_unique<TestStateChangeHandler>());
  auto const & handler2 = *model.get_handler();

  serialize::setup_model_from_json_stream(iss, model2);

  ASSERT_EQ(model, model2);
}

} // namespace model::serialize::test
