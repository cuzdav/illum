#pragma once
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "SingleMove.hpp"
#include <iosfwd>
#include <optional>

namespace solver {

enum class MoveMotive { FORCED, FOLLOWUP, SPECULATION };

constexpr char const *
to_string(MoveMotive const & mm) {
  using enum MoveMotive;
  switch (mm) {
    case FORCED:
      return "FORCED";
    case FOLLOWUP:
      return "FOLLOWUP";
    case SPECULATION:
      return "SPECULATION";
  }
  throw std::runtime_error("Unhandled MoveMotive");
}

struct AnnotatedMove {
  model::SingleMove next_move;
  DecisionType      reason;
  MoveMotive        motive;
  model::OptCoord   reference_location;

  friend auto operator<=>(AnnotatedMove const &,
                          AnnotatedMove const &) = default;
};

using OptAnnotatedMove = std::optional<AnnotatedMove>;

std::ostream & operator<<(std::ostream &        os,
                          AnnotatedMove const & solution_move);

} // namespace solver
