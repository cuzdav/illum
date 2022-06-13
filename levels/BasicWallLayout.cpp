#include "BasicWallLayout.hpp"
#include "Adjacents.hpp"
#include "AnalysisBoard.hpp"
#include "BoardModel.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "PositionBoard.hpp"
#include "Solver.hpp"
#include "trivial_moves.hpp"
#include <iostream>

namespace levels {
inline namespace v1 {

using RNG          = std::mt19937;
using Distribution = std::uniform_int_distribution<RNG::result_type>;
using enum solver::DecisionType;

struct GenContext {
  RNG & rng;

  // current working board
  solver::AnalysisBoard & board;

  // output, once found
  std::optional<solver::Solution> solution;

  // a cache of reused objects
  std::vector<model::Coord>              empty_cells;
  std::unique_ptr<solver::BoardAnalysis> board_analysis =
      solver::create_board_analysis(board.basic_board());
  solver::AnnotatedMoves annotated_moves;

  int count_adjacent(model::Coord, model::CellState cell);
};

int
GenContext::count_adjacent(model::Coord coord, model::CellState cell) {
  int count = 0;
  board.visit_adjacent(coord, [&](model::Coord, model::CellState adj_cell) {
    count += adj_cell == cell;
  });
  return count;
}

bool fill_board_from_current_position(GenContext & context);

//

std::optional<model::Coord>
pick_random_empty_coord(GenContext & context) {
  auto & empty_cells = context.empty_cells;
  empty_cells.clear();

  context.board.visit_empty([&](model::Coord c, model::CellState cell) {
    assert(is_empty(cell));
    empty_cells.push_back(c);
  });

  if (empty_cells.empty()) {
    return std::nullopt;
  }
  auto dist = std::uniform_int_distribution<int>(0, empty_cells.size() - 1);

  unsigned int idx = dist(context.rng);

  assert(is_empty(context.board.get_cell(context.empty_cells.at(idx))));

  return context.empty_cells[idx];
}

Adjacents
get_adjacent_empties(GenContext & context, model::Coord coord) {
  Adjacents adjacents;
  context.board.visit_adjacent(
      coord, [&](model::Coord adj_coord, model::CellState cell) {
        if (is_empty(cell)) {
          adjacents.push_back(adj_coord);
        }
      });
  return adjacents;
}

void
sprinkle_random_walls(GenContext & context) {

  auto board_size = context.board.width() * context.board.height();
  auto distribute = Distribution(0, board_size - 1);

  for (int num_plain_walls = Distribution(3, (board_size - 1) / 3)(context.rng);
       num_plain_walls > 0;
       --num_plain_walls) {
    if (auto coord = pick_random_empty_coord(context)) {
      context.board.set_cell(
          *coord,
          model::CellState::WALL0,
          solver::PositionBoard::SetCellPolicy::FORCE_REEVALUATE_BOARD);
    }
  }
}

bool
verify_solvable(GenContext & context) {
  model::BoardModel model;
  model.reset_game(context.board.basic_board(),
                   model::BoardModel::ResetGamePolicy::ONLY_COPY_WALLS);
  auto solution = solver::solve(model.get_underlying_board());
  if (solution.is_solved()) {
    context.solution = std::move(solution);
    return true;
  }
  context.board = solution.board();
  return false;
}

void
add_more_stuff(GenContext &     context,
               solver::OptCoord location_hint = std::nullopt) {
  auto   coord = location_hint.has_value() ? *location_hint
                                           : pick_random_empty_coord(context);
  auto & board = context.board;

  if (not coord) {
    return;
  }
  auto empty_adjacents = get_adjacent_empties(context, *coord);
  switch (empty_adjacents.size()) {
    case 0:
      context.board.set_cell(*coord, model::CellState::WALL0);
      break;

    case 1:
      board.set_cell(*coord, model::CellState::WALL1);
      board.set_cell(empty_adjacents[0], model::CellState::BULB);
      break;

    case 2:
      board.set_cell(*coord, model::CellState::WALL2);
      board.set_cell(empty_adjacents[0], model::CellState::BULB);
      board.set_cell(empty_adjacents[1], model::CellState::BULB);
      break;

    case 3:
      board.set_cell(*coord, model::CellState::WALL3);
      board.set_cell(empty_adjacents[0], model::CellState::BULB);
      board.set_cell(empty_adjacents[1], model::CellState::BULB);
      board.set_cell(empty_adjacents[2], model::CellState::BULB);
      break;

    case 4:
      board.set_cell(*coord, model::CellState::WALL4);
      board.set_cell(empty_adjacents[0], model::CellState::BULB);
      board.set_cell(empty_adjacents[1], model::CellState::BULB);
      board.set_cell(empty_adjacents[2], model::CellState::BULB);
      board.set_cell(empty_adjacents[3], model::CellState::BULB);
      break;

    default:
      break;
  }
}

bool
fix_unilluminable_mark(GenContext & context) {
  using enum model::CellState;
  auto coord = *context.board.get_ref_location();

  // There are a few ways to possibly fix this, because unilluminable marks
  // can happen for different reasons.

  // 1) ADD a bulb to shine onto it. BULB may need extra walls to block
  //    other bulbs from seeing it, or new walls to force it.

  // 2) change mark to bulb and see if surroundings can be adjusted to "make
  //    it work". E.g. increase deps on nearby walls, or add a new wall with
  //    deps to force it to be a bulb.
  // 3) remove walls around it to allow for light to reach it.

  // 4) just fill it in with a WALL0

  // Each may or may not work, and could introduce their own problems.

  int neighboring_walls_of_mark = 0;
  context.board.visit_adjacent(
      coord, [&](model::Coord adj_coord, model::CellState adj_cell) {
        neighboring_walls_of_mark += is_wall(adj_cell);
      });

  // 3 neighboring walls around the mark should always be possible.
  if (neighboring_walls_of_mark == 3) {
    context.board.visit_adjacent(
        coord, [&](model::Coord adj_coord, model::CellState adj_cell) {
          int const deps = num_wall_deps(adj_cell);
          if (deps > 0) {
            // already has deps, so just add one to it to account for the bulb
            // we're installing
            context.board.set_cell(adj_coord, add_wall_dep(adj_cell));
          }
          else if (adj_cell == WALL0) {
            // ADD another bulb dep to this wall, so that it forces the mark to
            // be illuminated. NOTE: WALL0 may have adjacent bulbs, so count and
            // include them in the dependency count, plus the bulb we're adding.
            int adj_bulbs = 0;
            context.board.visit_adjacent(
                adj_coord, [&](model::Coord, model::CellState adj_wall_cell) {
                  adj_bulbs += is_bulb(adj_wall_cell);
                });
            context.board.set_cell(adj_coord,
                                   model::wall_with_deps(adj_bulbs + 1));
          }
        });
    return true;
  }

  // for now lets be simple and just put an empty wall here.
  context.board.set_cell(coord, model::CellState::WALL0);
  return true;
}

// increase or decrease deps on wall to fix its requirements.
bool
adjust_wall_to(GenContext & context, int delta_deps) {
  assert(delta_deps == 1 or delta_deps == -1);
  if (auto loc = context.board.get_ref_location()) {
    auto cell = context.board.get_cell(*loc);
    if (is_wall(cell)) {
      auto wall = delta_deps == 1 ? add_wall_dep(cell) : remove_wall_dep(cell);
      context.board.set_cell(*loc, wall);
      return true;
    }
  }
  return false;
}

bool
try_to_correct(GenContext & context) {
  switch (context.board.decision_type()) {

    case MARK_CANNOT_BE_ILLUMINATED:
      return fix_unilluminable_mark(context);

    case WALL_HAS_TOO_MANY_BULBS:
      return adjust_wall_to(context, +1);
      break;

    case WALL_CANNOT_BE_SATISFIED:
      return adjust_wall_to(context, -1);
      break;

    default:
      // nothing else to do
      break;
  }
  return false;
}

void
disambiguate(GenContext & context) {
  // When board is generated and solved, but solution is not discoverable from
  // an empty board, it's usually due to some old "marks" left on the board from
  // previous positions that were modified. So, the board may have solutions but
  // is ambiguous because there's not a single UNIQUE solution. So we can modify
  // walls to remove such cases once we have filled up a board.

  // Idea: find a wall that is not satisfied and set its deps to exactly what is
  // needed, to force it.
  context.board.visit_board([&](model::Coord coord, model::CellState cell) {
    if (int deps = model::num_wall_deps(cell); deps > 0) {
      int empties = 0;
      int bulbs   = 0;
      context.board.visit_adjacent(coord, [&](auto adj_coord, auto adj_cell) {
        empties += is_empty(adj_cell);
        bulbs += is_bulb(adj_cell);
      });
      if (deps - bulbs > empties) {
        context.board.set_cell(coord, model::wall_with_deps(bulbs + empties));
      }
    }
  });
}

bool
fill_board_from_current_position(GenContext & context) {
  solver::AnalysisBoard & board = context.board;

  int iter_count = 0;

  auto try_corrections = [&]() {
    int num_corrections = 0;
    while (board.has_error() && not board.is_ambiguous() &&
           ++num_corrections < 5) {
      try_to_correct(context);
    }
  };

  int illumable       = board.num_cells_needing_illumination();
  int walls_with_deps = board.num_walls_with_deps();

  while (not board.is_solved() && ++iter_count < 50) {

    if (board.num_cells_needing_illumination() < illumable) {
      illumable  = board.num_cells_needing_illumination();
      iter_count = 0;
    }
    else if (board.num_walls_with_deps() < walls_with_deps) {
      walls_with_deps = board.num_walls_with_deps();
      iter_count      = 0;
    }

    board.reevaluate_board_state();

    if (board.has_error() && not board.is_ambiguous()) {
      try_corrections();
    }

    board.set_has_error(false, solver::DecisionType::NONE);
    board.clone_position();
    add_more_stuff(context);

    if (board.has_error() && not board.is_ambiguous()) {
      try_corrections();
    }

    context.annotated_moves.clear();
    while (model::OptCoord invalid_mark_location =
               solver::find_trivial_moves(board.basic_board(),
                                          context.board_analysis.get(),
                                          context.annotated_moves)) {
      board.set_cell(*invalid_mark_location, model::CellState::WALL0);
    }
    for (auto & move : context.annotated_moves) {
      board.apply_move(move.next_move);
    }

    if (board.has_error()) {
      try_corrections();
      if (board.has_error() and not board.is_ambiguous()) {
        board.pop();
        illumable       = board.num_cells_needing_illumination();
        walls_with_deps = board.num_walls_with_deps();
      }
    }
    if (context.board.is_solved()) {
      // A board reporting it's solved means it's filled without detectable
      // errors, but doesn't mean that the solution is actually attainable
      // from a sequence of deductions. Therefore, replay from an empty board
      // to ensure that the "solution"" is actually reachable.
      if (verify_solvable(context)) {
        return true;
      }
      std::cout << "Disambiguating...\n" << board << "\n";
      disambiguate(context);
      std::cout << "AFTER Disambiguating...\n" << board << "\n";
    }
  }

  return verify_solvable(context);
}

} // v1

model::BoardModel
BasicWallLayout::create(RNG & rng, int height, int width) {

  solver::AnalysisBoard board(model::BasicBoard(height, width));

  GenContext context{rng, board, {}};

  sprinkle_random_walls(context);

  if (fill_board_from_current_position(context)) {
    assert(context.solution.has_value() && context.solution->is_solved());
    model::BoardModel model;
    model.reset_game(context.solution->board().board(),
                     model::BoardModel::ResetGamePolicy::ONLY_COPY_WALLS);
    return model;
  }

  std::cout << context.board << std::endl;
  throw std::runtime_error("Unable to generate.  Investigate");
}

} // namespace levels
