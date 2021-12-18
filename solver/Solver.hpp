#pragma once

#include "BasicBoard.hpp"
#include "Solution.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace solver {

using SinglePlay = std::function<bool(Solution &)>;

// if solution is provided, it can be validated against.
Solution solve(model::BasicBoard const &        board,
               std::optional<model::BasicBoard> known_solution = std::nullopt);

size_t speculate(Solution & solution);

bool check_solved(model::BasicBoard const & board);

bool find_moves(Solution & solution);

} // namespace solver
