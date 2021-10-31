#pragma once

#include "BasicBoard.hpp"
#include "Solution.hpp"

namespace solver {

Solution solve(model::BasicBoard const & board);

bool check_solved(model::BasicBoard const & board);

void play_single_move(Solution & solution);

} // namespace solver
