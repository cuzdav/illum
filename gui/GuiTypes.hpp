#pragma once

#include "BoardModel.hpp"
#include <functional>

using BoardGenerator = std::function<model::BoardModel()>;

using UpdateHandler = std::function<bool(float ElapsedTime)>;
