#include "BoardModel.hpp"
#include "CellState.hpp"
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream>

namespace model::test {

// I'd like to just chain calls together to create the level row-wise, but it
// does not look good in clang-format and I don't want to have to disable
// formatting to get a nice look. So one row per statement makes for vertical
// alignment without messing up the rest of the formatting rules. :)

class ASCIILevelCreator {
public:
  enum class StartPolicy { CallStart, DontCallStart };

  void
  operator()(std::string const & row) {
    if (unparsed_rows_.empty()) {
      width_ = row.size();
    }
    else if (row.size() != width_) {
      throw std::out_of_range("All strings must be the same width of " +
                              std::to_string(width_));
    }
    unparsed_rows_.push_back(row);
  }

  void
  finished(BoardModel * model,
           StartPolicy  start_policy = StartPolicy::CallStart) {
    model->reset_game(size(unparsed_rows_), width_);
    finished_impl([model](CellState cell, Coord coord) {
      if (cell != CellState::Empty) {
        model->add(cell, coord);
      }
    });
    if (start_policy == StartPolicy::CallStart) {
      model->start_game();
    }
  }

  void
  finished(BasicBoard * board) {
    board->reset(size(unparsed_rows_), width_);
    finished_impl([board](CellState cell, Coord coord) {
      if (cell != CellState::Empty) {
        board->set_cell(coord, cell);
      }
    });
  }

private:
  void
  finished_impl(auto && cell_handler) {
    int rownum = 0;
    for (auto const & row : unparsed_rows_) {
      int colnum = 0;
      for (char c : row) {
        CellState cell = get_state_from_char(c);
        cell_handler(cell, Coord{rownum, colnum++});
      }
      rownum++;
    }
  }

private:
  int                      width_ = 0;
  std::vector<std::string> unparsed_rows_;
};

} // namespace model::test
