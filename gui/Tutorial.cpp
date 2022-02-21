#include "Tutorial.hpp"
#include "GuiTypes.hpp"
#include "olcRetroMenu.hpp"
#include "utils/EnumUtils.hpp"
#include "utils/picojson.hpp"
#include <fstream>
#include <stdexcept>

enum class TutorialId { RULES, TRIVIAL_MOVES, EASY_MOVES, SPECULATIVE_MOVES };

static constexpr char const * SECTION_NAME = "section_name";

std::string
to_string(TutorialId id) {
  switch (id) {
    case TutorialId::RULES:
      return "RULES";
    case TutorialId::TRIVIAL_MOVES:
      return "TRIVIAL_MOVES";
    case TutorialId::EASY_MOVES:
      return "EASY_MOVES";
    case TutorialId::SPECULATIVE_MOVES:
      return "SPECULATIVE_MOVES";
    default:
      throw std::runtime_error("Unknown TutorialId");
  }
}

class TutorialLevels {
public:
  TutorialLevels(TutorialId start_at, picojson::value const * level_json)
      : levels_{*level_json}, level_index_{0} {
    assert(levels_.contains("name"));
    assert(levels_.contains("levels"));

    using array = picojson::value::array;
    if (not levels_.is<array>()) {
      throw std::runtime_error(
          "Error in tutorial json: \"levels\" is expected to be an array");
    }
    auto const & levels  = levels_.get<array>();
    std::string  section = to_string(start_at);
    for (auto const & obj : levels) {
      if (obj.contains(SECTION_NAME) &&
          obj.get(SECTION_NAME).to_str() == section) {
        return;
      }
      ++level_index_;
    }
    throw std::runtime_error("No tutorial json section named '" + section +
                             "'");
  }

  model::BoardModel
  operator()() const {
    std::cout << "tutorial levels called" << std::endl;
    return model::BoardModel{};
  }

private:
  picojson::value levels_;
  int             level_index_;
};

std::string
load_file(std::string const & filename) {
  std::ifstream input(filename);
  if (not input.good()) {
    throw std::runtime_error("Unable to open file: " + filename);
  }
  return std::string{std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>()};
}

Tutorial::Tutorial() : levels_{std::make_unique<picojson::value>()} {

  auto tutorial_levels_jsonstr = load_file("tutorial_levels.json");
  if (std::string errmsg = picojson::parse(*levels_, tutorial_levels_jsonstr);
      not errmsg.empty()) {
    throw std::runtime_error("Invalid json in tutorial data:" + errmsg);
  }
}

Tutorial::~Tutorial() = default;

void
setup_menu(int base_id, olc::popup::Menu & tutorial_menu) {
  tutorial_menu.SetTable(1, 4);
  tutorial_menu["Rules"].SetID(+TutorialId::RULES + base_id);
  tutorial_menu["Trivial moves"].SetID(+TutorialId::TRIVIAL_MOVES + base_id);
  tutorial_menu["Easy moves"].SetID(+TutorialId::EASY_MOVES + base_id);
  tutorial_menu["Speculative moves"].SetID(+TutorialId::SPECULATIVE_MOVES +
                                           base_id);
}

void
Tutorial::initialize(int base_id, olc::popup::Menu & tutorial_menu) {
  setup_menu(base_id, tutorial_menu);
}

void
Tutorial::update_menu(int selection, BoardGenerator & board_generator) {
  TutorialId tutorial_id{selection};
  std::cout << "tutorial update_menu" << std::endl;
  board_generator = TutorialLevels{tutorial_id, levels_.get()};
}
