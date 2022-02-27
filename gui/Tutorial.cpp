#include "Tutorial.hpp"
#include "ASCIILevelCreator.hpp"
#include "GuiTypes.hpp"
#include "olcRetroMenu.hpp"
#include "utils/EnumUtils.hpp"
#include "utils/picojson.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std::literals;

enum class TutorialId { RULES, TRIVIAL_MOVES, EASY_MOVES, SPECULATIVE_MOVES };

static constexpr char const * ACTIONS_KEY      = "actions";
static constexpr char const * BOARD_KEY        = "board";
static constexpr char const * LEVEL_NAME_KEY   = "level_name";
static constexpr char const * LEVELS_KEY       = "levels";
static constexpr char const * NAME_KEY         = "name";
static constexpr char const * SECTION_NAME_KEY = "section_name";

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

static void
ensure_key_exists(char const * key, picojson::value const & val) {
  if (not val.contains(key)) {
    throw std::runtime_error("Jason expected to have key of: "s + key);
  }
}

picojson::value::array const &
get_levels_array(picojson::value const & json) {
  using value = picojson::value;
  using array = value::array;

  value const & levels = json.get(LEVELS_KEY);

  if (not levels.is<array>()) {
    throw std::runtime_error(
        "Error in tutorial json: \"levels\" field of json is expected to "
        "be "
        "an array");
  }
  return levels.get<array>();
}

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
  TutorialLevels(TutorialId start_at, picojson::value const * level_json)
      : levels_data_{*level_json}, level_index_{0} {
    ensure_key_exists(NAME_KEY, levels_data_);
    ensure_key_exists(LEVELS_KEY, levels_data_);

    std::string section = to_string(start_at);
    for (auto const & obj : get_levels_array(levels_data_)) {
      if (obj.contains(SECTION_NAME_KEY) &&
          obj.get(SECTION_NAME_KEY).to_str() == section) {
        return;
      }
      ++level_index_;
    }
    throw std::runtime_error("No tutorial level json section \""s +
                             SECTION_NAME_KEY + "\" named \"" + section + "\"");
  }

  model::BoardModel
  operator()() {
    std::cout << "******************** operator() called on TutorialLevels"
              << std::endl;
    auto const & levels_ary = get_levels_array(levels_data_);
    if (levels_ary.size() > level_index_) {
      auto const & level_json = levels_ary[level_index_];
      ++level_index_;
      ensure_key_exists(LEVEL_NAME_KEY, level_json);
      ensure_key_exists(BOARD_KEY, level_json);
      ensure_key_exists(ACTIONS_KEY, level_json);

      // TODO: use the actions and level name
      return model::ASCIILevelCreator{}.create_from_json(
          level_json.get(BOARD_KEY));
    }

    throw std::runtime_error("Walked off end of tutorials levels array");
  }

private:
  picojson::value levels_data_;
  std::size_t     level_index_;
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

Tutorial::Tutorial() : levels_data_{std::make_unique<picojson::value>()} {

  auto tutorial_levels_jsonstr = load_file("tutorial_levels.json");
  if (std::string errmsg =
          picojson::parse(*levels_data_, tutorial_levels_jsonstr);
      not errmsg.empty()) {
    throw std::runtime_error("Invalid json in tutorial data:" + errmsg);
  }
}

Tutorial::~Tutorial() = default;

bool
Tutorial::update_game(float fElapsedTime) {
  return true;
}

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
  std::cout << "DBG: tutorial update_menu, selection = " << selection
            << std::endl;
  board_generator = TutorialLevels{tutorial_id, levels_data_.get()};
}
