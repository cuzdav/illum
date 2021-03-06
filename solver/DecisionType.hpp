#pragma once
#include "utils/EnumUtils.hpp"
#include <iostream>

namespace solver {

enum class DecisionType : std::uint8_t {
  NONE,

  // speculative
  SPECULATION,

  // forced move reason
  WALL_SATISFIED_HAVING_OPEN_FACES,
  WALL_DEPS_EQUAL_OPEN_FACES,
  ISOLATED_MARK,
  ISOLATED_EMPTY_SQUARE,

  // contradiction reasons
  BULBS_SEE_EACH_OTHER,
  MARK_CANNOT_BE_ILLUMINATED,
  WALL_HAS_TOO_MANY_BULBS,
  WALL_CANNOT_BE_SATISFIED,
  VIOLATES_SINGLE_UNIQUE_SOLUTION,
};

constexpr bool
is_forced_decision_type(DecisionType dt) {
  switch (dt) {
    case DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES:
    case DecisionType::WALL_DEPS_EQUAL_OPEN_FACES:
    case DecisionType::ISOLATED_MARK:
    case DecisionType::ISOLATED_EMPTY_SQUARE:
      return true;

    default:
      return false;
  }
}

constexpr bool
is_clustered_decision_type(DecisionType dt) {
  switch (dt) {
    case DecisionType::WALL_SATISFIED_HAVING_OPEN_FACES:
    case DecisionType::WALL_DEPS_EQUAL_OPEN_FACES:
      return true;

    default:
      return false;
  }
}

constexpr bool
is_contradiction_decsion_type(DecisionType dt) {
  return not is_forced_decision_type(dt);
}

constexpr char const *
to_string(DecisionType reason) {
  using enum DecisionType;
  switch (reason) {
    case NONE:
      return "NONE";

      // speculations

    case SPECULATION:
      return "SPECULATION";

      // forced move reasons

    case WALL_SATISFIED_HAVING_OPEN_FACES:
      return "WALL_SATISFIED_HAVING_OPEN_FACES";

    case WALL_DEPS_EQUAL_OPEN_FACES:
      return "WALL_DEPS_EQUAL_OPEN_FACES";

    case ISOLATED_MARK:
      return "ISOLATED_MARK";

    case ISOLATED_EMPTY_SQUARE:
      return "ISLOATED_EMPTY_SQUARE";

      // contradictions

    case BULBS_SEE_EACH_OTHER:
      return "BULBS_SEE_EACH_OTHER";

    case MARK_CANNOT_BE_ILLUMINATED:
      return "MARK_CANNOT_BE_ILLUMINATED";

    case VIOLATES_SINGLE_UNIQUE_SOLUTION:
      return "VIOLATES_SINGLE_UNIQUE_SOLUTION";

    case WALL_CANNOT_BE_SATISFIED:
      return "WALL_CANNOT_BE_SATISFIED";

    case WALL_HAS_TOO_MANY_BULBS:
      return "WALL_HAS_TOO_MANY_BULBS";

    default:
      return "(unhandled Decisionype)";
  }
}

inline std::ostream &
operator<<(std::ostream & os, DecisionType pt) {
  os << to_string(pt);
  return os;
}

} // namespace solver
