#include "Tutorial.hpp"
#include "GuiTypes.hpp"
#include "olcPixelGameEngine.h"
#include "olcRetroMenu.hpp"
#include "utils/EnumUtils.hpp"
#include "utils/jsonschema.hpp"
#include "utils/picojson.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

std::string
load_file(std::string const & filename) {
  std::ifstream input(filename);
  if (not input.good()) {
    throw std::runtime_error("Unable to open file: " + filename);
  }
  return std::string{std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>()};
}

Tutorial::Tutorial(olc::PixelGameEngine * engine)
    : levels_data_{std::make_unique<picojson::value>()}, game_engine_{engine} {
  *levels_data_ = load_and_validate_json("tutorial_levels.json",
                                         "tutorial_levels_schema.json");
  // auto tutorial_levels_jsonstr = load_file("tutorial_levels.json");
  // if (std::string errmsg =
  //         picojson::parse(*levels_data_, tutorial_levels_jsonstr);
  //     not errmsg.empty()) {
  //   throw std::runtime_error("Invalid json in tutorial data:" + errmsg);
  // }
}

// unique_ptr member declared with an incomplete type in header,
// so this must be defined here, now that "held type" is complete.
Tutorial::~Tutorial() = default;

bool
Tutorial::update_gamestate(float fElapsedTime) {
  if (game_engine_->GetMouse(olc::Mouse::LEFT).bPressed ||
      (game_engine_->GetMouse(olc::Mouse::RIGHT).bPressed)) {
    tutorial_levels_->advance_section();
  }

  return true;
}

void
Tutorial::initialize(int base_id, olc::popup::Menu & tutorial_menu) {
  tutorial_menu.SetTable(1, 4);
  tutorial_menu["Rules"].SetID(+TutorialId::RULES + base_id);
  tutorial_menu["Trivial moves"].SetID(+TutorialId::TRIVIAL_MOVES + base_id);
  tutorial_menu["Easy moves"].SetID(+TutorialId::EASY_MOVES + base_id);
  tutorial_menu["Speculative moves"].SetID(+TutorialId::SPECULATIVE_MOVES +
                                           base_id);
}

TutorialLevels
Tutorial::handle_menu_selection(int selection) {
  // A tutorial entry was selected.  Switch to tutorial handlers, etc.

  TutorialId tutorial_id{selection};
  std::cout << "DBG: tutorial update_menu, selection = " << selection
            << std::endl;
  tutorial_levels_.emplace(tutorial_id, levels_data_.get());
  return *tutorial_levels_;
}
