#pragma once
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include <array>
#include <fmt/format.h>
#include <iosfwd>
#include <optional>
#include <stdexcept>

#ifdef DEBUGPROFILE
#define DEBUGPROFILE_INC_COUNTER(COUNTER) ++COUNTER
#else
#define DEBUGPROFILE_INC_COUNTER(COUNTER)
#endif

namespace model {

class BasicBoard {

public:
  static int constexpr MAX_GRID_EDGE = 21;
  static int constexpr MAX_CELLS     = MAX_GRID_EDGE * MAX_GRID_EDGE;

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
  bool visit_board_if(CellVisitor auto &&        visitor,
                      CellVisitPredicate auto && should_visit_pred) const;

  // visit the immediately adjacent squares around coord, guaranteed in
  // counter-clockwise order: UP, LEFT, DOWN, RIGHT
  bool visit_adjacent(Coord coord, CellVisitor auto && visitor) const;

  // visit the single "left" and "right" cells of a coordinate, given a
  // direction. Still visited in absolute (to the board, not relative to your
  // direction) order: (UP,DOWN), or (LEFT,RIGHT)
  bool visit_adj_flank(Coord               coord,
                       Direction           dir,
                       CellVisitor auto && visitor) const;

  // visit the (diagonal) corners of wall, guaranteed in counter-clockwise
  // order: Quadrant I, II, III, IV
  bool visit_adj_corners(Coord coord, CellVisitor auto && visitor) const;
  bool visit_empty(CellVisitor auto && visitor) const;

  // will stop _after_ visiting a wall, or if visitor returns false
  bool visit_row_left_of(Coord coord, OptDirCellVisitor auto && visitor) const;
  bool visit_row_right_of(Coord coord, OptDirCellVisitor auto && visitor) const;
  bool visit_col_above(Coord coord, OptDirCellVisitor auto && visitor) const;
  bool visit_col_below(Coord coord, OptDirCellVisitor auto && visitor) const;

  void visit_perpendicular(Coord                     coord,
                           Direction                 dir,
                           OptDirCellVisitor auto && visitor) const;

  // visit upwards and downwards from given coord, starting each direction
  // adjacent to coord, going outward.
  void visit_row_outward(Coord coord, OptDirCellVisitor auto && visitor) const;
  // visit left and right from given coord, starting each direction
  // adjacent to coord, going outward.
  void visit_col_outward(Coord coord, OptDirCellVisitor auto && visitor) const;

  // visitor will always visit every direction.  Essentially combines
  // visit_row_outward and visit_col_outward
  void visit_rows_cols_outward(Coord                     coord,
                               OptDirCellVisitor auto && visitor) const;

  friend std::ostream & operator<<(std::ostream &, BasicBoard const &);
  CellState             get_cell_flat_unchecked(int) const;
  OptCoord              get_last_move_coord() const;

  auto operator<=>(BasicBoard const &) const = default;

  inline static int visit_cell_counter              = 0;
  inline static int visit_board_counter             = 0;
  inline static int visit_adjacent_counter          = 0;
  inline static int visit_adj_flank_counter         = 0;
  inline static int visit_adj_corners_counter       = 0;
  inline static int visit_empty_counter             = 0;
  inline static int visit_row_left_counter          = 0;
  inline static int visit_row_right_counter         = 0;
  inline static int visit_col_up_counter            = 0;
  inline static int visit_col_down_counter          = 0;
  inline static int visit_perp_counter              = 0;
  inline static int visit_rows_cols_outward_counter = 0;
  inline static int visit_row_outward_counter       = 0;
  inline static int visit_col_outward_counter       = 0;

private:
  int get_flat_idx(Coord coord) const;
  int get_flat_idx_unchecked(Coord coord) const;

  bool
  visit_cell(Direction, Coord, int idx, CellVisitorSome auto && visitor) const;

  bool
  visit_cell(Direction, Coord, int idx, CellVisitorAll auto && visitor) const;

  bool visit_cell(Direction                       direction,
                  Coord                           coord,
                  int                             idx,
                  DirectedCellVisitorSome auto && visitor) const;

  bool visit_cell(Direction                      direction,
                  Coord                          coord,
                  int                            idx,
                  DirectedCellVisitorAll auto && visitor) const;

