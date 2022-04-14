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

TEST(JsonValidateTest, validate_int64_type) {
  picojson::value v;
  Validator       validator{Double{}};
  auto            error_msg = validator.validate(v);

  ASSERT_TRUE(error_msg.has_value());

  v.set("hello"s);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  std::int64_t num = 123456;
  v.set(num);
  error_msg = validator.validate(v);
  ASSERT_FALSE(error_msg.has_value());
}

TEST(JsonValidateTest, validate_double_type) {
  picojson::value v;
  Validator       validator{Double{}};
  auto            error_msg = validator.validate(v);

  ASSERT_TRUE(error_msg.has_value());

  v.set("hello"s);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  v.set(1.5);
  error_msg = validator.validate(v);
  ASSERT_FALSE(error_msg.has_value());
}

TEST(JsonValidateTest, validate_bool_type) {
  picojson::value v;
  Validator       validator{Bool{}};
  auto            error_msg = validator.validate(v);

  ASSERT_TRUE(error_msg.has_value());

  v.set("hello"s);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  v.set(true);
  error_msg = validator.validate(v);
  ASSERT_FALSE(error_msg.has_value());
}

TEST(JsonValidateTest, validate_bool_value) {
  picojson::value v;
  Validator       validator{Bool{Equal{true}}};
  auto            error_msg = validator.validate(v);

  ASSERT_TRUE(error_msg.has_value());

  v.set("hello"s);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  v.set(false);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  v.set(true);
  error_msg = validator.validate(v);
  ASSERT_FALSE(error_msg.has_value());
}

TEST(JsonValidateTest, AnyOf_test1) {
  AnyOf     truebool_or_dbl{Bool{Equal{true}}, Double{}};
  Validator validator{truebool_or_dbl};

  picojson::value v;
  auto            error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  v.set(false);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  v.set("hello"s);
  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  //-----------------
  // NASTY
  v.set(double(234));
  ASSERT_TRUE(v.is<double>());
  ASSERT_FALSE(v.is<int64_t>());
  v.set(std::int64_t(234));
  ASSERT_TRUE(v.is<double>());
  ASSERT_TRUE(v.is<int64_t>()); // <=== whoa
  //-----------------

  error_msg = validator.validate(v);
  ASSERT_TRUE(error_msg.has_value());

  // Success cases:

  v.set(234.0);
  error_msg = validator.validate(v);
  ASSERT_FALSE(error_msg.has_value());

  v.set(true);
  error_msg = validator.validate(v);
  ASSERT_FALSE(error_msg.has_value());
}

} // namespace jsonvalidate
