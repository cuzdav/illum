#pragma once

#include "CellState.hpp"
#include "Coord.hpp"
#include "Direction.hpp"
#include <concepts>

namespace model {

// Non-directional visitors don't have a direction. They are used with
// visit-algorithms that are not "linear", but involve mult-direction scans,
// such as the whole board, or "what's adjacent to this cell"

//
// returning void indicates there is no early-stop while visiting
// If you don't care about short-circuiting the visit, don't return true.
//
template <typename T>
concept CellVisitorAll = requires(T visitor, CellState cell) {
  { visitor(Coord{0, 0}, cell) } -> std::same_as<void>;
};

//
// returning a bool indicates the visit may be stopped prematurely
//
template <typename T>
concept CellVisitorSome = requires(T visitor, CellState cell) {
  { visitor(Coord{0, 0}, cell) } -> std::same_as<bool>;
};

//
// Unify both Some/All cell visitors
// Many visit algos can work with either - when undirected
//
template <typename T>
concept CellVisitor = CellVisitorAll<T> || CellVisitorSome<T>;

// ==============

// Directed visitors are CellVisitors but are also notified of the direction
// they are going. This is generally associated with the visitors that travel
// along lines.

template <typename T>
concept DirectedCellVisitorAll = requires(T visitor, CellState cell) {
  { visitor(Direction{}, Coord{0, 0}, cell) } -> std::same_as<void>;
};

template <typename T>
concept DirectedCellVisitorSome = requires(T visitor, CellState cell) {
  { visitor(Direction{}, Coord{0, 0}, cell) } -> std::same_as<bool>;
};

//
// Short circuitable or not, up to the caller
//
template <typename T>
concept DirectedCellVisitor =
    DirectedCellVisitorAll<T> || DirectedCellVisitorSome<T>;

// The most flexible of all for the caller, short-circuitable or not, directed
// or not.
template <typename T>
concept OptDirCellVisitor = DirectedCellVisitor<T> || CellVisitor<T>;

} // namespace model
