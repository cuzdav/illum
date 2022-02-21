#pragma once

#include "BoardModel.hpp"
#include <functional>

using BoardGenerator = std::function<model::BoardModel()>;
