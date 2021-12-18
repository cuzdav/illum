#pragma once
#include "utils/EnumUtils.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace model {

enum class CellState : std::uint16_t {
  WALL0 = (1 << 0),
  WALL1 = (1 << 1),
  WALL2 = (1 << 2),
  WALL3 = (1 << 3),
  WALL4 = (1 << 4),
  EMPTY = (1 << 5),
  BULB  = (1 << 6),
  MARK  = (1 << 7),
  ILLUM = (1 << 8),
};

DEFINE_ENUM_BIT_OPERATIONS(CellState);

namespace cellstategroups {
using enum CellState;

// a piece the player can add to / remove from the board
constexpr CellState playable = BULB | MARK;

// a cell that can change during the course of play, either played directly by
// player, or added/removed as a side effect (i.e. illumination extending from a
// bulb adds illumination))
constexpr CellState dynamic_entity = BULB | EMPTY | MARK | ILLUM;

// a cell that needs illumination, but does not have it
constexpr CellState illuminable = EMPTY | MARK;

// a cell that a beam of light can pass through
constexpr CellState translucent = EMPTY | MARK | ILLUM;

// any kind of wall
constexpr CellState any_wall = WALL0 | WALL1 | WALL2 | WALL3 | WALL4;

// a wall at least one dependency
constexpr CellState wall_with_deps = WALL1 | WALL2 | WALL3 | WALL4;

} // namespace cellstategroups

// some of these predicates are super simple, but they are readable, and ADL
// friendly

// a piece the player can add to the board
constexpr bool
is_playable(CellState cell) {
  return cell == (cell & cellstategroups::playable);
}

// a cell that can change during the course of play, either played directly by
// player, or added/removed as a side effect (i.e. illumination extending from a
// bulb adds illumination)).  Also thought of as, "not a wall"
constexpr bool
is_dynamic_entity(CellState cell) {
  return cell == (cell & cellstategroups::dynamic_entity);
}

// a cell that needs illumination, but does not have it
constexpr bool
is_illuminable(CellState cell) {
  return cell == (cell & cellstategroups::illuminable);
}

// a cell that a beam of light can pass through
constexpr bool
is_translucent(CellState cell) {
  return cell == (cell & cellstategroups::translucent);
}

constexpr bool
is_empty(CellState cell) {
  return cell == CellState::EMPTY;
}

constexpr bool
is_mark(CellState cell) {
  return cell == CellState::MARK;
}

constexpr bool
is_bulb(CellState cell) {
  return cell == CellState::BULB;
}

// any wall
constexpr bool
is_wall(CellState cell) {
  return cell == (cell & cellstategroups::any_wall);
}

// a wall with at least one bulb dependency
constexpr bool
is_wall_with_deps(CellState cell) {
  return cell == (cell & cellstategroups::wall_with_deps);
}

// Convert WALL4 to WALL3, or WALL1 to WALL0, etc. Leave non-walls alone.
constexpr CellState
remove_wall_dep(CellState cell) {
  if (is_wall_with_deps(cell)) {
    return CellState(+cell >> 1);
  }
  return cell;
}

constexpr CellState
add_wall_dep(CellState cell) {
  // WALL4 is the max
  if (+cell >= +CellState::WALL0 && +cell < +CellState::WALL4) {
    return CellState(+cell << 1);
  }
  return cell;
}

constexpr int
num_wall_deps(CellState cell) {
  switch (cell) {
    case CellState::WALL1:
      return 1;
    case CellState::WALL2:
      return 2;
    case CellState::WALL3:
      return 3;
    case CellState::WALL4:
      return 4;
    default:
      return 0;
  }
}

constexpr CellState
wall_with_deps(int num_deps) {
  switch (num_deps) {
    case 0:
      return CellState::WALL0;
    case 1:
      return CellState::WALL1;
    case 2:
      return CellState::WALL2;
    case 3:
      return CellState::WALL3;
    case 4:
      return CellState::WALL4;
    default:
      throw std::logic_error("invalid dep count for wall");
  }
}

