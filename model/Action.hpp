#pragma once
#include "utils/EnumUtils.hpp"
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace model {

enum class Action : char { ADD, REMOVE, RESET_GAME, START_GAME };

constexpr char
ordinal(Action value) {
  return static_cast<char>(value);
}
constexpr int Action_ORDINAL_BITS = 2;

namespace chr {
constexpr char ADD        = '+';
constexpr char REMOVE     = '-';
constexpr char RESET_GAME = 'R';
constexpr char START_GAME = 'S';
} // namespace chr

namespace str {

constexpr char const * ADD        = "ADD";
constexpr char const * REMOVE     = "REMOVE";
constexpr char const * RESET_GAME = "RESET_GAME";
constexpr char const * START_GAME = "START_GAME";
} // namespace str

constexpr std::string_view ActionNames[] = {
    str::ADD, str::REMOVE, str::RESET_GAME, str::START_GAME};

constexpr Action ActionValues[] = {
    Action::ADD, Action::REMOVE, Action::RESET_GAME, Action::START_GAME};

constexpr char
to_char(Action action) {
  using enum Action;
  switch (action) {
    case ADD:
      return chr::ADD;
    case REMOVE:
      return chr::REMOVE;
    case RESET_GAME:
      return chr::RESET_GAME;
    case START_GAME:
      return chr::START_GAME;
    default:
      throw std::runtime_error("Invalid Action in Serialize to_char()" +
                               std::to_string(static_cast<int>(action)));
  }
}

constexpr Action
get_action_from_char(char action) {
  using enum Action;
  switch (action) {
    case chr::ADD:
      return ADD;
    case chr::REMOVE:
      return REMOVE;
    case chr::RESET_GAME:
      return RESET_GAME;
    case chr::START_GAME:
      return START_GAME;
    default:
      throw std::runtime_error(
          std::string("Invalid Action in Serialize action_from_char: ") +
          action);
  }
}

constexpr char const *
to_string(Action action) {
  using enum Action;
  switch (action) {
    case ADD:
      return str::ADD;
    case REMOVE:
      return str::REMOVE;
    case RESET_GAME:
      return str::RESET_GAME;
    case START_GAME:
      return str::START_GAME;
    default:
      throw std::runtime_error("Unknown Action in Serialize to_string: " +
                               std::to_string(static_cast<int>(action)));
  };
}

inline Action
get_action_from_string(std::string_view str) {
  if (auto begin = std::begin(ActionNames),
      end        = std::end(ActionNames),
      it         = std::lower_bound(begin, end, str);
      it != end && *it == str) {
    return ActionValues[std::distance(begin, it)];
  }
  throw std::runtime_error("Unknown Action in action_from_string: " +
                           std::string(str.data(), str.size()));
}

inline std::ostream &
operator<<(std::ostream & os, Action action) {
  return os << to_string(action);
}

} // namespace model

// template <>
// struct fmt::formatter<model::Action> {
//   template <typename ParseContext>
//   constexpr auto
//   parse(ParseContext & ctx) {
//     return ctx.begin();
//   }

//   template <typename FormatContext>
//   auto
//   format(model::Action const & action, FormatContext & ctx) {
//     return fmt::format_to(ctx.out(), "{}", to_string(action));
//   }
// };
