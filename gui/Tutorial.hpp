#pragma once

#include "GuiTypes.hpp"
#include "TutorialLevels.hpp"
#include <memory>
#include <optional>

namespace olc {
class PixelGameEngine;

namespace popup {
class Menu;
} // namespace popup

} // namespace olc

namespace picojson {
class value;
}

class Tutorial {
public:
  Tutorial(olc::PixelGameEngine *);
  ~Tutorial();

  // called inside the game update loop while a tutorial is running
  bool update_gamestate(float fElapsedTime);

  // base id is the offset in the total menu where tutorial submenu starts.
  // All tutorialIds are offsets from this base.
  void initialize(int base_id, olc::popup::Menu & tutorial_submenu);

  // create a board generator (implemented as TutorialLevels) and setup state
  // for this tutorial in response to menu selection
  TutorialLevels handle_menu_selection(int selection);

private:
  std::optional<TutorialLevels>    tutorial_levels_;
  std::unique_ptr<picojson::value> levels_data_;
  olc::PixelGameEngine *           game_engine_;
};
