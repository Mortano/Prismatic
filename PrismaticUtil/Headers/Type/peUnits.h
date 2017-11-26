#pragma once
#include <type_traits>

namespace pe {

//! \brief The constant Pi
template <typename T> constexpr T Pi = T(3.1415926535897932385L);
//! \brief The constant 1 / Pi
template <typename T> constexpr T InvPi = T(1.0 / Pi<T>);

namespace detail {
//! \brief Conversion between units, specialized by the concrete units
template <typename FromUnit, typename ToUnit> struct UnitConvert {
  static_assert(std::_Always_false<FromUnit>::value,
                "No unit conversion exists between these two units!");
};

//! \brief Basic unit type
//! \tparam DataType The data type used for the unit
//! \tparam Dim The dimensionality of the unit
template <typename DataType, size_t Dim = 1> struct BaseUnit {
  using DataType_t = DataType;
  constexpr static size_t Dimensionality = Dim;
};

} // namespace detail

#pragma region Angle

//! \brief Type-safe scalar
template <typename Unit> struct Scalar {
  template <typename U> friend struct Scalar;

  using DataType_t = typename Unit::DataType_t;

  constexpr Scalar() : _magnitude(0) {}

  constexpr explicit Scalar(DataType_t mag) : _magnitude(mag) {}

  constexpr Scalar(const Scalar<Unit> &other) = default;

  template <typename OtherUnit,
            typename = std::enable_if_t<!std::is_same_v<Unit, OtherUnit>>>
  constexpr Scalar(const Scalar<OtherUnit> &other)
      : _magnitude(other._magnitude *
                   detail::UnitConvert<OtherUnit, Unit>::value) {}

  constexpr Scalar<Unit> &operator=(const Scalar<Unit> &other) = default;

  template <typename OtherUnit>
  constexpr std::enable_if_t<!std::is_same_v<Unit, OtherUnit>, Scalar<Unit> &>
  operator=(const Scalar<OtherUnit> &other) {
    _magnitude = other._magnitude * detail::UnitConvert<OtherUnit, Unit>::value;
    return *this;
  }

#pragma region ArithmeticOperations

  template <typename T> constexpr Scalar<Unit> &operator+=(T mag) {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    _magnitude += mag;
    return *this;
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> &operator+=(const Scalar<OtherUnit> &other) {
    _magnitude +=
        other._magnitude * detail::UnitConvert<OtherUnit, Unit>::value;
    return *this;
  }

  template <typename T> constexpr Scalar<Unit> operator+(T mag) const {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    return Scalar<Unit>{_magnitude + mag};
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> operator+(const Scalar<OtherUnit> &other) const {
    return Scalar<Unit>{_magnitude +
                        other._magnitude *
                            detail::UnitConvert<OtherUnit, Unit>::value};
  }

  template <typename T> constexpr Scalar<Unit> &operator-=(T mag) {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    _magnitude -= mag;
    return *this;
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> &operator-=(const Scalar<OtherUnit> &other) {
    _magnitude -=
        other._magnitude * detail::UnitConvert<OtherUnit, Unit>::value;
    return *this;
  }

  template <typename T> constexpr Scalar<Unit> operator-(T mag) const {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    return Scalar<Unit>{_magnitude - mag};
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> operator-(const Scalar<OtherUnit> &other) const {
    return Scalar<Unit>{_magnitude -
                        other._magnitude *
                            detail::UnitConvert<OtherUnit, Unit>::value};
  }

  template <typename T> constexpr Scalar<Unit> &operator*=(T mag) {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    _magnitude *= mag;
    return *this;
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> &operator*=(const Scalar<OtherUnit> &other) {
    _magnitude *=
        other._magnitude * detail::UnitConvert<OtherUnit, Unit>::value;
    return *this;
  }

  template <typename T> constexpr Scalar<Unit> operator*(T mag) const {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    return Scalar<Unit>{_magnitude * mag};
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> operator*(const Scalar<OtherUnit> &other) const {
    return Scalar<Unit>{_magnitude * other._magnitude *
                        detail::UnitConvert<OtherUnit, Unit>::value};
  }

  template <typename T> constexpr Scalar<Unit> &operator/=(T mag) {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    _magnitude /= mag;
    return *this;
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> &operator/=(const Scalar<OtherUnit> &other) {
    _magnitude /=
        other._magnitude * detail::UnitConvert<OtherUnit, Unit>::value;
    return *this;
  }

  template <typename T> constexpr Scalar<Unit> operator/(T mag) const {
    static_assert(
        std::is_convertible_v<T, DataType_t>,
        "Type is not convertible to underlying data type of this Scalar!");
    return Scalar<Unit>{_magnitude / mag};
  }

  template <typename OtherUnit>
  constexpr Scalar<Unit> operator/(const Scalar<OtherUnit> &other) const {
    return Scalar<Unit>{_magnitude / other._magnitude *
                        detail::UnitConvert<OtherUnit, Unit>::value};
  }

#pragma endregion

  constexpr operator DataType_t() const { return _magnitude; }

  constexpr DataType_t get() const { return _magnitude; }

  //! \brief Converts to another unit
  //! \returns This scalar with other unit
  //! \tparam OtherUnit Another unit, different from <paramref name="Unit">
  template <typename OtherUnit>
  constexpr std::enable_if_t<!std::is_same_v<Unit, OtherUnit>,
                             Scalar<OtherUnit> &>
  To() const {
    return {*this};
  }

private:
  DataType_t _magnitude;
};

namespace detail {
struct Unit_Degrees : BaseUnit<float> {};

struct Unit_Radians : BaseUnit<float> {};

template <> struct UnitConvert<Unit_Degrees, Unit_Radians> {
  using ValType_t =
      std::common_type_t<Unit_Degrees::DataType_t, Unit_Radians::DataType_t>;
  constexpr static ValType_t value =
      static_cast<ValType_t>(Pi<ValType_t> / 180.0);
};

template <> struct UnitConvert<Unit_Radians, Unit_Degrees> {
  using ValType_t =
      std::common_type_t<Unit_Degrees::DataType_t, Unit_Radians::DataType_t>;
  constexpr static ValType_t value =
      static_cast<ValType_t>(180.0 / Pi<ValType_t>);
};

} // namespace detail

#pragma endregion

} // namespace pe

//! \brief Angle in degrees
using Degrees = pe::Scalar<pe::detail::Unit_Degrees>;
//! \brief Angle in radians
using Radians = pe::Scalar<pe::detail::Unit_Radians>;

inline Degrees operator"" _deg(long double degrees) {
  return Degrees{static_cast<Degrees::DataType_t>(degrees)};
}

inline Degrees operator"" _deg(unsigned long long int degrees) {
  return Degrees{static_cast<Degrees::DataType_t>(degrees)};
}

inline Radians operator"" _rad(long double radians) {
  return Radians{static_cast<Radians::DataType_t>(radians)};
}

inline Radians operator"" _rad(unsigned long long int radians) {
  return Radians{static_cast<Radians::DataType_t>(radians)};
}
