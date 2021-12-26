#pragma once
#include "CellState.hpp"
#include "CellVisitorConcepts.hpp"
#include "Coord.hpp"
#include "utils/EnumUtils.hpp"

#include <array>
#include <compare>
#include <fmt/format.h>
#include <fmt/ostream.h>
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
  static int constexpr MAX_GRID_EDGE = Coord::MAX_GRID_EDGE;
  static int constexpr MAX_CELLS     = MAX_GRID_EDGE * MAX_GRID_EDGE;

  enum class VisitPolicy {
    DEFAULT               = 0,
    VISIT_START_COORD     = 1,
    SKIP_TERMINATING_WALL = 2,
  };
  friend auto
  operator+(VisitPolicy e) {
    return static_cast<int>(e);
  }
  friend VisitPolicy
  operator|(VisitPolicy v1, VisitPolicy v2) {
    return VisitPolicy(+v1 | +v2);
  }

  BasicBoard() = default;
  BasicBoard(int height, int width) { reset(height, width); }

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
  bool visit_adjacent(Coord coord, OptDirCellVisitor auto && visitor) const;

  // visit the single "left" and "right" cells of a coordinate, given a
  // direction. Still visited in absolute (to the board, not relative to your
  // direction) order: (UP,DOWN), or (LEFT,RIGHT)
  bool visit_adj_flank(Coord                     coord,
                       Direction                 dir,
                       OptDirCellVisitor auto && visitor) const;

  // visit all currently empty cells (unless prematurely stopped)
  bool visit_empty(CellVisitor auto && visitor) const;

  // will stop _after_ visiting a wall, or if visitor returns false
  // Does not visit coord unless VISIT_STARTING_COOED is passed in.
  bool visit_row_left_of(Coord                     coord,
                         OptDirCellVisitor auto && visitor,
                         VisitPolicy = VisitPolicy::DEFAULT) const;
  bool visit_row_right_of(Coord                     coord,
                          OptDirCellVisitor auto && visitor,
                          VisitPolicy = VisitPolicy::DEFAULT) const;
  bool visit_col_above(Coord                     coord,
                       OptDirCellVisitor auto && visitor,
                       VisitPolicy = VisitPolicy::DEFAULT) const;
  bool visit_col_below(Coord                     coord,
                       OptDirCellVisitor auto && visitor,
                       VisitPolicy = VisitPolicy::DEFAULT) const;

  void visit_perpendicular(Coord                     coord,
                           Direction                 dir,
                           OptDirCellVisitor auto && visitor) const;

  // visitor will always visit every direction. If it is a CellVisitorSome
  // and returns false, it will stop for the current direction, but still
  // visit other directions.
  void visit_rows_cols_outward(Coord                     coord,
                               OptDirCellVisitor auto && visitor,
                               Direction = directiongroups::all) const;

  friend std::ostream & operator<<(std::ostream &, BasicBoard const &);
  CellState             get_cell_flat_unchecked(int) const;
  OptCoord              get_last_move_coord() const;

  inline static int visit_cell_counter              = 0;
  inline static int visit_board_counter             = 0;
  inline static int visit_adjacent_counter          = 0;
  inline static int visit_adj_flank_counter         = 0;
  inline static int visit_empty_counter             = 0;
  inline static int visit_row_left_counter          = 0;
  inline static int visit_row_right_counter         = 0;
  inline static int visit_col_up_counter            = 0;
  inline static int visit_col_down_counter          = 0;
  inline static int visit_perp_counter              = 0;
  inline static int visit_rows_cols_outward_counter = 0;

  // emscripten does not support <=> on std::array<CellState, N>, so cannot use
  // default. This also avoids comparing the full array for boards that are
  // smaller than MAX width and height.
  auto friend
  operator<=>(BasicBoard const & lhs, BasicBoard const & rhs) {
    {
      auto width_result = lhs.width_ <=> rhs.width_;
      if (width_result != 0) {
        return width_result;
      }
    }
    {
      auto height_result = lhs.height_ <=> rhs.height_;
      if (height_result != 0) {
        return height_result;
      }
    }
    {
      auto last_move_result = lhs.last_move_coord_ <=> rhs.last_move_coord_;
      if (last_move_result != 0) {
        return last_move_result;
      }
    }

    for (int i = 0, stop = lhs.width_; i < stop;) {
      auto result = lhs.cells_[i] <=> rhs.cells_[i];
      if (result != 0) {
        return result;
      }
    }
    return std::strong_ordering::equal;
  }

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
      OptDirCellVisitor auto && visitor,
      VisitPolicy) const;

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
    cell = CellState::EMPTY;
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
BasicBoard::visit_adjacent(Coord                     coord,
                           OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_adjacent_counter);
  auto visit = [&](Coord coord) {
    if (int idx = get_flat_idx(coord); idx != -1) {
      return visit_cell(Direction::NONE, coord, idx, visitor);
    }
    return true;
  };

  auto [row, col] = coord;
  return visit({row - 1, col}) && visit({row, col - 1}) &&
         visit({row + 1, col}) && visit({row, col + 1});
}

