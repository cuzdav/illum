#pragma once
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "SingleMove.hpp"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iosfwd>
#include <optional>

namespace solver {

enum class MoveMotive { FORCED, FOLLOWUP, SPECULATION };

struct AnnotatedMove {
  model::SingleMove next_move;
  DecisionType      reason;
  MoveMotive        motive;
  model::OptCoord   reference_location;

  friend auto operator<=>(AnnotatedMove const &,
                          AnnotatedMove const &) = default;
};

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

using OptAnnotatedMove = std::optional<AnnotatedMove>;

std::ostream & operator<<(std::ostream &        os,
                          AnnotatedMove const & solution_move);

} // namespace solver

template <>
struct fmt::formatter<::solver::AnnotatedMove> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::solver::AnnotatedMove const & amove, FormatContext & ctx) {
    return fmt::format_to(ctx.out(),
                          "#<AnnotatedMove:\n"
                          "\tmove={},\n"
                          "\treason={},\n"
                          "\tmotive={},\n"
                          "\tref_location={}>",
                          amove.next_move,
                          amove.reason,
                          amove.motive,
                          amove.reference_location);
  }
};

template <>
struct fmt::formatter<::solver::OptAnnotatedMove> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::solver::OptAnnotatedMove const & optmove, FormatContext & ctx) {
    if (optmove) {
      return fmt::format_to(ctx.out(), "{}", *optmove);
    }
    else {
      return fmt::format_to(ctx.out(), "(No AnnotatedMove)\n");
    }
  }
};
