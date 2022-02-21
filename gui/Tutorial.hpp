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

  void initialize(int base_id, olc::popup::Menu & tutorial_submenu);

  // set the state and board_generator to proper tutorial level
  void update_menu(int selection, BoardGenerator & board_generator);

private:
  std::unique_ptr<picojson::value> levels_;
};
