#pragma once
#include "BoardModel.hpp"
#include <random>

namespace levels {

class BasicWallLayout2 {
public:
  model::BoardModel create(std::mt19937 & rng, int height, int width);
};

} // namespace levels
