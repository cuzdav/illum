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

  TestStateChangeHandler handler;
  BoardModel             model(&handler);

  ::model::test::LevelCreator creator(&model);
  creator("00010");
  creator("0   1");
  creator("00010");
  creator.finished();

  auto serialized = ::model::serialize::to_ptree(model);

  TestStateChangeHandler handler2;
  BoardModel             model2(&handler2);

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

  for (int i = 0, e = model.width() * model.height(); i < e; ++i) {
    ASSERT_EQ(model.get_cell_from_flat_idx(i),
              model2.get_cell_from_flat_idx(i));
  }
}

TEST(Serializer, to_from_json) {

  TestStateChangeHandler handler;
  BoardModel             model(&handler);

  ::model::test::LevelCreator creator(&model);
  creator("0100010");
  creator("1  0  1");
  creator("010 010");
  creator("0  0  0");
  creator("1  0  1");
  creator("0000000");
  creator.finished();

  std::ostringstream oss;
  serialize::to_json_stream(oss, model);

  std::string streamed = oss.str();

  std::istringstream iss(streamed);

  TestStateChangeHandler handler2;
  BoardModel             model2(&handler2);

  serialize::setup_model_from_json_stream(iss, model2);

  ASSERT_EQ(model, model2);
}

} // namespace model::serialize::test
