
#include "Action.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "StateChangeHandler.hpp"
#include <fmt/core.h>
#include <iostream> // debug
#include <stdexcept>
#include <vector>

namespace model::test {

// Tracks whatever was emitted, and tries to keep a local state mimicing the
// announced state. Clearing the board pushes a new board rather than
// forgetting the old.  So we end up with a sequence of gamestates.

class TestStateChangeHandler : public ::model::StateChangeHandler {
public:
  using Col  = CellState;
  using Row  = std::vector<Col>;
  using Rows = std::vector<Row>;

  // Kept such that a reset can "clear" data but we don't lose the state of
  // the game before it.  A vector of GameStates just grows.
  struct GameState {
    int                     height_;
    int                     width_;
    Rows                    rows_of_cells_;
    std::vector<SingleMove> moves_;
  };

  void
  set_trace(bool yn) const {
    debug_trace_ = yn;
  }

  GameState const &
  cur() const {
    return games_.back();
  }

  GameState &
  cur() {
    return games_.back();
  }

  SingleMove const &
  last_move() const {
    if (cur().moves_.size() == 0) {
      throw std::runtime_error(
          "Calling last move on empty TestStateChangeHandler");
    }
    return cur().moves_.back();
  }

  int
  num_games() const {
    return games_.size();
  }

  void
  on_state_change(Action    action,
                  CellState from_state,
                  CellState to_state,
                  Coord     coord) override {
    using enum model::Action;
    using enum model::CellState;

    if (debug_trace_) {
      std::cout << "TestSCH: on_state_change(" << action << ", " << from_state
                << " -> " << to_state << ", at " << coord << "\n";
    }

    switch (action) {

      case ADD:
      case REMOVE:
        {
          if (games_.empty()) {
            throw std::runtime_error("ADD/REMOVE without reset called first");
          }
          auto & game = cur();
          if (not coord.in_range(game.height_, game.width_)) {
            throw std::out_of_range(
                fmt::format("({},{}) out of bounds: Allowed range is ({},{})",
                            coord.row_,
                            coord.col_,
                            game.height_,
                            game.width_));
          }
          game.moves_.push_back({action, from_state, to_state, coord});
          break;
        }

      case RESETGame:
        {
          Rows rows(coord.row_);
          for (auto & r : rows) {
            r.resize(coord.col_);
          }

          auto [height, width] = coord;
          games_.push_back(
              GameState{height,
                        width,
                        std::move(rows),
                        std::vector<SingleMove>{
                            SingleMove{action, from_state, to_state, coord}}});
          break;
        }

      case START_GAME:
        {
          auto & game = cur();
          game.moves_.push_back({action, from_state, to_state, coord});
          break;
        }

      default:
        throw std::runtime_error("Unknown State");
    }
  }

private:
  mutable bool           debug_trace_;
  std::vector<GameState> games_;
};

} // namespace model::test
