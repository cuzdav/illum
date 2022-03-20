#include "Tutorial.hpp"
#include "GuiTypes.hpp"
#include "olcPixelGameEngine.h"
#include "olcRetroMenu.hpp"
#include "utils/EnumUtils.hpp"
#include "picojson.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::literals;

std::string
load_file(std::string const & filename) {
  std::ifstream input(filename);
  if (not input.good()) {
    throw std::runtime_error("Unable to open file: " + filename);
  }
  return std::string{std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>()};
}

class JsonValidation {
public:
  JsonValidation(picojson::value const & top)
      : json_path_{{"outermost json"s, &top}} {
    // empty body
  }

  void
  enter(char const * name) {
    picojson::value const & next = cur().get(name);
    if (next.contains(name)) {
      json_path_.emplace_back(name, &next);
    }
    else {
      error("does not contain "s + name);
    }
  }

  void
  enter(size_t idx) {
    picojson::value const & next = cur().get(idx);
    json_path_.emplace_back(std::to_string(idx), &next);
  }

  void
  pop() {
    json_path_.pop_back();
  }

  void
  assert_object() {
    assert_json<picojson::value::object>("object");
  }
  void
  assert_array() {
    assert_json<picojson::value::array>("array");
  }
  void
  assert_string() {
    assert_json<std::string>("string");
  }
  void
  assert_double() {
    assert_json<double>("double");
  }
  void
  assert_bool() {
    assert_json<bool>("double");
  }
  void
  assert_int64() {
    assert_json<int64_t>("int64");
  }

private:
  struct CurFrame {
    std::string             name;
    picojson::value const * json;
  };

  std::vector<CurFrame> json_path_;

  std::string
  make_error_path() {
    std::string errmsg;
    for (bool first = true; auto const & frame : json_path_) {
      if (not first) {
        errmsg += '[';
      }
      errmsg += frame.name;
      if (not first) {
        errmsg += ']';
      }
      first = false;
    }
    errmsg += " ";
    return errmsg;
  }

  picojson::value const &
  cur() const {
    return *json_path_.back().json;
  }

  template <typename T>
  void
  assert_json(char const * type) {
    if (not cur().is<T>()) {
      error("is not a "s + type);
    }
  }

  void
  error(std::string const & msg) {
    throw std::runtime_error(make_error_path() + msg);
  }
};

picojson::value
load_and_validate_tutorial_json(char const * filename) {
  picojson::value result;
  auto            tutorial_levels_jsonstr = load_file("tutorial_levels.json");
  if (std::string errmsg = picojson::parse(result, tutorial_levels_jsonstr);
      not errmsg.empty()) {
    throw std::runtime_error("Invalid json in tutorial data:" + errmsg);
  }

  JsonValidation validator(result);
  validator.assert_object();
  return result;
}

Tutorial::Tutorial(olc::PixelGameEngine * engine)
    : levels_data_{std::make_unique<picojson::value>()}, game_engine_{engine} {
  *levels_data_ = load_and_validate_tutorial_json("tutorial_levels.json");
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
