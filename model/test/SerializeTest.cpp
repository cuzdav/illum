#include "Serialize.hpp"
#include "BoardModel.hpp"
#include "LevelCreator.hpp"
#include "TestStateChangeHandler.hpp"
#include "gtest/gtest.h"
#include <string_view>

namespace model::serialize::test {

using enum CellState;
using enum Action;

using namespace ::testing;
using namespace ::model::test;

using namespace std::literals;

TEST(Serializer, to_ptree) {

  TestStateChangeHandler handler;
  BoardModel             model(&handler);

  ::model::test::LevelCreator creator(&model);
  creator("00000");
  creator("0   1");
  creator("00100");
  creator.finished();

  auto serialized = ::model::serialize::to_ptree(model);

  ::model::serialize::to_json_file("/tmp/foo.json", serialized);

  TestStateChangeHandler handler2;
  BoardModel             model2(&handler2);

  ::model::serialize::setup_model_from_ptree(serialized, model2);

  ASSERT_EQ(model, model2);
}

} // namespace model::serialize::test