namespace chr {
constexpr char BULB  = '*';
constexpr char ILLUM = '+';
constexpr char EMPTY = '.';
constexpr char WALL0 = '0';
constexpr char WALL1 = '1';
constexpr char WALL2 = '2';
constexpr char WALL3 = '3';
constexpr char WALL4 = '4';
constexpr char MARK  = 'X';
} // namespace chr

namespace str {
constexpr char const * BULB  = "BULB";
constexpr char const * EMPTY = "EMPTY";
constexpr char const * ILLUM = "ILLUM";
constexpr char const * WALL0 = "WALL0";
constexpr char const * WALL1 = "WALL1";
constexpr char const * WALL2 = "WALL2";
constexpr char const * WALL3 = "WALL3";
constexpr char const * WALL4 = "WALL4";
constexpr char const * MARK  = "MARK";

} // namespace str

// keep sorted and in "pairwise lockstep" with CellStateValues
constexpr std::string_view CellStateNames[] = {str::BULB,
                                               str::EMPTY,
                                               str::ILLUM,
                                               str::MARK,
                                               str::WALL0,
                                               str::WALL1,
                                               str::WALL2,
                                               str::WALL3,
                                               str::WALL4};

// keep sorted and in "pairwise lockstep" with CellStateNames
constexpr CellState CellStateValues[] = {CellState::BULB,
                                         CellState::EMPTY,
                                         CellState::ILLUM,
                                         CellState::MARK,
                                         CellState::WALL0,
                                         CellState::WALL1,
                                         CellState::WALL2,
                                         CellState::WALL3,
                                         CellState::WALL4};

constexpr char
to_char(CellState state) {
  using enum CellState;
  switch (state) {
    case EMPTY:
      return chr::EMPTY;
    case ILLUM:
      return chr::ILLUM;
    case WALL0:
      return chr::WALL0;
    case WALL1:
      return chr::WALL1;
    case WALL2:
      return chr::WALL2;
    case WALL3:
      return chr::WALL3;
    case WALL4:
      return chr::WALL4;
    case BULB:
      return chr::BULB;
    case MARK:
      return chr::MARK;

    default:
      throw std::runtime_error(
          std::string("Invalid CellState in Serialize to_char(): ") +
          std::to_string(static_cast<int>(state)));
  }
}

constexpr CellState
get_state_from_char(char state) {
  using enum CellState;
  switch (state) {
    case chr::EMPTY:
      return EMPTY;
    case chr::ILLUM:
      return ILLUM;
    case chr::WALL0:
      return WALL0;
    case chr::WALL1:
      return WALL1;
    case chr::WALL2:
      return WALL2;
    case chr::WALL3:
      return WALL3;
    case chr::WALL4:
      return WALL4;
    case chr::BULB:
      return BULB;
    case chr::MARK:
      return MARK;
    default:
      throw std::runtime_error(
          std::string("Invalid State char in Serialize state_from_char(): ") +
          state);
  }
}

constexpr char const *
to_string(CellState state) {
  using enum CellState;
  switch (state) {
    case EMPTY:
      return str::EMPTY;
    case ILLUM:
      return str::ILLUM;
    case WALL0:
      return str::WALL0;
    case WALL1:
      return str::WALL1;
    case WALL2:
      return str::WALL2;
    case WALL3:
      return str::WALL3;
    case WALL4:
      return str::WALL4;
    case BULB:
      return str::BULB;
    case MARK:
      return str::MARK;
    default:
      throw std::runtime_error("Invalid CellState in Serialize to_string: " +
                               std::to_string(static_cast<int>(state)));
  };
}

inline CellState
get_state_from_string(std::string_view str) {
  if (auto begin = std::begin(CellStateNames),
      end        = std::end(CellStateNames),
      it         = std::lower_bound(begin, end, str);
      it != end && *it == str) {
    return CellStateValues[std::distance(begin, it)];
  }
  throw std::runtime_error("Unknown CellState string in state_from_string: " +
                           std::string(str.data(), str.size()));
}

inline std::ostream &
operator<<(std::ostream & os, CellState state) {
  return os << to_string(state);
}

} // namespace model
