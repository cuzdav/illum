#pragma once
#include "Coord.hpp"

namespace levels {

class Adjacents {
public:
  using iterator  = model::Coord *;
  using size_type = int;

  iterator
  begin() {
    return adjacents_;
  }

  iterator
  end() {
    return adjacents_ + size_;
  }

  size_type
  size() {
    return size_;
  }

  model::Coord
  back() {
    assert(size_ > 0);
    return adjacents_[size_ - 1];
  }

  bool
  remove(model::Coord coord) {
    if (auto iter = std::find(begin(), end(), coord); iter != end()) {
      *iter = back();
      size_--;
      return true;
    }
    return false;
  }

  bool
  push_back(model::Coord coord) {
    if (size_ == 4) {
      return false;
    }
    if (auto iter = std::find(begin(), end(), coord); iter != end()) {
      return false;
    }
    adjacents_[size_++] = coord;
    return true;
  }

  bool
  pop_back() {
    if (size_ == 0) {
      return false;
    }
    size_--;
    return true;
  }

  model::Coord &
  operator[](int idx) {
    assert(idx >= 0 && idx < 4);
    return adjacents_[idx];
  }

private:
  int          size_ = 0;
  model::Coord adjacents_[4];
};

} // namespace levels
