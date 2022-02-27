#pragma once

#include "GuiTypes.hpp"
#include <memory>

namespace picojson {
class value;
}

namespace olc::popup {
class Menu;
}

class Tutorial {
public:
  Tutorial();
  ~Tutorial();

  // called inside the game update loop while a tutorial is running
  bool update_game(float fElapsedTime);

  // current action in tutorial is complete, move on to next. (Usually user
  // clicked something to say "next")
  bool advance_section();

  // base id is the offset in the total menu where tutorial submenu starts.
  // All tutorialIds are offsets from this base.
  void initialize(int base_id, olc::popup::Menu & tutorial_submenu);

  // set the state and board_generator to proper tutorial level
  void update_menu(int selection, BoardGenerator & board_generator);

private:
  std::unique_ptr<picojson::value> levels_data_;
};
