#pragma once

#include "picojson.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace jsonvalidate {

using namespace std::string_literals;

using Value = picojson::value;

enum Required { True, False };

struct AnyValue {
  static bool
  check(auto const &) {
    return true;
  }

  static std::string
  describe() {
    return "Always Valid";
  }
};

class Context {
public:
  auto
  get_error_string() const {
    return error_string_;
  }

  void
  mismatch(std::string_view description) {
    if (req_top() == Required::True) {
      if (not error_string_.has_value()) {
        error_string_ = make_error_message(description);
      }
    }
  }

  bool
  check_type(char const * description, bool passes) {
    if (not passes) {
      mismatch(description);
    }
    return passes;
  }

  template <typename Type, typename ValueValidatorT>
  bool
  check(char const * type_description, Value const & value) {
    if (not value.is<Type>()) {
      mismatch(type_description);
      return false;
    }
    ValueValidatorT value_validator;
    if constexpr (not std::is_same_v<picojson::null, Type>) {
      bool passes = value_validator.check(value.get<Type>());
      if (not passes) {
        mismatch(value_validator.describe());
        return false;
      }
    }
    return true;
  }

  Required
  req_top() const {
    return required_stack_.back();
  }

  void
  push_required_stack(Required req) {
    required_stack_.push_back(req);
  }
  void
  pop_required_stack(Required req) {
    if (required_stack_.size() > 1) {
      required_stack_.pop_back();
      if (req_top() == Required::False) {
        error_string_ = std::nullopt;
      }
    }
    else {
      throw std::runtime_error("Invalid pop_required_stack");
    }
  }

private:
  std::string
  make_error_message(std::string_view description) {
    return "<an error happened> "s +
           std::string(description.data(), description.size());
  }

  std::vector<Required>      required_stack_{Required::True};
  std::optional<std::string> error_string_;
};

struct Null {
  bool
  operator()(Context & c, Value const & value) {
    return c.check<picojson::null, AnyValue>("null", value);
  }
};

template <typename ValueValidatorT = AnyValue>
struct Bool {
  bool
  operator()(Context & c, Value const & value) {
    return c.check<bool, ValueValidatorT>("bool", value);
  }
};

template <typename ValueValidatorT = AnyValue>
struct Double {
  bool
  operator()(Context & c, Value const & value) {
    return c.check<double, ValueValidatorT>("double", value);
  }
};

template <typename ValueValidatorT = AnyValue>
struct Int64 {
  bool
  operator()(Context & c, Value const & value) {
    return c.check<std::int64_t, ValueValidatorT>("int64", value);
  }
};

struct String {};

template <typename T>
struct Array {};

template <typename T>
struct Object {};

template <typename... T>
struct AnyOf {};

template <typename... T>
struct AllOf {};

template <typename SchemaT>
class Validator {
public:
  Validator(SchemaT const & schema) : schema_{schema} {}

  std::optional<std::string>
  validate(Value const & value) {
    Context context;
    schema_(context, value);
    return context.get_error_string();
  }

private:
  SchemaT schema_;
};

} // namespace jsonvalidate
