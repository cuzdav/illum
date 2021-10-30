#pragma once

#include "CellState.hpp"
#include <concepts>

namespace model {

//
// returning void indicates there is no early-stop while visiting
//
template <typename T>
concept CellVisitorAll = requires(T visitor) {
  { visitor(0, 0, CellState::Empty) } -> std::same_as<void>;
};

//
// returning a bool indicates the visit may be stopped prematurely
//
template <typename T>
concept CellVisitorSome = requires(T visitor) {
  { visitor(0, 0, CellState::Empty) } -> std::same_as<bool>;
};

template <typename T>
concept CellVisitor = CellVisitorAll<T> || CellVisitorSome<T>;

} // namespace model
