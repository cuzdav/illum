#include "TutorialLevels.hpp"

#include "ASCIILevelCreator.hpp"
#include <stdexcept>

static constexpr char const * ACTIONS_KEY      = "actions";
static constexpr char const * BOARD_KEY        = "board";
static constexpr char const * LEVEL_NAME_KEY   = "level_name";
static constexpr char const * LEVELS_KEY       = "levels";
static constexpr char const * NAME_KEY         = "name";
static constexpr char const * SECTION_NAME_KEY = "section_name";

using namespace std::literals;

static std::string
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
    throw std::runtime_error("JSON expected to have key of: "s + key);
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

TutorialLevels::TutorialLevels(TutorialId              start_at,
                               picojson::value const * level_json)
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
TutorialLevels::operator()() {
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

bool
TutorialLevels::advance_section() {
  std::cout << "******************** TutorialLevels::advance() called"
            << std::endl;

  return true;
}
