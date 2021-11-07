#pragma once
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include <array>
#include <iostream>
#include <optional>
#include <stdexcept>

namespace model {

class BasicBoard {
public:
  static int constexpr MAX_CELLS = 625;

  bool operator==(BasicBoard const & other) const;

  void reset(int height, int width);
  bool is_initialized() const;

  CellState                get_cell(Coord coord) const;
  std::optional<CellState> get_opt_cell(Coord coord) const;

  bool set_cell(Coord coord, CellState state);

  template <typename PredT>
  bool set_cell_if(Coord coord, CellState state, PredT pred);

  int width() const;
  int height() const;

  // visitor is invoked for each cell with (row, col, cellstate)
  // visitor may return void or bool:
  //   void == visit all cells              (CellVisitorAll)
  //   bool == true:keep-going, false:stop  (CellVisitorSome)

  // returns false if CellVisitorSome stopped visiting prematurely.
  // Otherwise returns true.
  bool visit_board(CellVisitor auto && visitor) const;
  bool visit_row_left_of(Coord coord, CellVisitor auto && visitor) const;
  bool visit_row_right_of(Coord coord, CellVisitor auto && visitor) const;
  bool visit_col_above(Coord coord, CellVisitor auto && visitor) const;
  bool visit_col_below(Coord coord, CellVisitor auto && visitor) const;

  // Note: Use care with a CellVisitorSome. If cell visitor stops early, there
  // is no guarantee of order of what was visited first, so only break if search
  // is for existence of something and it's found, so no more searching
  // necessary.
  bool visit_rows_cols_outward(Coord coord, CellVisitor auto && visitor) const;

  // same caveat as visit_rows_cols_outward
  bool visit_adjacent(Coord coord, CellVisitor auto && visitor) const;

