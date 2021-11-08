
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

  GameState const &
  cur() const {
    return games_.back();
  }

  GameState &
  cur() {
    return games_.back();
  }

  int
  num_games() const {
    return games_.size();
  }

  void
  on_state_change(Action action, CellState prev_state, CellState to_state,
                  Coord coord) override {
    using enum model::Action;
    using enum model::CellState;

    switch (action) {

    case Add:
    case Remove: {
      if (games_.empty()) {
        throw std::runtime_error("Add/Remove without reset called first");
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
      game.moves_.push_back({action, to_state, coord});
      break;
    }

    case ResetGame: {
      Rows rows(coord.row_);
      for (auto & r : rows) {
        r.resize(coord.col_);
      }

      auto [height, width] = coord;
      games_.emplace_back(height,
                          width,
                          std::move(rows),
                          std::vector<SingleMove>{{action, to_state, coord}});
      break;
    }

    case StartGame: cur().moves_.push_back({action, to_state, coord}); break;

    default: throw std::runtime_error("Unknown State");
    }
  }

private:
  std::vector<GameState> games_;
};

} // namespace model::test
