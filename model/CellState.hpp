#pragma once
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace model {

enum class CellState { Wall0, Wall1, Wall2, Wall3, Wall4, Empty, Bulb, Mark };

namespace chr {
constexpr char Bulb  = '*';
constexpr char Empty = '.';
constexpr char Wall0 = '0';
constexpr char Wall1 = '1';
constexpr char Wall2 = '2';
constexpr char Wall3 = '3';
constexpr char Wall4 = '4';
constexpr char Mark  = 'X';
} // namespace chr

namespace str {
constexpr char const * Bulb  = "Bulb";
constexpr char const * Empty = "Empty";
constexpr char const * Wall0 = "Wall0";
constexpr char const * Wall1 = "Wall1";
constexpr char const * Wall2 = "Wall2";
constexpr char const * Wall3 = "Wall3";
constexpr char const * Wall4 = "Wall4";
constexpr char const * Mark  = "Mark";

} // namespace str

// keep names/values sorted and in "pairwise lockstep"
constexpr std::string_view CellStateNames[] = {str::Bulb,
                                               str::Empty,
                                               str::Mark,
                                               str::Wall0,
                                               str::Wall1,
                                               str::Wall2,
                                               str::Wall3,
                                               str::Wall4};

constexpr CellState CellStateValues[] = {CellState::Bulb,
                                         CellState::Empty,
                                         CellState::Mark,
                                         CellState::Wall0,
                                         CellState::Wall1,
                                         CellState::Wall2,
                                         CellState::Wall3,
                                         CellState::Wall4};

constexpr char
to_char(CellState state) {
  using enum CellState;
  switch (state) {
  case Empty: return chr::Empty;
  case Wall0: return chr::Wall0;
  case Wall1: return chr::Wall1;
  case Wall2: return chr::Wall2;
  case Wall3: return chr::Wall3;
  case Wall4: return chr::Wall4;
  case Bulb: return chr::Bulb;
  case Mark: return chr::Mark;

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
  case chr::Empty: return Empty;
  case chr::Wall0: return Wall0;
  case chr::Wall1: return Wall1;
  case chr::Wall2: return Wall2;
  case chr::Wall3: return Wall3;
  case chr::Wall4: return Wall4;
  case chr::Bulb: return Bulb;
  case chr::Mark: return Mark;
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
  case Empty: return str::Empty;
  case Wall0: return str::Wall0;
  case Wall1: return str::Wall1;
  case Wall2: return str::Wall2;
  case Wall3: return str::Wall3;
  case Wall4: return str::Wall4;
  case Bulb: return str::Bulb;
  case Mark: return str::Mark;
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