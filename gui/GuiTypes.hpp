#pragma once

#include "BoardModel.hpp"
#include <functional>

enum class TutorialId { RULES, TRIVIAL_MOVES, EASY_MOVES, SPECULATIVE_MOVES };

using BoardGenerator = std::function<model::BoardModel()>;

using UpdateHandler = std::function<bool(float ElapsedTime)>;
