#include "jsonschema.hpp"

// For future use if valijson is updated

#include <valijson/adapters/picojson_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/utils/picojson_utils.hpp>
#include <valijson/validator.hpp>

picojson::value
load_document(std::string const & filename) {
  picojson::value result;
  if (!valijson::utils::loadDocument(filename, result)) {
    throw std::runtime_error("Failed to load json " + filename);
  }
  return result;
}

picojson::value
load_and_validate_json(std::string const & target_filename,
                       std::string const & schema_filename) {

  valijson::Schema schema;

  auto schema_json = load_document(schema_filename);
  valijson::adapters::PicoJsonAdapter adapter(schema_json);

  valijson::SchemaParser parser;

  parser.populateSchema(adapter, schema);
  picojson::value target_json = load_document(target_filename);
  valijson::adapters::PicoJsonAdapter target_adapter(target_json);
  valijson::Validator                 validator;

  if (!validator.validate(schema, target_adapter, nullptr)) {
    throw std::runtime_error("Validation failed.");
  }

  return target_json;
}
