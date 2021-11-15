#pragma once

#include "BasicBoard.hpp"
#include "Solution.hpp"
#include <functional>
#include <vector>

namespace solver {

using SinglePlay = std::function<bool(Solution &)>;

Solution solve(model::BasicBoard const & board);

bool check_solved(model::BasicBoard const & board);

bool play_single_move(Solution & solution);

} // namespace solver