inline bool
BasicBoard::visit_adj_flank(Coord                     coord,
                            Direction                 dir,
                            OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_adj_flank_counter);
  auto visit = [&](Direction dir, Coord coord) {
    if (int idx = get_flat_idx(coord); idx != -1) {
      return visit_cell(dir, coord, idx, visitor);
    }
    return true;
  };

  auto [row, col] = coord;
  if (dir == Direction::UP || dir == Direction::DOWN) {
    return visit(Direction::LEFT, {row, col - 1}) &&
           visit(Direction::RIGHT, {row, col + 1});
  }
  else {
    return visit(Direction::UP, {row - 1, col}) &&
           visit(Direction::DOWN, {row + 1, col});
  }
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
      if (not visit_cell(Direction::NONE, coord, i, visitor)) {
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
                                OptDirCellVisitor auto && visitor,
                                BasicBoard::VisitPolicy   visit_policy) const {
  // initial movement normally skips the starting point, since we are scanning
  // above, to the right of, etc. starting point. But it can optionally be
  // visited
  if ((+visit_policy & +VisitPolicy::VISIT_START_COORD) == 0) {
    update_coord(coord);
  }

  int idx = get_flat_idx(coord);
  if (idx < 0) {
    return true;
  }
  bool const should_visit_wall =
      (+visit_policy & +VisitPolicy::SKIP_TERMINATING_WALL) == 0;

  while (test_coord(coord)) {
    bool const hit_wall = is_wall(cells_[idx]);
    if (not hit_wall || should_visit_wall) {
      if (not visit_cell(dir, coord, idx, visitor)) {
        return false;
      }
    }
    if (hit_wall) {
      break;
    }
    // post iteration update.  Change coord, compute new flat index
    update_coord(coord);
    update_index(idx);
  }
  return true;
}

inline bool
BasicBoard::visit_row_left_of(Coord                     coord,
                              OptDirCellVisitor auto && visitor,
                              VisitPolicy               visit_policy) const {
  DEBUGPROFILE_INC_COUNTER(visit_row_left_counter);
  auto update_coord = [](Coord & coord) { --coord.col_; };
  auto update_idx   = [](int & idx) { --idx; };
  auto test_coord   = [](Coord coord) { return coord.col_ >= 0; };
  return visit_straight_line(Direction::LEFT,
                             coord,
                             update_coord,
                             update_idx,
                             test_coord,
                             visitor,
                             visit_policy);
}

inline bool
BasicBoard::visit_row_right_of(Coord                     coord,
                               OptDirCellVisitor auto && visitor,
                               VisitPolicy               visit_policy) const {
  DEBUGPROFILE_INC_COUNTER(visit_row_right_counter);
  auto update_coord = [](Coord & coord) { ++coord.col_; };
  auto update_idx   = [](int & idx) { ++idx; };
  auto test_coord   = [w = width_](Coord coord) { return coord.col_ < w; };
  return visit_straight_line(Direction::RIGHT,
                             coord,
                             update_coord,
                             update_idx,
                             test_coord,
                             visitor,
                             visit_policy);
}

inline bool
BasicBoard::visit_col_above(Coord                     coord,
                            OptDirCellVisitor auto && visitor,
                            VisitPolicy               visit_policy) const {
  DEBUGPROFILE_INC_COUNTER(visit_col_up_counter);
  auto update_coord = [](Coord & coord) { --coord.row_; };
  auto update_idx   = [w = width_](int & idx) { idx -= w; };
  auto test_coord   = [](Coord coord) { return coord.row_ >= 0; };
  return visit_straight_line(Direction::UP,
                             coord,
                             update_coord,
                             update_idx,
                             test_coord,
                             visitor,
                             visit_policy);
}

