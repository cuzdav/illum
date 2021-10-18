#include "BoardModel.hpp"
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace model::test {

class LevelCreator {
public:
  LevelCreator(BoardModel * model) : model_(model) {}

  LevelCreator &
  operator()(std::string const & row) {
    if (unparsed_rows_.empty()) {
      width_ = row.size();
    }
    else if (row.size() != width_) {
      throw std::out_of_range("All strings must be the same width of " +
                              std::to_string(width_));
    }
    unparsed_rows_.push_back(row);
    return *this;
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
        case '0': cell = Wall0; break;
        case '1': cell = Wall1; break;
        case '2': cell = Wall2; break;
        case '3': cell = Wall3; break;
        case '4': cell = Wall4; break;
        case ' ': cell = Empty; break;
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
