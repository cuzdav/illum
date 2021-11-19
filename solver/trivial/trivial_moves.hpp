#pragma once
#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include <optional>
#include <variant>

namespace solver {

struct IsolatedMarkCoordinate {
  model::Coord coord_;
};

using OptMove  = std::optional<model::SingleMove>;
using OptCoord = std::optional<model::Coord>;

using IsolatedCell =
    std::variant<std::monostate, model::SingleMove, IsolatedMarkCoordinate>;

IsolatedCell find_isolated_cell(model::BasicBoard const & board);

OptCoord
find_wall_with_deps_equalling_open_faces(model::BasicBoard const & board);

OptCoord find_wall_with_satisfied_deps_having_open_faces(
    model::BasicBoard const & board);

// both trivial and forced moves
bool play_any_forced_move(Solution & solution);

// trivial marks are forced, but so trivial they don't count for speculation
// depth.  Play them all.
bool play_trivial_marks(Solution & solution);

// a simple forced move, but is counted for speculation depth.
bool play_forced_move(Solution & solution);

} // namespace solver
