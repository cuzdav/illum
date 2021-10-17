#include "BoardModel.hpp"
#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace model::test {

// Tracks whatever was emitted, and tries to keep a local state mimicing the
// announced state. Clearing the board pushes a new board rather than
// forgetting the old.

class TestStateChangeHandler : public BoardModel::StateChangeHandler {
public:
  struct Move {
    BoardModel::Action    action_;
    BoardModel::CellState state_;
    int                   row_;
    int                   col_;
  };

  using Col  = BoardModel::CellState;
  using Row  = std::vector<Col>;
  using Rows = std::vector<Row>;

  // Kept such that a reset can "clear" data but we don't lose the state of
  // the game before it.  A vector of GameStates just grows.
  struct GameState {
    int               height_;
    int               width_;
    Rows              rows_of_cells_;
    std::vector<Move> moves_;
  };

  GameState &
  cur() {
    return games_.back();
  }

  int
  num_games() const {
    return games_.size();
  }

  void
  onStateChange(BoardModel::Action action, BoardModel::CellState state, int row,
                int col) override {
    using enum model::BoardModel::Action;
    using enum model::BoardModel::CellState;

    switch (action) {

    case Add:
    case Remove: {
      if (games_.empty()) {
        throw std::runtime_error("Add/Remove without reset called first");
      }
      auto & game = cur();
      if (row < 0 || col < 0 || row >= game.height_ || col >= game.width_) {
        throw std::out_of_range(
            fmt::format("({},{}) out of bounds: Allowed range is ({},{})",
                        row,
                        col,
                        game.height_,
                        game.width_));
      }
      game.moves_.push_back({action, state, row, col});
      break;
    }

    case ResetGame: {
      Rows rows(row);
      for (auto & r : rows) {
        r.resize(col);
      }
      games_.emplace_back(row,
                          col,
                          std::move(rows),
                          std::vector<Move>{{action, state, row, col}});
      break;
    }

    case StartGame: break;

    default: throw std::runtime_error("Unknown State");
    }
  }

private:
  std::vector<GameState> games_;
};

} // namespace model::test