  friend std::ostream & operator<<(std::ostream &, BasicBoard const &);

private:
  int  get_flat_idx(Coord coord) const;
  int  get_flat_idx_unchecked(Coord coord) const;
  bool visit_cell(Coord, int idx, CellVisitorSome auto && visitor) const;
  bool visit_cell(Coord, int idx, CellVisitorAll auto && visitor) const;

private:
  int                              height_ = 0;
  int                              width_  = 0;
  std::array<CellState, MAX_CELLS> cells_;
};

inline bool
BasicBoard::operator==(BasicBoard const & other) const {
  if (height_ != other.height_ || width_ != other.width_) {
    return false;
  }
  for (int i = 0, e = height_ * width_; i < e; ++i) {
    if (cells_[i] != other.cells_[i]) {
      return false;
    }
  }
  return true;
}

inline void
BasicBoard::reset(int height, int width) {
  for (auto & cell : cells_) {
    cell = CellState::Empty;
  }
  height_ = height;
  width_  = width;
  if (height_ < 0 || width_ < 0 || height_ * width_ >= cells_.size()) {
    throw std::runtime_error("Invalid dimensions");
  }
}

inline bool
BasicBoard::is_initialized() const {
  return width_ * height_ > 0;
}

inline CellState
BasicBoard::get_cell(Coord coord) const {
  if (int idx = get_flat_idx(coord); idx != -1) {
    return cells_[idx];
  }
  throw std::range_error("Out of bounds");
}

inline std::optional<CellState>
BasicBoard::get_opt_cell(Coord coord) const {
  if (int idx = get_flat_idx(coord); idx != -1) {
    return cells_[idx];
  }
  return std::nullopt;
}

inline bool
BasicBoard::set_cell(Coord coord, CellState state) {
  if (auto idx = get_flat_idx(coord); idx != -1) {
    cells_[idx] = state;
    return true;
  }
  return false;
}

template <typename PredT>
bool
BasicBoard::set_cell_if(Coord coord, CellState state, PredT pred) {
  if (auto idx = get_flat_idx(coord); idx > -1 && pred(cells_[idx])) {
    cells_[idx] = state;
    return true;
  }
  return false;
}

inline int
BasicBoard::get_flat_idx_unchecked(Coord coord) const {
  return coord.row_ * width_ + coord.col_;
}

inline int
BasicBoard::get_flat_idx(Coord coord) const {
  if (coord.in_range(height_, width_)) {
    return get_flat_idx_unchecked(coord);
  }
  return -1;
}

inline int
BasicBoard::width() const {
  return width_;
}

inline int
BasicBoard::height() const {
  return height_;
}

inline bool
BasicBoard::visit_cell(Coord coord, int i,
                       CellVisitorSome auto && visitor) const {
  assert(get_flat_idx(coord) == i);
  return visitor(coord, cells_[i]);
}

inline bool
BasicBoard::visit_cell(Coord coord, int i,
                       CellVisitorAll auto && visitor) const {
  assert(get_flat_idx(coord) == i);
  visitor(coord, cells_[i]);
  return true;
}

inline bool
BasicBoard::visit_board(CellVisitor auto && visitor) const {
  for (int r = 0, c = 0, i = 0; r < height_; ++i) {
    if (not visit_cell(Coord{r, c}, i, visitor)) {
      return false;
    }
    if (++c == width_) {
      c = 0;
      ++r;
    }
  }
  return true;
}

inline bool
BasicBoard::visit_row_left_of(Coord coord, CellVisitor auto && visitor) const {
  --coord.col_;
  int idx = get_flat_idx(coord);
  if (idx >= 0) {
    while (coord.col_ >= 0) {
      if (not visit_cell(coord, idx, visitor)) {
        return false;
      }
      --coord.col_;
      --idx;
    }
  }
  return true;
}

inline bool
BasicBoard::visit_row_right_of(Coord coord, CellVisitor auto && visitor) const {
  ++coord.col_;
  int idx = get_flat_idx(coord);
  if (idx >= 0) {
    while (coord.col_ < width_) {
      if (not visit_cell(coord, idx, visitor)) {
        return false;
      }
      ++coord.col_;
      ++idx;
    }
  }
  return true;
}

inline bool
BasicBoard::visit_col_above(Coord coord, CellVisitor auto && visitor) const {
  --coord.row_;
  int idx = get_flat_idx(coord);
  if (idx >= 0) {
    while (coord.row_ >= 0) {
      if (not visit_cell(coord, idx, visitor)) {
        return false;
      }
      --coord.row_;
      idx -= width_;
    }
  }
  return true;
}

inline bool
BasicBoard::visit_col_below(Coord coord, CellVisitor auto && visitor) const {
  ++coord.row_;
  int idx = get_flat_idx(coord);
  if (idx >= 0) {
    while (coord.row_ < height_) {
      if (not visit_cell(coord, idx, visitor)) {
        return false;
      }
      ++coord.row_;
      idx += width_;
    }
  }
  return true;
}

inline bool
BasicBoard::visit_rows_cols_outward(Coord               coord,
                                    CellVisitor auto && visitor) const {
  return visit_row_left_of(coord, visitor) &&
         visit_row_right_of(coord, visitor) &&
         visit_col_above(coord, visitor) && visit_col_below(coord, visitor);
}

inline bool
BasicBoard::visit_adjacent(Coord coord, CellVisitor auto && visitor) const {
  auto visit = [&](Coord coord) {
    if (int idx = get_flat_idx(coord); idx != -1) {
      return visit_cell(coord, idx, visitor);
    }
    return true;
  };

  auto [row, col] = coord;
  return visit({row + 1, col}) && visit({row - 1, col}) &&
         visit({row, col - 1}) && visit({row, col + 1});
}

inline std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {
  os << "Board: [\n\t";

  for (int i = 0, r = board.width_, e = r * board.height_; i < e; ++i) {
    auto state = board.cells_[i];
    os << to_char(state);
    if (--r == 0) {
      os << "\n\t";
      r = board.width_;
    }
  }
  os << "]";
  return os;
}

} // namespace model
