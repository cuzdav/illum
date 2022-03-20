#include "BoardModel.hpp"
#include "GuiTypes.hpp"
#include "picojson.h"

/*
 * This class applies a sequence of "levels" as described in json, with
 * a sequence of actions to apply.
 *
 * It is registered as a handler for the "create board" callback. Operator()
 * creates the next tutorial level, rather than a random level, as normal play
 * uses.
 */
class TutorialLevels {
public:
  TutorialLevels(TutorialId start_at, picojson::value const * level_json);

  // create board / tutorial section handler
  model::BoardModel operator()();

  // current action in tutorial is complete, move on to next. (Usually user
  // clicked something to say "next"). For a given board, we may have a chain of
  // instructions or examples, this moves through them, one at a time.
  bool advance_section();

private:
  picojson::value levels_data_;
  std::size_t     level_index_;
};
