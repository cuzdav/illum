#include "trivial_moves.hpp"
#include "Action.hpp"
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "DebugLog.hpp"
#include "SingleMove.hpp"
#include "Solution.hpp"
#include "meta.hpp"
#include <optional>

namespace solver {

using enum model::CellState;
using model::Coord;

// Handles 3 simple cases:

// 1) Empty cell cannot see any empty cells in any direction - must be a bulb.
// 2) Mark cannot see any empty cells in any direction - contradiction
// 3) Mark can *only* see one empty cell in any direction - that cell must have
//    a bulb.

enum IsolatedResult { MarkCannotBeLit, IsolatedEmpty, IsolatedMark };

// Result handler sig :
// VisitStatus(Coord bulb_coord, Coord mark_coord, IsolatedResult);
//
// * bulb_coord is where the bulb should be played.
// * mark_coord is only used if MarkCannotBeLit (mark that cannot be
// illuminated),
//   or IsolatedMark (mark has only one possible cell that can light it)

void
find_isolated_impl(model::BasicBoard const & board, auto && resultHandler) {
  IsolatedCell result;
  board.visit_board([&](Coord coord, model::CellState cell) {
    if (is_illumable(cell)) {
      OptCoord empty_cell_location;
      int      visible_empty_neighbors = 0;
      board.visit_rows_cols_outward(coord,
                                    [&](Coord coord, model::CellState cell) {
                                      if (is_empty(cell)) {
                                        visible_empty_neighbors++;
                                        empty_cell_location = coord;
                                      }
                                    });

      if (visible_empty_neighbors == 0) {
        if (is_empty(cell)) {
          return resultHandler(coord, coord, IsolatedEmpty);
        }
        else {
          return resultHandler(coord, coord, MarkCannotBeLit);
        }
      }
      else if (is_mark(cell) and visible_empty_neighbors == 1) {
        return resultHandler(*empty_cell_location, coord, IsolatedMark);
      }
    }
    return model::KEEP_VISITING;
  });
}

// find a single instance of an isolated mark or empty cell, and return it in
// the variant
IsolatedCell
find_isolated_cell(model::BasicBoard const & board) {
  IsolatedCell retval;
  find_isolated_impl(
      board, [&](Coord bulb_coord, Coord mark_coord, IsolatedResult result) {
        switch (result) {
          case MarkCannotBeLit:
            LOG_DEBUG("[FORCED ] Mark at {} cannot be illuminated\n",
                      mark_coord);
            retval.emplace<IsolatedMarkCoordinate>(mark_coord);
            break;
          case IsolatedEmpty:
            LOG_DEBUG("[FORCED ] ADD Bulb: {} isolated Empty cell\n",
                      bulb_coord);
            retval.emplace<model::SingleMove>(model::Action::Add,
                                              model::CellState::Empty, // from
                                              model::CellState::Bulb,  // to
                                              bulb_coord);
            break;
          case IsolatedMark:
            LOG_DEBUG(
                "[FORCED ] ADD Bulb: {} Mark at {} must be illuminated by "
                "this\n",
                bulb_coord,
                mark_coord);

            retval.emplace<model::SingleMove>(model::Action::Add,
                                              model::CellState::Empty, // from
                                              model::CellState::Bulb,  // to
                                              bulb_coord);
            break;
        }
        return model::STOP_VISITING;
      });
  return retval;
}

// find all isolated cells and add to vector
bool
find_isolated_cells(model::BasicBoard const & board, Moves & moves) {
  bool success = true;
  find_isolated_impl(
      board, [&](Coord bulb_coord, Coord mark_coord, IsolatedResult result) {
        switch (result) {
          case MarkCannotBeLit:
            success = false;
            return model::STOP_VISITING;

          case IsolatedEmpty:
          case IsolatedMark:
            moves.emplace_back(model::Action::Add,
                               model::CellState::Empty, // from
                               model::CellState::Bulb,  // to
                               bulb_coord);
            break;
        }
        return model::KEEP_VISITING;
      });
  return success;
}

void
find_wall_deps_equal_faces_impl(model::BasicBoard const & board,
                                auto &&                   resultHandler) {
  board.visit_board([&](Coord coord, model::CellState cell) {
    if (int deps = num_wall_deps(cell); deps > 0) {
      int empty_count = 0;
      int bulb_count  = 0;
      board.visit_adjacent(coord, [&](Coord, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
      if (empty_count == deps - bulb_count) {
        return resultHandler(coord);
      }
    }
    return model::KEEP_VISITING;
  });
}

OptCoord
find_wall_with_deps_equal_open_faces(model::BasicBoard const & board) {
  OptCoord result;
  find_wall_deps_equal_faces_impl(board, [&](Coord coord) {
    result = coord;
    return model::STOP_VISITING;
  });
  return result;
}

void
find_walls_with_deps_equalling_open_faces(model::BasicBoard const & board,
                                          Moves &                   moves) {
  find_wall_deps_equal_faces_impl(board, [&](Coord coord) {
    board.visit_adjacent(coord, [&](Coord coord, model::CellState cell) {
      if (is_empty(cell)) {
        moves.emplace_back(
            model::Action::Add, cell, model::CellState::Bulb, coord);
      }
    });
    return model::KEEP_VISITING;
  });
}

void
find_satisfied_with_open_faces_impl(model::BasicBoard const & board,
                                    auto &&                   resultHandler) {
  board.visit_board([&](Coord coord, model::CellState cell) {
    if (int deps = num_wall_deps(cell)) {
      int bulb_count  = 0;
      int empty_count = 0;
      board.visit_adjacent(coord, [&](Coord, auto cell) {
        empty_count += cell == Empty;
        bulb_count += cell == Bulb;
      });
      if (bulb_count == deps && empty_count > 0) {
        return resultHandler(coord);
      }
    }
    return model::KEEP_VISITING;
  });
}

void
find_satisfied_wall_having_open_faces(model::BasicBoard const & board,
                                      Moves &                   moves) {
  find_satisfied_with_open_faces_impl(board, [&](Coord coord) {
    board.visit_adjacent(coord, [&](Coord coord, auto cell) {
      if (is_empty(cell)) {
        moves.emplace_back(
            model::Action::Add, cell, model::CellState::Bulb, coord);
      }
    });
    return model::KEEP_VISITING;
  });
}

OptCoord
find_satisfied_wall_having_open_faces(model::BasicBoard const & board) {
  OptCoord result;
  find_satisfied_with_open_faces_impl(board, [&](Coord coord) {
    board.visit_adjacent(coord, [&](Coord coord, auto cell) {
      if (is_empty(cell)) {
        result = coord;
        return model::STOP_VISITING;
      }
      return model::KEEP_VISITING;
    });

    return result ? model::STOP_VISITING : model::KEEP_VISITING;
  });
  return result;
}

void
apply_move(Solution & solution, model::SingleMove single_move) {
  switch (single_move.to_) {
    case model::CellState::Bulb:
      solution.board_.add_bulb(single_move.coord_);
      break;

    case model::CellState::Mark:
      solution.board_.add_mark(single_move.coord_);
      break;

    default:
      break;
  }
}

bool
find_trivial_moves(model::BasicBoard const & board, Moves & moves) {
  find_satisfied_wall_having_open_faces(board, moves);
  find_walls_with_deps_equalling_open_faces(board, moves);
  return find_isolated_cells(board, moves);
}

void
apply_move(Solution & solution, OptMove opt_move) {
  if (opt_move) {
    apply_move(solution, *opt_move);
  }
}

bool
play_any_forced_move(Solution & solution) {
  if (not play_trivial_marks(solution)) {
    return play_forced_move(solution);
  }
  return true;
}

// trivial moves are played in 1 of 3 situations.  The square is empty
// and...

// 1) cell cannot be illuminated by any other square, must be a bulb

// 2) cell is adjacent to wall that requires N additional adjacent bulbs and
// also has exactly N empty adjacent squares. That is, every open face
// requires a bulb. So it must be a bulb.

// 3) A wall with N deps has N bulbs next to it, and M open faces.  Each
// of those M faces must be marked. 4) a marked cell that can only be
// illuminated by a single other square
//    on the same row or column.

// split into separate functions because trivial marks don't count toward
// depth in speculation.  They are considered "played with the bulb".

// But new bulbs, or isolated marks, are considered new moves.
bool
play_trivial_marks(Solution & solution) {
  solution.step_count_++;
  auto const & board = solution.board_.board();
  if (OptCoord coord = find_satisfied_wall_having_open_faces(board)) {
    board.visit_adjacent(*coord, [&](Coord coord, auto cell) {
      if (cell == Empty) {
        apply_move(solution,
                   model::SingleMove{model::Action::Add,
                                     model::CellState::Empty, // from
                                     model::CellState::Mark,  // to
                                     coord});
      };
    });
    return true;
  }
  return false;
}

bool
play_forced_move(Solution & solution) {
  OptMove next_move;

  auto const & board = solution.board_.board();
  if (OptCoord opt_coord = find_wall_with_deps_equal_open_faces(board)) {
    solution.step_count_++;
    LOG_DEBUG("[FORCED ] ADD Bulb: {} Wall with N deps has N open faces\n",
              *opt_coord);
    board.visit_adjacent(*opt_coord, [&](Coord coord, auto cell) {
      if (cell == Empty) {
        next_move = model::SingleMove{model::Action::Add,
                                      model::CellState::Empty, // from
                                      model::CellState::Bulb,  // to
                                      coord};
        return model::STOP_VISITING;
      };
      return model::KEEP_VISITING;
    });
    return true;
  }
  else if (auto isolated_cell = find_isolated_cell(board);
           isolated_cell.index() != 0) {
    solution.step_count_++;
    // clang-format off
    std::visit(
      overloaded{
        [] (std::monostate) {},
        [&](model::SingleMove const & move) {
          next_move = move;
        },
        [&](IsolatedMarkCoordinate const & mark_coord) {
           solution.board_.set_has_error(true);
        }
      },
      isolated_cell);
    // clang-format on
  }

  if (next_move) {
    apply_move(solution, next_move);
    return true;
  }
  return false;
}

} // namespace solver
