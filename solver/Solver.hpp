#pragma once

#include "BasicBoard.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace solver {

using SinglePlay = std::function<bool(Solution &)>;

// resets the caches to be empty
void initialize_speculation_context(Solution & solution);

// adds a new speculation context to the cache
Solution::ContextCache &
add_speculation_context_for_move(Solution & solution, model::SingleMove move);

// solution needs speculation_context initialized before calling. Either
// initialize_speculation_context followed by add_speculation_context_for_move
// (called n times for n moves), or indirectly, as calling speculate() will
// invoke these too for all possible bulbs and marks on the given board.
size_t speculate_over_cache(Solution & solution);

size_t speculate(Solution & solution);

// Applies all trivial moves until there are none, then speculates to find the
// next move, until it's solved or runs into a board error.
// if solution is provided, it can be validated against. (TODO)
Solution solve(model::BasicBoard const &        board,
               std::optional<model::BasicBoard> known_solution = std::nullopt);

bool check_solved(model::BasicBoard const & board);

bool find_moves(Solution & solution);

} // namespace solver
