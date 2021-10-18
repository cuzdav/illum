#include "Serialize.hpp"
#include "BoardModel.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <string_view>

namespace model::serialize {

using namespace std::literals;

using enum Action;
using enum CellState;

std::string
to_string(model::Move const & move) {
  return fmt::format("{},{},{},{}",
                     to_string(move.action_),
                     to_string(move.state_),
                     move.row_,
                     move.col_);
}

// expecting a 4-tuple string "<action>,<state>,<row>,<col>"
model::Move
get_move_from_string(std::string const & str) {
  if (not str.empty()) {

    auto tok = [&str, view = std::string_view{str}]() mutable {
      auto off = view.find(',');
      if (off == std::string_view::npos) {
        return view;
      }
      std::string_view result(view.begin(), view.begin() + off);
      view.remove_prefix(off + 1);
      return result;
    };

    auto action = tok();
    auto state  = tok();
    auto row    = tok();
    auto col    = tok();

    return Move{get_action_from_string(action),
                get_state_from_string(state),
                std::atoi(row.data()),
                std::atoi(col.data())};
  }
  throw std::runtime_error("invalid move string: " +
                           std::string(str.data(), str.size()));
}

// convert a given model to json
pt::ptree
to_ptree(BoardModel const & model) {
  pt::ptree tree;
  pt::ptree children;
  model.for_each_move([&](int, Move const & move) {
    children.push_back(std::pair("", pt::ptree(to_string(move))));
  });
  tree.add_child("moves", children);
  return tree;
}

// restore a level or in-progress game model from array of json moves
void
setup_model_from_ptree(pt::ptree & moves, BoardModel & model) {
  bool has_reset   = false;
  bool has_started = false;
  for (auto & move : moves.get_child("moves")) {
    std::string move_str = move.second.get<std::string>("");

    auto [action, state, row, col] = get_move_from_string(move_str);

    switch (action) {
    case Add: model.add(state, row, col); break;
    case Remove: model.remove(row, col); break;
    case StartGame: model.start_game(); break;
    case ResetGame:
      model.reset_game(row, col); // height, width
      break;
    default: throw "Invalid action in from_json";
    }
  }
}

void
setup_model_from_json_file(std::string const & filename, BoardModel & model) {
  pt::ptree tree;
  pt::read_json(filename, tree);
  setup_model_from_ptree(tree, model);
}

void
to_json_file(std::string const & filename, pt::ptree & tree) {
  pt::write_json(filename, tree);
}

void
to_json_file(std::string const & filename, BoardModel const & model) {
  auto tree = to_ptree(model);
  to_json_file(filename, tree);
}

} // namespace model::serialize
