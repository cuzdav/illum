#pragma once
#include "BasicBoard.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include <optional>

namespace solver {

std::optional<model::SingleMove>
find_isolated_empty_cell(model::BasicBoard const & board);

std::optional<model::SingleMove>
find_wall_with_deps_equalling_open_faces(model::BasicBoard const & board);

std::optional<model::SingleMove>
find_wall_with_satisfied_deps_and_open_faces(model::BasicBoard const & board);

bool play_trivial_move(Solution & solution);

} // namespace solver
