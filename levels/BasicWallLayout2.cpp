#include "BasicWallLayout2.hpp"
#include "Adjacents.hpp"
#include "AnalysisBoard.hpp"
#include "BasicBoard.hpp"
#include "BoardModel.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "PositionBoard.hpp"
#include "Solver.hpp"
#include "trivial_moves.hpp"
#include <algorithm>
#include <iostream>


namespace levels {
inline namespace v2 {

using RNG          = std::mt19937;
using Distribution = std::uniform_int_distribution<RNG::result_type>;
using enum solver::DecisionType;

struct GenContext {
  RNG & rng;
  solver::PositionBoard           board;
  std::optional<solver::Solution> solution;
  std::optional<model::Coord>     prev_coordinate;
};

std::optional<model::Coord>
pick_random_empty(GenContext & context) {
  auto & board = context.board;
  if (board.num_cells_needing_illumination() == 0) {
    return std::nullopt;
  }

  std::vector<model::Coord> empty_cells;
  empty_cells.reserve(board.num_cells_needing_illumination());
  board.visit_empty(
      [&](model::Coord c, model::CellState cell) { empty_cells.push_back(c); });

  auto idx = static_cast<int>(Distribution(0, empty_cells.size())(context.rng));
  return empty_cells[idx];
}

class PlacementStrategy {
public:
  virtual ~PlacementStrategy() = default;
  virtual model::OptCoord next_bulb() = 0;
  virtual model::OptCoord next_wall() = 0;
};

class PureRandomPlacementStrategy : public PlacementStrategy {
public:
  PureRandomPlacementStrategy(GenContext & gen_context)
      : gen_context_{gen_context} {}

  model::OptCoord
  next_bulb() override {
    return pick_random_empty(gen_context_);
  }

  model::OptCoord
  next_wall() override {
    return pick_random_empty(gen_context_);
  }

private:
  std::reference_wrapper<GenContext> gen_context_;
};

int
count_adjacent(model::BasicBoard const & board,
               model::Coord              coord,
               model::CellState          cell) {
  int count = 0;
  board.visit_adjacent(coord, [&](model::Coord, model::CellState adj_cell) {
    count += adj_cell == cell;
  });
  return count;
}

std::optional<model::Coord>
pick_mirrored_random_empty_coord(GenContext & context) {
  if (context.prev_coordinate) {
    model::Coord mirrored{
        context.board.height() - context.prev_coordinate->row_ - 1,
        context.board.width() - context.prev_coordinate->col_ - 1};
    if (auto mirrored_cell = context.board.get_cell(mirrored);
        is_empty(mirrored_cell)) {
      context.prev_coordinate = std::nullopt;
      return mirrored;
    }
  }
  return pick_random_empty(context);
}

bool
sprinkle_random_walls(GenContext & context, int num_plain_walls) {
  for (; num_plain_walls > 0; --num_plain_walls) {
    if (auto coord = pick_mirrored_random_empty_coord(context)) {
      context.board.set_cell(*coord, model::CellState::WALL0);
    }
    else {
      return false;
    }
  }
  return true;
}

template <typename PlacementStrategyT>
void
add_bulbs(GenContext & context, PlacementStrategyT & placement_strategy) {
  // place lights on board which will be the "final" solution. Then we'll add
  // constraints and walls around them to create a unique unambiguous puzzle.
  // There may be diferent strategies for placing the lights, so we'll do this
  // with a weighted random placement, with weights decided by a
  // PlacementStrategy object, which uses the board in it's "current" state to
  // assign weights to empty cells on the board where lights could go.

  auto & board = context.board;
  while (context.board.num_cells_needing_illumination() > 0) {
    auto coord = placement_strategy.next_bulb();
    assert(coord.has_value());
    board.add_bulb(*coord);
  }
}

bool
verify_unique_solution(GenContext & context, model::BasicBoard const & board) {
  auto solution = solver::solve(board);
  if (solution.is_ambiguous()) {
    std::cerr
        << "*** Ready to add numbers and find solution with constraints\n";
    throw "Foo";
  }

  if (solution.is_solved()) {
    context.solution = std::move(solution);
    return true;
  }
  return false;
}

} // v2

model::BoardModel
BasicWallLayout2::create(RNG & rng, int height, int width) {

  GenContext context{.rng=rng, .board={height, width}};

  std::cout << "NUM CELLS NEEDING ILLUMINATION: "
            << context.board.num_cells_needing_illumination() << "\n";

  int num_initial_plain_walls =
    Distribution(2, height * width * 0.05)(context.rng);

  sprinkle_random_walls(context, num_initial_plain_walls);

  PureRandomPlacementStrategy placement_strategy(context);
  add_bulbs(context, placement_strategy);


  // bool has_unique_solution = false;
  // do {

  //   // auto solution          = find_solution(context);
  //   // auto constrained_board = add_wall_constraints(context);
  //   // has_unique_solution    = verify_unique_solution(context, constrained_board);
  // } while (has_unique_solution == false);

  fmt::print("{}\n", context.board);
  //  throw std::runtime_error("Unable to generate.  Investigate");

  model::BoardModel result = context.board.board();

  std::cout << "Returning result...\n" << std::endl;
  return result;
}

} // namespace levels
