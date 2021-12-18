#pragma once
#include "Action.hpp"
#include "BasicBoard.hpp"
#include "CellState.hpp"
#include "Coord.hpp"
#include "SingleMove.hpp"
#include "StateChangeHandler.hpp"
#include <iosfwd>
#include <memory>
#include <string_view>
#include <vector>

namespace model {

// game is a sequence of state changes. The first is RESETGame to establish
// dimensions. Then it's followed by a sequence of add() calls to place walls
// and such, setting up the level. Then a START_GAME marker, to separate the
// setup from the player moves (and to serve as a sentinel in the moves list
// to stop UNDO from going before user moves.)

// The level setup and player moves are in the same sequence for serialziation
// purposes and easy re-creatability. Replaying this list of moves should
// recreate the level and user plays, including all undo information. This is
// also why player requests to remove a piece is still added as a move, which is
// distinctly different from an undo, which makes the "moves" list smaller.

class BoardModel {
public:
  BoardModel();
  BoardModel(std::unique_ptr<StateChangeHandler> handler);

  BoardModel(BoardModel const &) = delete;
  BoardModel & operator=(BoardModel const &) = delete;

  BoardModel(BoardModel &&) = default;
  BoardModel & operator=(BoardModel &&) = default;

  bool                  operator==(BoardModel const &) const;
  friend std::ostream & operator<<(std::ostream &, BoardModel const &);

  // general sequence of events:
  // 1) reset_game()    -- initialize a new empty board with given dimensions
  // 2) <LEVEL SETUP>   -- add walls to setup level
  // 3) START_GAME()    -- add marker to separate level setup from player moves
  //                       esp. to protect UNDO from going back too far
  // 4) <PLAYER MOVES>  -- sequence of add/remove calls
  // These are all appeneded to the vector of moves.  (For purposes of undo,
  // even "remove" is added, so it can be undone to return to previous state.)

  enum class RESETGamePolicy { COPY_PLAYER_MOVES, ONLY_COPY_WALLS };

  void reset_game(int height, int width); // empty
  void reset_game(
      BasicBoard const & initial_board,
      RESETGamePolicy copy_player_moves = RESETGamePolicy::COPY_PLAYER_MOVES);
  void start_game();

  void add(CellState, Coord coord);
  void apply(SingleMove move);
  void remove(Coord coord);

  // This actually removes entries from the moves vector, but cannot go
  // past/before the start-of-game marker.  Returns true if something was
  // undone.  If game not started, or moves not played, it returns false.
  bool undo();
  bool restart_game(); // clear user-moves

  template <typename MoveHandlerT>
  void for_each_move(MoveHandlerT && handler) const;

  StateChangeHandler const * get_handler() const;

  void set_state_change_handler(std::unique_ptr<StateChangeHandler> handler);

  int  num_total_moves() const;
  int  num_played_moves() const;
  bool started() const;

  BasicBoard const & get_underlying_board() const;
  CellState          get_cell(Coord coord) const;
  int                width() const;
  int                height() const;

  void visit_board(CellVisitor auto && visitor) const;

private:
  void apply_move(Action, CellState from, CellState to, Coord coord);
  void on_state_change(Action, CellState from, CellState to, Coord coord);

private:
  bool started_         = false;
  int  num_setup_moves_ = 0; // played before any user moves
  std::unique_ptr<StateChangeHandler> handler_;
  std::vector<SingleMove>             moves_;
  BasicBoard                          board_;
};

template <typename MoveHandlerT>
void
BoardModel::for_each_move(MoveHandlerT && handler) const {
  int i = 0;
  for (auto & move : moves_) {
    handler(i++, move);
  }
}

inline StateChangeHandler const *
BoardModel::get_handler() const {
  return handler_.get();
}

inline int
BoardModel::num_total_moves() const {
  return moves_.size();
}

inline int
BoardModel::num_played_moves() const {
  return moves_.size() - num_setup_moves_;
  ;
}

inline bool
BoardModel::started() const {
  return started_;
}

inline BasicBoard const &
BoardModel::get_underlying_board() const {
  return board_;
}

inline CellState
BoardModel::get_cell(Coord coord) const {
  return board_.get_cell(coord);
}

inline int
BoardModel::width() const {
  return board_.width();
}

inline int
BoardModel::height() const {
  return board_.height();
}

inline void
BoardModel::visit_board(CellVisitor auto && visitor) const {
  board_.visit_board(std::forward<decltype(visitor)>(visitor));
}

} // namespace model
