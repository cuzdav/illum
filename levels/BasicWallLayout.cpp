#include "BasicWallLayout.hpp"

namespace levels {

using RNG = std::mt19937;

model::BoardModel
BasicWallLayout::create(RNG & rng, int height, int width) {

  // model::BoardModel board(nullptr);
  // board.reset_game(height, width);

  // std::uniform_int_distribution<RNG::result> pct(5, 50);
  // int num_walls = std::uniform_int_distribution<rng::result> distribute(
  //     0, width * height - 1);

  // std::uniform_int_distribution<RNG::result> distribute(0, width * height -
  // 1);

  // std::random_device os_seed;
  // const u32          seed = os_seed();

  // engine                             generator(seed);
  //  std::uniform_int_distribution<u32> distribute(1, 6);

  // std::cout << distribute(generator) << std::endl;

  return {nullptr};
}
} // namespace levels
