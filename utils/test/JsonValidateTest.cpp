#include "utils/JsonValidate.hpp"
#include "picojson.h"
#include "gtest/gtest.h"

namespace jsonvalidate {

using namespace std::literals;

TEST(JsonValidateTest, validate_null) {
  picojson::value v;
  Validator       validator(Null{});
  auto            error_msg = validator.validate(v);

  ASSERT_FALSE(error_msg.has_value());

  v.set("hello"s);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());
}

TEST(JsonValidateTest, validate_number) {
  picojson::value v;
  Validator       validator{Double{}};
  auto            error_msg = validator.validate(v);

  ASSERT_TRUE(error_msg.has_value());

  v.set("hello"s);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  double dbl = 1.5;
  v.set(dbl);
  error_msg = validator.validate(v);
  ASSERT_FALSE(error_msg.has_value());
}

} // namespace jsonvalidate
