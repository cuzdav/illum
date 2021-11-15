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

using OptMove = std::optional<model::SingleMove>;

using IsolatedCell =
    std::variant<std::monostate, model::SingleMove, IsolatedMarkCoordinate>;

IsolatedCell find_isolated_cell(model::BasicBoard const & board);

OptMove
find_wall_with_deps_equalling_open_faces(model::BasicBoard const & board);

OptMove
find_wall_with_satisfied_deps_and_open_faces(model::BasicBoard const & board);

bool play_trivial_move(Solution & solution);

} // namespace solver
