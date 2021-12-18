#pragma once
#include "utils/EnumUtils.hpp"
#include <iosfwd>

namespace model {

enum class Direction : char { NONE, UP = 1, DOWN = 2, RIGHT = 4, LEFT = 8 };
DEFINE_ENUM_BIT_OPERATIONS(Direction);

namespace directiongroups {
using enum Direction;
constexpr Direction all        = UP | DOWN | RIGHT | LEFT;
constexpr Direction horizontal = LEFT | RIGHT;
constexpr Direction vertical   = UP | DOWN;
} // namespace directiongroups

constexpr char const *
to_string(Direction d) {
  switch (d) {
    case Direction::NONE:
      return "NONE";
    case Direction::UP:
      return "UP";
    case Direction::DOWN:
      return "DOWN";
    case Direction::LEFT:
      return "LEFT";
    case Direction::RIGHT:
      return "RIGHT";
  }
  return "<Unhandled direction>";
}

std::ostream & operator<<(std::ostream & os, Direction d);

constexpr Direction
rotate90_left(Direction d) {
  using enum Direction;
  switch (d) {
    case UP:
      return LEFT;
    case LEFT:
      return DOWN;
    case DOWN:
      return RIGHT;
    case RIGHT:
      return UP;
    default:
      return NONE;
  }
}

constexpr Direction
flip(Direction dir) {
  using enum Direction;
  switch (dir) {
    case UP:
      return DOWN;
    case LEFT:
      return RIGHT;
    case DOWN:
      return UP;
    case RIGHT:
      return LEFT;
    default:
      return NONE;
  }
}

} // namespace model