  // a building block for all the visit_(row|col) variations to be assembled
  bool visit_straight_line(
      Direction dir,
      Coord     coord,
      auto &&   update_coord, // modify row or col by one
      auto &&   update_index, // +/- 1 for left/right +/- width for up/down
      auto &&   test_coord, // check if reached end of range (0 or height/width)
      OptDirCellVisitor auto && visitor) const;

private:
  int                              height_ = 0;
  int                              width_  = 0;
  std::array<CellState, MAX_CELLS> cells_;
  OptCoord                         last_move_coord_;
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
  assert(height > 0);
  assert(width > 0);
  for (auto & cell : cells_) {
    cell = CellState::Empty;
  }
  height_ = height;
  width_  = width;
  if (height_ < 0 || width_ < 0 || height_ * width_ >= cells_.size()) {
    throw std::runtime_error("Invalid dimensions");
  }
  last_move_coord_.reset();
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

inline CellState
BasicBoard::get_cell_flat_unchecked(int idx) const {
  return cells_[idx];
}

inline OptCoord
BasicBoard::get_last_move_coord() const {
  return last_move_coord_;
}

inline bool
BasicBoard::set_cell(Coord coord, CellState state) {
  if (auto idx = get_flat_idx(coord); idx != -1) {
    cells_[idx] = state;
    if (is_playable(state)) {
      last_move_coord_ = coord;
    }
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
BasicBoard::visit_cell(Direction               direction,
                       Coord                   coord,
                       int                     i,
                       CellVisitorSome auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_cell_counter);
  assert(get_flat_idx(coord) == i);
  return visitor(coord, cells_[i]) == model::KEEP_VISITING;
}

inline bool
BasicBoard::visit_cell(Direction              direction,
                       Coord                  coord,
                       int                    i,
                       CellVisitorAll auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_cell_counter);
  assert(get_flat_idx(coord) == i);
  visitor(coord, cells_[i]);
  return true;
}

inline bool
BasicBoard::visit_cell(Direction                       direction,
                       Coord                           coord,
                       int                             i,
                       DirectedCellVisitorSome auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_cell_counter);
  assert(get_flat_idx(coord) == i);
  return visitor(direction, coord, cells_[i]) == KEEP_VISITING;
}

inline bool
BasicBoard::visit_cell(Direction                      direction,
                       Coord                          coord,
                       int                            i,
                       DirectedCellVisitorAll auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_cell_counter);
  assert(get_flat_idx(coord) == i);
  visitor(direction, coord, cells_[i]);
  return true;
}

inline bool
BasicBoard::visit_adjacent(Coord coord, CellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_adjacent_counter);
  auto visit = [&](Coord coord) {
    if (int idx = get_flat_idx(coord); idx != -1) {
      return visit_cell(Direction::None, coord, idx, visitor);
    }
    return true;
  };

  auto [row, col] = coord;
  return visit({row - 1, col}) && visit({row, col - 1}) &&
         visit({row + 1, col}) && visit({row, col + 1});
}

inline bool
BasicBoard::visit_adj_flank(Coord               coord,
                            Direction           dir,
                            CellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_adj_flank_counter);
  auto visit = [&](Coord coord) {
    if (int idx = get_flat_idx(coord); idx != -1) {
      return visit_cell(Direction::None, coord, idx, visitor);
    }
    return true;
  };

  auto [row, col] = coord;
  if (dir == Direction::Up || dir == Direction::Down) {
    return visit({row, col - 1}) && visit({row, col + 1});
  }
  else {
    return visit({row - 1, col}) && visit({row + 1, col});
  }
}

inline bool
BasicBoard::visit_adj_corners(Coord coord, CellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_adj_corners_counter);
  auto visit = [&](Coord coord) {
    if (int idx = get_flat_idx(coord); idx != -1) {
      return visit_cell(Direction::None, coord, idx, visitor);
    }
    return true;
  };

  auto [row, col] = coord;
  return visit({row - 1, col + 1}) && visit({row - 1, col - 1}) &&
         visit({row + 1, col - 1}) && visit({row + 1, col + 1});
}

inline bool
BasicBoard::visit_empty(CellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_empty_counter);
  return visit_board_if(visitor,
                        [](auto, CellState cell) { return is_empty(cell); });
}

inline bool
BasicBoard::visit_board_if(CellVisitor auto &&        visitor,
                           CellVisitPredicate auto && visit_cell_pred) const {
  DEBUGPROFILE_INC_COUNTER(visit_board_counter);
  for (int r = 0, c = 0, i = 0; r < height_; ++i) {
    Coord coord{r, c};
    if (visit_cell_pred(coord, cells_[i])) {
      if (not visit_cell(Direction::None, coord, i, visitor)) {
        return false;
      }
    }
    if (++c == width_) {
      c = 0;
      ++r;
    }
  }
  return true;
}

