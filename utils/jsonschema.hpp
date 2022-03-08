#pragma once

#include "utils/picojson.hpp"
#include <string>

picojson::value load_and_validate_json(std::string const & document_filename,
                                       std::string const & schema_filename);
