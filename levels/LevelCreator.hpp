#pragma once

#include "BasicWallLayout.hpp"
#include "BoardModel.hpp"

namespace levels {

class LevelCreator {
public:
  LevelCreator(int height, int width);

private:
  model::BoardModel model_;
  BasicWallLayout   layout_;
};

} // namespace levels