inline bool
BasicBoard::visit_board(CellVisitor auto && visitor) const {
  return visit_board_if(visitor, [](auto, auto) { return true; });
}

inline bool
BasicBoard::visit_straight_line(Direction                 dir,
                                Coord                     coord,
                                auto &&                   update_coord,
                                auto &&                   update_index,
                                auto &&                   test_coord,
                                OptDirCellVisitor auto && visitor) const {
  // initial movement, since we are scanning above, to the right of, etc., the
  // starting point.
  update_coord(coord);

  int idx = get_flat_idx(coord);
  if (idx >= 0) {
    // reached end of line?
    while (test_coord(coord)) {
      if (not visit_cell(dir, coord, idx, visitor)) {
        return false;
      }
      if (is_wall(cells_[idx])) {
        break;
      }
      // post iteration update.  Change coord, compute new flat index
      update_coord(coord);
      update_index(idx);
    }
  }
  return true;
}

inline bool
BasicBoard::visit_row_left_of(Coord                     coord,
                              OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_row_left_counter);
  auto update_coord = [](Coord & coord) { --coord.col_; };
  auto update_idx   = [](int & idx) { --idx; };
  auto test_coord   = [](Coord coord) { return coord.col_ >= 0; };
  return visit_straight_line(
      Direction::Left, coord, update_coord, update_idx, test_coord, visitor);
}

inline bool
BasicBoard::visit_row_right_of(Coord                     coord,
                               OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_row_right_counter);
  auto update_coord = [](Coord & coord) { ++coord.col_; };
  auto update_idx   = [](int & idx) { ++idx; };
  auto test_coord   = [w = width_](Coord coord) { return coord.col_ < w; };
  return visit_straight_line(
      Direction::Right, coord, update_coord, update_idx, test_coord, visitor);
}

inline bool
BasicBoard::visit_col_above(Coord                     coord,
                            OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_col_up_counter);
  auto update_coord = [](Coord & coord) { --coord.row_; };
  auto update_idx   = [w = width_](int & idx) { idx -= w; };
  auto test_coord   = [](Coord coord) { return coord.row_ >= 0; };
  return visit_straight_line(
      Direction::Up, coord, update_coord, update_idx, test_coord, visitor);
}

inline bool
BasicBoard::visit_col_below(Coord                     coord,
                            OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_col_down_counter);
  auto update_coord = [](Coord & coord) { ++coord.row_; };
  auto update_idx   = [w = width_](int & idx) { idx += w; };
  auto test_coord   = [h = height_](Coord coord) { return coord.row_ < h; };
  return visit_straight_line(
      Direction::Down, coord, update_coord, update_idx, test_coord, visitor);
}

inline void
BasicBoard::visit_row_outward(Coord                     coord,
                              OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_row_outward_counter);
  visit_row_left_of(coord, visitor);
  visit_row_right_of(coord, visitor);
}

inline void
BasicBoard::visit_col_outward(Coord                     coord,
                              OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_col_outward_counter);
  visit_col_above(coord, visitor);
  visit_col_below(coord, visitor);
}

inline void
BasicBoard::visit_rows_cols_outward(Coord                     coord,
                                    OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_rows_cols_outward_counter);
  visit_row_outward(coord, visitor);
  visit_col_outward(coord, visitor);
}

// Provide the direction you are scanning the board, and a coordinate, and it
// will visit the crossing row or column. That is, if you're going left/right,
// along a row, it'll visit the vertical column that goes above and below you.
// If you're going up/down along a column, it'll visit the row you're on. (It
// will NOT visit your starting cell). If you use a Directed visitor, it'll give
// you the direction the scan is moving on the board in absolute perspective,
// relative to a fixed camera looking at the board. Your direction is not
// considered. That is, lower-numbered columns are always "Left" of higher
// numbered columns, and higher columns are always to the Right of
// lower-numbered columns. Similarly lower-numbered rows are Up, and higher
// numbered rows are Down.
inline void
BasicBoard::visit_perpendicular(Coord                     coord,
                                Direction                 dir,
                                OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_perp_counter);
  switch (dir) {
    case Direction::Left:
    case Direction::Right:
      visit_col_above(coord, visitor);
      visit_col_below(coord, visitor);
      break;

    case Direction::Up:
    case Direction::Down:
      visit_row_left_of(coord, visitor);
      visit_row_right_of(coord, visitor);
      break;

    case Direction::None:
      break;
  }
}

} // namespace model

template <>
struct fmt::formatter<::model::BasicBoard>;
