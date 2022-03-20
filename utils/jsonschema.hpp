#pragma once

#include "picojson.h"
#include <string>

picojson::value load_and_validate_json(std::string const & document_filename,
                                       std::string const & schema_filename);
