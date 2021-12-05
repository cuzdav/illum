#pragma once
#include "utils/EnumUtils.hpp"
#include <iosfwd>

namespace model {

enum class Direction : char { None, Up = 1, Down = 2, Right = 4, Left = 8 };
DEFINE_ENUM_BIT_OPERATIONS(Direction);

namespace directiongroups {
using enum Direction;
constexpr Direction all        = Up | Down | Right | Left;
constexpr Direction horizontal = Left | Right;
constexpr Direction vertical   = Up | Down;
} // namespace directiongroups

constexpr char const *
to_string(Direction d) {
  switch (d) {
    case Direction::None:
      return "None";
    case Direction::Up:
      return "Up";
    case Direction::Down:
      return "Down";
    case Direction::Left:
      return "Left";
    case Direction::Right:
      return "Right";
  }
  return "<Unhandled direction>";
}

std::ostream & operator<<(std::ostream & os, Direction d);

constexpr Direction
rotate90_left(Direction d) {
  using enum Direction;
  switch (d) {
    case Up:
      return Left;
    case Left:
      return Down;
    case Down:
      return Right;
    case Right:
      return Up;
    default:
      return None;
  }
}

constexpr Direction
flip(Direction dir) {
  using enum Direction;
  switch (dir) {
    case Up:
      return Down;
    case Left:
      return Right;
    case Down:
      return Up;
    case Right:
      return Left;
    default:
      return None;
  }
}

} // namespace model
