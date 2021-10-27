#pragma once

namespace model {

enum class Action;
enum class CellState;

class StateChangeHandler {
public:
  virtual ~StateChangeHandler()                                     = default;
  virtual void on_state_change(Action, CellState, int row, int col) = 0;
};

} // namespace model
