#pragma once

#include "Coord.hpp"

namespace model {

enum class Action;
enum class CellState;

class StateChangeHandler {
public:
  virtual ~StateChangeHandler() = default;

  virtual void on_state_change(Action action, CellState prev_state,
                               CellState to_state, Coord coord) = 0;
};

} // namespace model
