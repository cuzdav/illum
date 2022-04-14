#pragma once

#include "picojson.h"
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace jsonvalidate {

namespace detail {

template <typename T>
char const * type_description;

template <>
char const * type_description<double> = "number[double]";
template <>
char const * type_description<int64_t> = "number[int64]";
template <>
char const * type_description<bool> = "bool";
template <>
char const * type_description<picojson::null> = "null";
template <>
char const * type_description<picojson::array> = "array";
template <>
char const * type_description<picojson::object> = "object";
template <>
char const * type_description<std::string> = "string";

inline std::string
to_string(bool b) {
  return b ? "true" : "false";
}

inline std::string
to_string(double dbl) {
  return std::to_string(dbl);
}

inline std::string
to_string(int64_t i64) {
  return std::to_string(i64);
}

inline std::string
to_string(picojson::null) {
  return "null";
}

} // namespace detail

using namespace std::string_literals;

using Value = picojson::value;

enum Required { True, False };

/////////////////
// VALUE CHECKING
/////////////////

struct AnyValue {
  bool
  check(auto const &) const {
    return true;
  }

  std::string
  describe() const {
    return "Always Valid";
  }
};

template <typename T>
struct Equal {
  Equal(T const & expected) : expected_(expected) {}

  bool
  check(auto const & value) const {
    return value == expected_;
  }

  std::string
  describe() const {
    return "[equals "s + detail::to_string(expected_);
  }

private:
  T expected_;
};

///////////////////////////// End of VALUE CHECKING

class Context {
public:
  auto
  get_error_string() const {
    return error_string_;
  }

  void
  clear_error() {
    error_string_ = std::nullopt;
  }

  void
  mismatch(std::string_view description) {
    if (req_top() == Required::True) {
      if (not error_string_.has_value()) {
        error_string_ = make_error_message(description);
      }
    }
  }

  template <typename Type>
  bool
  check(Value const & value, auto const & value_validator) {
    if (not value.is<Type>()) {
      mismatch(detail::type_description<Type>);
      return false;
    }

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
    return c.check<picojson::null>(value, AnyValue{});
  }
};

template <typename BasicTypeT>
struct DefaultChecker {
  template <typename ValChkT>
  bool
  operator()(Context &       context,
             Value const &   value,
             ValChkT const & value_validator) {
    return context.check<BasicTypeT>(value, value_validator);
  }
};

template <typename ValChkT,
          typename BasicTypeT,
          typename CheckFuncT = DefaultChecker<BasicTypeT>>
struct BasicTypeCheck {
  BasicTypeCheck(ValChkT const & val_check = ValChkT{})
      : value_validator_(val_check) {}

  bool
  operator()(Context & context, Value const & value) {
    return CheckFuncT{}(context, value, value_validator_);
  }

  [[no_unique_address]] ValChkT value_validator_;
};

template <typename ValChkT = AnyValue>
using Bool = BasicTypeCheck<ValChkT, bool>;

template <typename ValChkT = AnyValue>
using String = BasicTypeCheck<ValChkT, std::string>;

template <typename ValChkT = AnyValue>
using Int64 = BasicTypeCheck<ValChkT, std::int64_t>;

// picojson has a strange way of treating int64_t as if it was a double, but
// does not treat a double as an int64_t. That is, when v holds an int64, and
// you call v.is<int64> it returns true, but when you call v.is<double> it also
// return true. If you call v.get<double> when it holds an int64_t, it'll
// actually rewrite the value in the union from an int64 to double, and change
// the type flag to that of double. There is no going back. (double is NOT
// considered int64_t). This conversion casts away constness, which could in some
// situations, cause undefined behavior. I'm opting to avoid the issue by
// treating int64 and double as distinctly different types, and ensure we're not
// dealing with an int64 when we really want doubles.
template <typename ValChkT = AnyValue>
using Double = BasicTypeCheck<ValChkT,
                              double,
                              decltype([](Context &       context,
                                          Value const &   value,
                                          ValChkT const & checker) {
                                if (value.is<std::int64_t>()) {
                                  context.mismatch("int64 as fake double");
                                  return false;
                                }
                                return context.check<double>(value, checker);
                              })>;

template <typename... TypeCheckT>
struct AnyOf {
  AnyOf(TypeCheckT const &... type_checkers) : validators_{type_checkers...} {}

  bool
  operator()(Context & c, Value const & value) {
    return [&, this ]<size_t... Idx>(std::index_sequence<Idx...>) {
      return ((c.clear_error(), std::get<Idx>(validators_)(c, value)) || ...);
    }
    (std::make_index_sequence<sizeof...(TypeCheckT)>());
  }

  std::tuple<TypeCheckT...> validators_;
};

template <typename T>
struct Array {};

template <typename T>
struct Object {};

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
