#pragma once
#include <stack>
#include <vector>

namespace model {

// game is a sequence of state changes. The first is ResetGame to establish
// dimensions. Then it's followed by a sequence of add() calls to place walls
// and such, setting up the level. Then a StartGame marker, to separate the
// setup from the player moves (and to serve as a sentinel in the moves list
// to stop UNDO from going before user moves.)

// The level setup and player moves are in the same sequence for serialziation
// purposes and easy re-creatability. Replaying this list of moves should
// recreate the level and user plays, including all undo information. This is
// also why player requests to remove a piece is still added as a move, which is
// distinctly different from an undo, which makes the "moves" list smaller.

class BoardModel {
public:
  enum class CellState { Empty, Wall0, Wall1, Wall2, Wall3, Wall4, Bulb, Mark };
  enum class Action { Add, Remove, ResetGame, StartGame };

  class StateChangeHandler {
  public:
    virtual ~StateChangeHandler() = default;

    virtual void onStateChange(Action, CellState, int row, int col) = 0;
  };

  BoardModel(StateChangeHandler * handler);

  // general sequence of events:
  // 1) reset_game()    -- initialize a new empty board with given dimensions
  // 2) <LEVEL SETUP>   -- add walls to setup level
  // 3) START_GAME()    -- add marker to separate level setup from player moves
  //                       esp. to protect UNDO from going back too far
  // 4) <PLAYER MOVES>  -- sequence of add/remove calls
  // These are all appeneded to the vector of moves.  (For purposes of undo,
  // even "remove" is added, so it can be undone to return to previous state.)
  void reset_game(int height, int width);
  void start_game();
  void add(CellState, int row, int col);
  void remove(int row, int col);

  // This actually removes entries from the moves vector, but cannot go past
  // the start-of-game marker.
  void undo();

  CellState
  get_cell(int row, int col) const {
    return cells_[get_idx(row, col)];
  }

  bool
  started() const {
    return started_;
  }

  int
  width() const {
    return width_;
  }

  int
  height() const {
    return height_;
  }

  int
  num_moves() const {
    return moves_.size();
  }

  template <typename VisitorT>
  void
  visit_cells(VisitorT && visitor) const {
    for (int i = 0; i < size(cells_); ++i) {
      visitor.accept(row_from_idx(i), col_from_idx(i), cells_[i]);
    }
  }

  template <typename MoveHandlerT>
  void
  for_each_move(MoveHandlerT && handler) const {
    int i = 0;
    for (auto & move : moves_) {
      handler(i++, move);
    }
  }

  StateChangeHandler *
  get_handler() const {
    return handler_;
  }

private:
  int get_idx(int row, int col) const;

  int
  row_from_idx(int idx) const {
    return idx / width_;
  }

  int
  col_from_idx(int idx) const {
    return idx % width_;
  }

  void apply_move(Action, CellState, int row, int col);

  struct Move {
    Action    action_;
    CellState state_;
    int       row_;
    int       col_;
  };

private:
  StateChangeHandler *   handler_;
  bool                   started_ = false;
  int                    height_  = 0;
  int                    width_   = 0;
  std::vector<CellState> cells_;
  std::vector<Move>      moves_;
};

} // namespace model
