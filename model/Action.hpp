#pragma once
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace model {

enum class Action { Add, Remove, ResetGame, StartGame };

namespace chr {
constexpr char Add       = '+';
constexpr char Remove    = '-';
constexpr char ResetGame = 'R';
constexpr char StartGame = 'S';
} // namespace chr

namespace str {

constexpr char const * Add       = "Add";
constexpr char const * Remove    = "Remove";
constexpr char const * ResetGame = "ResetGame";
constexpr char const * StartGame = "StartGame";
} // namespace str

constexpr std::string_view ActionNames[] = {
    str::Add, str::Remove, str::ResetGame, str::StartGame};

constexpr Action ActionValues[] = {
    Action::Add, Action::Remove, Action::ResetGame, Action::StartGame};

constexpr char
to_char(Action action) {
  using enum Action;
  switch (action) {
  case Add: return chr::Add;
  case Remove: return chr::Remove;
  case ResetGame: return chr::ResetGame;
  case StartGame: return chr::StartGame;
  default:
    throw std::runtime_error("Invalid Action in Serialize to_char()" +
                             std::to_string(static_cast<int>(action)));
  }
}

constexpr Action
get_action_from_char(char action) {
  using enum Action;
  switch (action) {
  case chr::Add: return Add;
  case chr::Remove: return Remove;
  case chr::ResetGame: return ResetGame;
  case chr::StartGame: return StartGame;
  default:
    throw std::runtime_error(
        std::string("Invalid Action in Serialize action_from_char: ") + action);
  }
}

constexpr char const *
to_string(Action action) {
  using enum Action;
  switch (action) {
  case Add: return str::Add;
  case Remove: return str::Remove;
  case ResetGame: return str::ResetGame;
  case StartGame: return str::StartGame;
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
