#pragma once
#include <iosfwd>

namespace model {

enum class Direction : char { None, Up, Down, Right, Left };

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

} // namespace model
