#pragma once

#include "AnnotatedMove.hpp"
#include "BasicBoard.hpp"
#include "Coord.hpp"
#include "DecisionType.hpp"
#include "SingleMove.hpp"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iosfwd>
#include <span>
#include <vector>

namespace solver {

class Hint {
public:
  using MovesSpan = std::span<AnnotatedMove const>;

  // A cluster of moves (usually just 1) considered to be done "together"
  class ExplainStep {
  public:
    using value_type     = AnnotatedMove;
    using iterator       = AnnotatedMove const *;
    using const_iterator = iterator;

    ExplainStep(DecisionType reason = DecisionType::NONE) : reason_{reason} {}

    MovesSpan moves() const;
    bool      add(AnnotatedMove const & move);
    int       size() const;

    AnnotatedMove &
    operator[](int idx) {
      return moves_[idx];
    }
    AnnotatedMove const &
    operator[](int idx) const {
      return moves_[idx];
    }

    const_iterator
    begin() const {
      return moves_;
    }

    iterator
    begin() {
      return moves_;
    }

    const_iterator
    end() const {
      return moves_ + count_;
    }

    iterator
    end() {
      return moves_ + count_;
    }

    friend std::ostream & operator<<(std::ostream &, Hint const &);

  private:
    AnnotatedMove moves_[4]{};
    DecisionType  reason_;
    int           count_ = 0;
  };

  using ExplainSteps  = std::vector<ExplainStep>;
  using ConstIterator = ExplainSteps::const_iterator;

  ExplainStep const &  next_moves() const;
  ExplainSteps const & explain_steps() const;
  DecisionType         reason() const;
  ConstIterator        begin() const;
  ConstIterator        end() const;

  bool
  has_error() const {
    return has_error_;
  }

  static Hint create(model::BasicBoard const & board);

private:
  Hint(DecisionType reason);

private:
  bool         has_error_ = false;
  DecisionType reason_;
  ExplainStep  next_moves_;    // what to play
  ExplainSteps explain_steps_; // shows contradiction
};

inline Hint::Hint(DecisionType reason) : reason_(reason), next_moves_{} {}
inline Hint::MovesSpan
Hint::ExplainStep::moves() const {
  return {moves_, moves_ + count_};
}

inline bool
Hint::ExplainStep::add(AnnotatedMove const & move) {
  if (count_ == 4) {
    return false;
  }
  moves_[count_++] = move;
  return true;
}

inline int
Hint::ExplainStep::size() const {
  return count_;
}

inline Hint::ExplainStep const &
Hint::next_moves() const {
  return next_moves_;
}

inline DecisionType
Hint::reason() const {
  return reason_;
}

inline Hint::ExplainSteps const &
Hint::explain_steps() const {
  return explain_steps_;
}

inline Hint::ConstIterator
Hint::begin() const {
  return explain_steps_.begin();
}

inline Hint::ConstIterator
Hint::end() const {
  return explain_steps_.end();
}

} // namespace solver

template <>
struct fmt::formatter<::solver::Hint::ExplainStep> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::solver::Hint::ExplainStep const & step, FormatContext & ctx) {
    fmt::format_to(ctx.out(), "#<Step\n");

    for (solver::AnnotatedMove const & step : step.moves()) {
      fmt::format_to(ctx.out(), "\t\t{}\n", step);
    }
    return fmt::format_to(ctx.out(), "\t]>");
  }
};

template <>
struct fmt::formatter<::solver::Hint> {
  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::solver::Hint const & hint, FormatContext & ctx) {
    fmt::format_to(ctx.out(),
                   "#<Hint:\n"
                   "\treason={}\n"
                   "\tnext_move={},\n"
                   "\tSteps: [\n",
                   hint.reason(),
                   hint.next_moves());

    for (auto const & step : hint.explain_steps()) {
      fmt::format_to(ctx.out(), "\t{}\n", step);
    }
    return fmt::format_to(ctx.out(), "\t]>");
  }
};

void inline f() {
  solver::AnnotatedMove m;
  std::cout << fmt::format("{}", m);

  solver::DecisionType d{};
  std::cout << fmt::format("{}", d);

  // auto hint = solver::Hint::create(model::BasicBoard{});
  // fmt::format("{}", hint);
}
