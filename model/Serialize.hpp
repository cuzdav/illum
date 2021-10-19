#pragma once
#include "Action.hpp"
#include "BoardModel.hpp"
#include "CellState.hpp"

#include <boost/property_tree/ptree.hpp>

namespace model::serialize {

namespace pt = boost::property_tree;

// save a game state (level only, or in-progress game) as property tree
pt::ptree to_ptree(BoardModel const & model);

// restore a level or in-progress game model from boost ptree
void setup_model_from_ptree(pt::ptree & moves, BoardModel & model);

// store/recreate a boardmodel as a json file
void to_json_stream(std::ostream & ostr, BoardModel const & model);
void to_json_stream(std::ostream & ostr, pt::ptree const & tree);

void setup_model_from_json_stream(std::istream & istr, BoardModel & model);

} // namespace model::serialize