inline bool
BasicBoard::visit_col_below(Coord                     coord,
                            OptDirCellVisitor auto && visitor,
                            VisitPolicy               visit_policy) const {
  DEBUGPROFILE_INC_COUNTER(visit_col_down_counter);
  auto update_coord = [](Coord & coord) { ++coord.row_; };
  auto update_idx   = [w = width_](int & idx) { idx += w; };
  auto test_coord   = [h = height_](Coord coord) { return coord.row_ < h; };
  return visit_straight_line(Direction::DOWN,
                             coord,
                             update_coord,
                             update_idx,
                             test_coord,
                             visitor,
                             visit_policy);
}

inline void
BasicBoard::visit_rows_cols_outward(Coord                     coord,
                                    OptDirCellVisitor auto && visitor,
                                    Direction directions) const {
  DEBUGPROFILE_INC_COUNTER(visit_rows_cols_outward_counter);
  if (contains_all(directions, Direction::LEFT)) {
    visit_row_left_of(coord, visitor);
  }
  if (contains_all(directions, Direction::RIGHT)) {
    visit_row_right_of(coord, visitor);
  }
  if (contains_all(directions, Direction::UP)) {
    visit_col_above(coord, visitor);
  }
  if (contains_all(directions, Direction::DOWN)) {
    visit_col_below(coord, visitor);
  }
}

// Provide the direction you are scanning the board, and a coordinate, and it
// will visit the crossing row or column. That is, if you're going left/right,
// along a row, it'll visit the vertical column that goes above and below you.
// If you're going up/down along a column, it'll visit the row you're on. (It
// will NOT visit your starting cell). If you use a Directed visitor, it'll
// give you the direction the scan is moving on the board in absolute
// perspective, relative to a fixed camera looking at the board. Your
// direction is not considered. That is, lower-numbered columns are always
// "LEFT" of higher numbered columns, and higher columns are always to the
// RIGHT of lower-numbered columns. Similarly lower-numbered rows are UP, and
// higher numbered rows are DOWN.
inline void
BasicBoard::visit_perpendicular(Coord                     coord,
                                Direction                 dir,
                                OptDirCellVisitor auto && visitor) const {
  DEBUGPROFILE_INC_COUNTER(visit_perp_counter);
  switch (dir) {
    case Direction::LEFT:
    case Direction::RIGHT:
      visit_col_above(coord, visitor);
      visit_col_below(coord, visitor);
      break;

    case Direction::UP:
    case Direction::DOWN:
      visit_row_left_of(coord, visitor);
      visit_row_right_of(coord, visitor);
      break;

    case Direction::NONE:
      break;
  }
}

} // namespace model

template <>
struct fmt::formatter<::model::BasicBoard> {

  constexpr static char const * const indent = "    ";

  template <typename ParseContext>
  constexpr auto
  parse(ParseContext & ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto
  format(::model::BasicBoard const & board, FormatContext & ctx) {
    fmt::format_to(ctx.out(), "Board:\n");
    const int cell_width = 1 + (board.width() > 9);

    // print numbers across top
    fmt::format_to(ctx.out(), "{}{:{}}  ", indent, "", cell_width + 1);
    for (int i = 0; i < board.width(); ++i) {
      fmt::format_to(ctx.out(), "{:^{}}", i, cell_width + 2);
    }
    fmt::format_to(ctx.out(), "\n");

    // print the dashed line
    fmt::format_to(ctx.out(), "{}{: <{}}", indent, "", cell_width + 1);
    fmt::format_to(
        ctx.out(), "{:-<{}}", "", (cell_width + 2) * board.width() + 1);
    fmt::format_to(ctx.out(), "\n");

    for (int i = 0, r = board.width(), e = r * board.height(); i < e; ++i) {
      auto state = board.get_cell_flat_unchecked(i);
      if (r == board.width()) {

        // Print the row number on left
        fmt::format_to(
            ctx.out(), "{} {:{}}| ", indent, i / board.width(), cell_width);
      }

      if (model::Coord coord{i / board.width(), i % board.width()};
          board.get_last_move_coord() &&
          (board.get_last_move_coord() == coord)) {
        // print the cell with "last move" marker around it
        fmt::format_to(ctx.out(), "[{:{}c}]", to_char(state), cell_width);
      }
      else {
        // print the cell as normal
        fmt::format_to(ctx.out(), " {:{}c} ", to_char(state), cell_width);
      }
      if (--r == 0) {
        fmt::format_to(ctx.out(), "\n");
        r = board.width();
      }
    }
    return fmt::format_to(ctx.out(), "{}", indent);
  }
};

//  LocalWords:  OptDirCellVisitor
