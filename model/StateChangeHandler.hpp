#pragma once

#include "Coord.hpp"
#include <cstdint>

namespace model {

enum class Action : char;
enum class CellState : std::uint16_t;

class StateChangeHandler {
public:
  virtual ~StateChangeHandler() = default;

  virtual void on_state_change(Action    action,
                               CellState prev_state,
                               CellState to_state,
                               Coord     coord) = 0;
};

} // namespace model
