#include "BoardModel.hpp"
#include "CellState.hpp"
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace model::test {

// I'd like to just chain calls together to create the level row-wise, but it
// does not look good in clang-format and I don't want to have to disable
// formatting to get a nice look. So one row per statement makes for vertical
// alignment without messing up the rest of the formatting rules. :)

class LevelCreator {
public:
  LevelCreator(BoardModel * model) : model_(model) {}

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
  finished() {
    model_->reset_game(size(unparsed_rows_), width_);

    using enum CellState;
    int rownum = 0;
    for (auto const & row : unparsed_rows_) {
      int colnum = 0;
      for (char c : row) {
        CellState cell;
        switch (c) {
        case chr::Wall0: cell = Wall0; break;
        case chr::Wall1: cell = Wall1; break;
        case chr::Wall2: cell = Wall2; break;
        case chr::Wall3: cell = Wall3; break;
        case chr::Wall4: cell = Wall4; break;
        case chr::Empty: cell = Empty; break;
        case chr::Bulb: cell = Bulb; break;
        case chr::Mark: cell = Mark; break;
        default:
          throw std::runtime_error(
              (std::string("Unknown cell char: ") + c).c_str());
        }
        model_->add(cell, rownum, colnum++);
      }
      rownum++;
    }
    model_->start_game();
  }

private:
  int                      width_ = 0;
  BoardModel *             model_ = nullptr;
  std::vector<std::string> unparsed_rows_;
};

} // namespace model::test
