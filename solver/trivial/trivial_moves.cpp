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
  std::cout << "FIND ISOLATED IMPL (TOP)\n";

  // either a move (where to place a bulb) or an isolated mark, indicating
  // a mark that cannot be illuminated
  IsolatedCell result;
  board.visit_board([&](Coord coord, model::CellState cell) {
    if (is_illumable(cell)) {
      std::cout << "Looking at cell: " << cell << "\n";
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
  std::cout << "<<<<<<< FIND ISOLATED CELL (TOP) >>>>>>>>\n";
  IsolatedCell retval;
  find_isolated_impl(
      board, [&](Coord bulb_coord, Coord mark_coord, IsolatedResult result) {
        std::cout << ">>> !!! find_isolated: Bulb: " << bulb_coord
                  << ", mark: " << mark_coord << ": ";
        switch (result) {
          case MarkCannotBeLit:
            std::cout << "MARK CANNOT BE LIT\n";
            LOG_DEBUG("[FORCED ] Mark at {} cannot be illuminated\n",
                      mark_coord);
            retval.emplace<IsolatedMarkCoordinate>(mark_coord);
            break;
          case IsolatedEmpty:
            std::cout << "Isolatd Empty cell Requires bulb\n";
            LOG_DEBUG("[FORCED ] ADD Bulb: {} isolated Empty cell\n",
                      bulb_coord);
            retval =
                AnnotatedMove{model::SingleMove{model::Action::Add,
                                                model::CellState::Empty, // from
                                                model::CellState::Bulb,  // to
                                                bulb_coord},
                              DecisionType::ISOLATED_EMPTY_SQUARE,
                              MoveMotive::FORCED,
                              mark_coord};
            break;
          case IsolatedMark:
            std::cout << "Isolatd mark Requires bulb\n";
            LOG_DEBUG(
                "[FORCED ] ADD Bulb: {} Mark at {} must be illuminated by "
                "this\n",
                bulb_coord,
                mark_coord);

            retval =
                AnnotatedMove{model::SingleMove{model::Action::Add,
                                                model::CellState::Empty, // from
                                                model::CellState::Bulb,  // to
                                                bulb_coord},
                              DecisionType::ISOLATED_MARK,
                              MoveMotive::FORCED,
                              mark_coord};
            break;
        }
        return model::STOP_VISITING;
      });
  std::cout << "<<<<<<< FIND ISOLATED CELL (BOTTOM) >>>>>>>>\n";
  return retval;
}

// find all isolated cells and add to vector
bool
find_isolated_cells(model::BasicBoard const & board, AnnotatedMoves & moves) {
  bool success = true;
  find_isolated_impl(
      board, [&](Coord bulb_coord, Coord mark_coord, IsolatedResult result) {
        DecisionType decision;
        switch (result) {
          case MarkCannotBeLit:
            success = false;
            return model::STOP_VISITING;

          case IsolatedEmpty:
            decision = DecisionType::ISOLATED_EMPTY_SQUARE;
            break;

          case IsolatedMark:
            decision = DecisionType::ISOLATED_MARK;
            break;
          default:
            throw std::runtime_error("Unhandled IsolatedResult");
        }
        moves.emplace_back(model::SingleMove{model::Action::Add,
                                             model::CellState::Empty, // from
                                             model::CellState::Bulb,  // to
                                             bulb_coord},
                           decision,
                           MoveMotive::FORCED,
                           mark_coord);

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
      if (empty_count > 0 && (empty_count == deps - bulb_count)) {
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
find_walls_with_deps_equal_open_faces(model::BasicBoard const & board,
                                      AnnotatedMoves &          moves) {
  find_wall_deps_equal_faces_impl(board, [&](Coord coord) {
    board.visit_adjacent(coord, [&](Coord adj_coord, model::CellState cell) {
      if (is_empty(cell)) {

        moves.emplace_back(model::SingleMove{model::Action::Add,
                                             model::CellState::Empty, // from
                                             model::CellState::Bulb,  // to
                                             adj_coord},
                           DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
                           MoveMotive::FORCED,
                           coord);
        std::cout << "Adding move: " << moves.back() << std::endl;
      }
    });
    return model::KEEP_VISITING;
  });
}

void
find_satisfied_wall_with_open_faces_impl(model::BasicBoard const & board,
                                         auto && resultHandler) {
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
find_satisfied_walls_having_open_faces(model::BasicBoard const & board,
                                       AnnotatedMoves &          moves) {
  find_satisfied_wall_with_open_faces_impl(board, [&](Coord coord) {
    board.visit_adjacent(coord, [&](Coord adj_coord, auto cell) {
      if (is_empty(cell)) {
        moves.emplace_back(model::SingleMove{model::Action::Add,
                                             model::CellState::Empty, // from
                                             model::CellState::Bulb,  // to
                                             adj_coord},
                           DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
                           MoveMotive::FORCED,
                           coord);
      }
    });
    return model::KEEP_VISITING;
  });
}

OptCoord
find_satisfied_wall_having_open_faces(model::BasicBoard const & board) {
  OptCoord result;
  find_satisfied_wall_with_open_faces_impl(board, [&](Coord coord) {
    result = coord;
    return model::STOP_VISITING;
  });
  return result;
}

void
apply_move(Solution & solution, AnnotatedMove const & single_move) {
  solution.enqueue_move(single_move);
}

bool
find_trivial_moves(model::BasicBoard const & board, AnnotatedMoves & moves) {
  std::cout << "TOP OF find_trivial_moves" << std::endl;

  find_satisfied_walls_having_open_faces(board, moves);

  std::cout
      << "[find_trivial_moves 1) satisified with open faces ]: total size="
      << moves.size() << std::endl;
  for (auto & m : moves) {
    std::cout << m << std::endl;
  }

  find_walls_with_deps_equal_open_faces(board, moves);

  {
    std::cout << "[find_trivial_moves 2) deps==open]: size=" << moves.size()
              << std::endl;
    int i = 0;
    for (auto & m : moves) {
      std::cout << i << ": " << m << std::endl;
    }
  }

  bool is_valid = find_isolated_cells(board, moves);

  std::cout << "[find_trivial_moves 3) isolated cell]: size=" << moves.size()
            << std::endl;
  for (auto & m : moves) {
    std::cout << m << std::endl;
  }

  std::sort(begin(moves), end(moves));
  moves.erase(std::unique(begin(moves), end(moves)), end(moves));

  std::cout << "[find_trivial_moves {end}]: size=" << moves.size() << std::endl;
  for (auto & m : moves) {
    std::cout << m << std::endl;
  }

  return is_valid;
}

void
apply_move(Solution & solution, OptAnnotatedMove opt_move) {
  if (opt_move) {
    apply_move(solution, *opt_move);
  }
}

bool
play_any_forced_move(Solution & solution) {
  std::cout << "PLAY ANY FORCED MOVE\n";
  if (play_trivial_marks(solution)) {
    std::cout << "*** Found trivial marks\n";
    return true;
  }
  bool result = play_forced_move(solution);

  std::cout << "... no trivial, any other FORCED MOVEs?  " << result << "\n";

  return result;
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
  solution.add_step();
  auto const & board = solution.board().board();
  if (OptCoord coord = find_satisfied_wall_having_open_faces(board)) {
    board.visit_adjacent(*coord, [&](Coord adj_coord, auto cell) {
      if (cell == Empty) {
        apply_move(solution,
                   {model::SingleMove{model::Action::Add,
                                      model::CellState::Empty, // from
                                      model::CellState::Mark,  // to
                                      adj_coord},
                    DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES,
                    MoveMotive::FORCED,
                    *coord});
      };
    });

    return true;
  }
  std::cout << "NO trivial marks.\n";
  return false;
}

bool
play_forced_move(Solution & solution) {
  OptAnnotatedMove next_move;
  bool             found_error = false;

  auto const & board = solution.board().board();
  std::cout << "play_forced_move BOARD: " << board << std::endl;
  if (OptCoord opt_coord = find_wall_with_deps_equal_open_faces(board)) {
    solution.add_step();
    board.visit_adjacent(*opt_coord, [&](Coord coord, auto cell) {
      if (cell == Empty) {
        LOG_DEBUG("[FORCED ] ADD Bulb: {} Wall with N deps has N open faces\n",
                  coord);

        next_move = {model::SingleMove{model::Action::Add,
                                       model::CellState::Empty, // from
                                       model::CellState::Bulb,  // to
                                       coord},
                     DecisionType::WALL_DEPS_EQUAL_OPEN_FACES,
                     MoveMotive::FORCED,
                     *opt_coord};

        std::cout << "Setting next move to " << *next_move << std::endl;

        return model::STOP_VISITING;
      };
      return model::KEEP_VISITING;
    });
  }
  else {
    std::cout << "CHECKING FOR ISOLATED CELL\n";

    auto isolated_cell = find_isolated_cell(board);

    solution.add_step();

    // clang-format off
    std::visit(
      overloaded{
        [] (std::monostate) {},
        [&](AnnotatedMove const & move) {
          std::cout << "Setting next move to " << move << std::endl;
          next_move = move;
        },
        [&](IsolatedMarkCoordinate const & mark_coord) {
           found_error = true;
           solution.set_has_error(true);
        }
      },
      isolated_cell);
    // clang-format on
  }

  if (next_move) {
    apply_move(solution, *next_move);
    return true;
  }
  std::cout << "Returning false (play_forced_move)\n";
  return false;
}

} // namespace solver
