#include "BoardModel.hpp"
#include "Serialize.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace model {

using namespace std::literals;

BoardModel::BoardModel(std::unique_ptr<StateChangeHandler> handler)
    : handler_(std::move(handler)) {}

BoardModel::BoardModel() : BoardModel(nullptr) {}

bool
BoardModel::operator==(BoardModel const & other) const {
  return started_ == other.started_ && board_ == other.board_ &&
         moves_ == other.moves_;
}

std::ostream &
operator<<(std::ostream & os, BoardModel const & model) {
  os << "BoardModel<" << (model.handler_ ? "<*>" : "<NULL>")
     << ", started: " << std::boolalpha << model.started_ << ", moves: [";

  for (SingleMove const & m : model.moves_) {
    os << "\n\t" << m;
  }
  os << "], " << model.board_ << ">";
  return os;
}

void
BoardModel::start_game() {
  if (not started_) {
    started_ = true;
    // Adds a marker into game histroy separating the level setup from
    // the player's moves.
    moves_.push_back({Action::StartGame, CellState::Empty, Coord{-1, -1}});
    on_state_change(
        Action::StartGame, CellState::Empty, CellState::Empty, Coord{-1, -1});
  }
}

void
BoardModel::undo() {
  if (started_ and moves_.back().action_ != Action::StartGame) {
    auto last = moves_.back();
    moves_.pop_back();
    CellState from, to;
    Action    action;
    if (last.action_ == Action::Add) {
      action = Action::Remove;
      from   = CellState::Empty;
      to     = last.state_;
    }
    else if (last.action_ == Action::Remove) {
      action = Action::Add;
      from   = last.state_;
      to     = CellState::Empty;
    }
    else {
      throw std::runtime_error("undoing a non-add, non-remove action: "s +
                               to_string(last.action_));
    }

    on_state_change(action, from, to, last.coord_);
  }
}

void
BoardModel::apply_move(Action action, CellState to_state, Coord coord) {
  if (not board_.is_initialized()) {
    throw std::runtime_error("Board uninitialized");
  }
  CellState orig_cell = board_.get_cell(coord);
  if (board_.set_cell(coord, to_state)) {
    moves_.push_back({action, to_state, coord});
    on_state_change(action, orig_cell, to_state, coord);
  }
  else {
    throw std::runtime_error("Invalid coordinate for apply_move");
  }
}

void
BoardModel::add(CellState state, Coord coord) {
  apply_move(Action::Add, state, coord);
}

void
BoardModel::apply(SingleMove move) {
  apply_move(move.action_, move.state_, move.coord_);
}

void
BoardModel::remove(Coord coord) {
  if (not started_) {
    throw std::runtime_error("Cannot remove cells before game has started");
  }
  CellState orig_cell = board_.get_cell(coord);
  if (orig_cell == CellState::Bulb or orig_cell == CellState::Mark) {
    apply_move(Action::Remove, CellState::Empty, coord);
  }
}

void
BoardModel::reset_game(int height, int width) {
  started_ = false;
  moves_.clear();
  board_.reset(height, width);
  Coord coord{height, width};
  moves_.push_back({Action::ResetGame, CellState::Empty, coord});
  on_state_change(Action::ResetGame, CellState::Empty, CellState::Empty, coord);
}

void
BoardModel::reset_game(BasicBoard const & initial_board) {
  started_ = false;
  moves_.clear();
  Coord dims = {initial_board.height(), initial_board.width()};
  moves_.push_back({Action::ResetGame, CellState::Empty, dims});
  on_state_change(Action::ResetGame, CellState::Empty, CellState::Empty, dims);

  board_ = initial_board;

  std::vector<SingleMove> deferred;

  board_.visit_board([this, &deferred](Coord coord, CellState cell) {
    if (cell != CellState::Empty) {
      if (cell == CellState::Illum || cell == CellState::Bulb) {
        deferred.emplace_back(Action::Add, cell, coord);
      }
      else {
        moves_.push_back({Action::Add, cell, coord});
      }
    }
  });
  for (auto move : deferred) {
    board_.set_cell(move.coord_, move.state_);
  }
}

void
BoardModel::on_state_change(Action action, CellState prev_state,
                            CellState to_state, Coord coord) {
  if (handler_) {
    handler_->on_state_change(action, prev_state, to_state, coord);
  }
}

} // namespace model
