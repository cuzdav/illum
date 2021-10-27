#pragma once
#include "CellState.hpp"
#include <array>
#include <iostream>
#include <stdexcept>

namespace model {

class BasicBoard {
public:
  static int constexpr MAX_CELLS = 625;

  bool operator==(BasicBoard const & other) const;

  void reset(int height, int width);
  bool is_initialized() const;

  CellState get_cell(int row, int col) const;

  bool set_cell(int row, int col, CellState state);

  template <typename PredT>
  bool set_cell_if(int row, int col, CellState state, PredT pred);

  int width() const;
  int height() const;

  template <typename VisitorT>
  void visit_cells(VisitorT && visitor) const;

  friend std::ostream & operator<<(std::ostream &, BasicBoard const &);

private:
  int get_flat_idx(int row, int col) const;

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
BasicBoard::get_cell(int row, int col) const {
  return cells_[get_flat_idx(row, col)];
}

inline bool
BasicBoard::set_cell(int row, int col, CellState state) {
  if (auto idx = get_flat_idx(row, col); idx != -1) {
    cells_[idx] = state;
    return true;
  }
  return false;
}

template <typename PredT>
bool
BasicBoard::set_cell_if(int row, int col, CellState state, PredT pred) {
  if (auto idx = get_flat_idx(row, col); idx > -1 && pred(cells_[idx])) {
    cells_[idx] = state;
    return true;
  }
  return false;
}

inline int
BasicBoard::get_flat_idx(int row, int col) const {
  if (int idx = row * width_ + col; idx < cells_.size() && idx >= 0) {
    return idx;
  }
  throw std::range_error("Out of bounds");
}

inline int
BasicBoard::width() const {
  return width_;
}

inline int
BasicBoard::height() const {
  return height_;
}

template <typename VisitorT>
void
BasicBoard::visit_cells(VisitorT && visitor) const {
  for (int r = 0, c = 0, i = 0; r < height_; ++i) {
    visitor.accept(r, c, cells_[i]);
    if (++c == width_) {
      c = 0;
      ++r;
    }
  }
}

inline std::ostream &
operator<<(std::ostream & os, BasicBoard const & board) {
  os << "Board: [\n\t";
  int i = board.width_;
  for (CellState state : board.cells_) {
    os << to_char(state);
    if (--i == 0) {
      os << "\n\t";
      i = board.width_;
    }
  }
  os << "]";
  return os;
}

} // namespace model
