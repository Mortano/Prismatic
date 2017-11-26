#pragma once

#include "Meta.h"
#include <array>

namespace pe {
namespace {
template <typename To> struct AsConvertibleHelper {
  template <typename Arg>
  struct Converter : std::bool_constant<std::is_constructible_v<Arg, To>> {};
};

template <typename Accum, typename Next> struct AndFold {
  using type = std::bool_constant<Accum{} && Next{}>;
};

template <typename To, typename... From> constexpr bool AllConvertible() {
  using Types = mdv::meta::Typelist<From...>;
  using Convertibles =
      mdv::meta::Transform_t<AsConvertibleHelper<To>::template Converter,
                             Types>;
  using Folded =
      mdv::meta::Foldl_t<AndFold, std::bool_constant<true>, Convertibles>;
  return Folded::value;
}

} // namespace

//! \brief Typesafe color structure
template <size_t Channels, typename Precision> class peColor {
public:
  static_assert(Channels > 0, "Channels must be greater than zero!");
  constexpr static size_t NumChannels = Channels;
  using Precision_t = Precision;
  using DataType_t = typename Precision::DataType_t;

  //! \brief initializes all channels of this color with the default value of
  //! Precision
  constexpr peColor() { _data.fill(DataType_t{}); }

  //! \brief initializes all channels of this color with the given value
  constexpr explicit peColor(DataType_t val) { _data.fill(val); }

  //! \brief Initializes each channel with the given value
  template <typename... Args, typename = std::enable_if_t<Channels == 1>>
  constexpr explicit peColor(DataType_t arg) : _data{{arg}} {}

  template <typename... Args, typename = std::enable_if_t<Channels == 2>>
  constexpr explicit peColor(DataType_t arg1, DataType_t arg2)
      : _data{{arg1, arg2}} {}

  template <typename... Args, typename = std::enable_if_t<Channels == 3>>
  constexpr explicit peColor(DataType_t arg1, DataType_t arg2, DataType_t arg3)
      : _data{{arg1, arg2, arg3}} {}

  template <typename... Args, typename = std::enable_if_t<Channels == 4>>
  constexpr explicit peColor(DataType_t arg1, DataType_t arg2, DataType_t arg3,
                             DataType_t arg4)
      : _data{{arg1, arg2, arg3, arg4}} {}

  constexpr peColor(const peColor &other) = default;
  constexpr peColor(peColor &&other) = default;

  constexpr peColor &operator=(const peColor &other) = default;
  constexpr peColor &operator=(peColor &&other) = default;

  //! \brief Returns a raw pointer to the data of this color
  //! \returns Raw pointer to data
  constexpr auto data() const { return _data.data(); }

  template <size_t Idx> constexpr const DataType_t &at() const {
    static_assert(Idx < NumChannels, "Index out of bounds!");
    return _data[Idx];
  }

  //! \brief Is this color completely black?
  bool IsBlack() const {
    for (size_t idx = 0; idx < Channels; ++idx) {
      if (_data[idx] != DataType_t(0))
        return false;
    }
    return true;
  }

#pragma region Operators

  constexpr DataType_t &operator[](const size_t idx) { return _data[idx]; }

  constexpr DataType_t const &operator[](const size_t idx) const {
    return _data[idx];
  }

  constexpr peColor operator+(const peColor &other) const {
    auto ret = *this;
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      ret._data[idx] += other._data[idx];
    }
    return ret;
  }

  constexpr peColor &operator+=(const peColor &other) {
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      _data[idx] += other._data[idx];
    }
    return *this;
  }

  constexpr peColor operator-(const peColor &other) const {
    auto ret = *this;
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      ret._data[idx] -= other._data[idx];
    }
    return ret;
  }

  constexpr peColor &operator-=(const peColor &other) {
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      _data[idx] -= other._data[idx];
    }
    return *this;
  }

  constexpr peColor operator*(const peColor &other) const {
    auto ret = *this;
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      ret._data[idx] *= other._data[idx];
    }
    return ret;
  }

  constexpr peColor &operator*=(const peColor &other) {
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      _data[idx] *= other._data[idx];
    }
    return *this;
  }

  constexpr peColor operator/(const peColor &other) const {
    auto ret = *this;
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      ret._data[idx] /= other._data[idx];
    }
    return ret;
  }

  constexpr peColor &operator/=(const peColor &other) {
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      _data[idx] /= other._data[idx];
    }
    return *this;
  }

  constexpr peColor &operator*=(DataType_t scalar) {
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      _data[idx] *= scalar;
    }
    return *this;
  }

  constexpr peColor &operator/=(DataType_t scalar) {
    for (size_t idx = 0; idx < NumChannels; ++idx) {
      _data[idx] /= scalar;
    }
    return *this;
  }

  friend constexpr peColor operator+(const peColor &col, DataType_t scalar) {
    auto ret = col;
    for (size_t idx = 0; idx < Channels; ++idx) {
      ret._data[idx] += scalar;
    }
    return ret;
  }

  friend constexpr peColor operator*(const peColor &col, DataType_t scalar) {
    auto ret = col;
    for (size_t idx = 0; idx < Channels; ++idx) {
      ret._data[idx] *= scalar;
    }
    return ret;
  }

  friend constexpr peColor operator*(DataType_t scalar, const peColor &col) {
    return col * scalar;
  }

  friend constexpr peColor operator/(const peColor &col, DataType_t scalar) {
    auto ret = col;
    for (size_t idx = 0; idx < Channels; ++idx) {
      ret._data[idx] /= scalar;
    }
    return ret;
  }

#pragma endregion

  DataType_t &r() { return _data[0]; }
  const DataType_t &r() const { return _data[0]; }

  template <size_t _Channels = Channels>
  std::enable_if_t<_Channels >= 2, DataType_t &> g() {
    return _data[1];
  }

  template <size_t _Channels = Channels>
  std::enable_if_t<_Channels >= 2, const DataType_t &> g() const {
    return _data[1];
  }

  template <size_t _Channels = Channels>
  std::enable_if_t<_Channels >= 3, DataType_t &> b() {
    return _data[2];
  }

  template <size_t _Channels = Channels>
  std::enable_if_t<_Channels >= 3, const DataType_t &> b() const {
    return _data[2];
  }

  template <size_t _Channels = Channels>
  std::enable_if_t<_Channels >= 4, DataType_t &> a() {
    return _data[3];
  }

  template <size_t _Channels = Channels>
  std::enable_if_t<_Channels >= 4, const DataType_t &> a() const {
    return _data[3];
  }

private:
  std::array<DataType_t, Channels> _data;
};

struct Precision8Bit {
  using DataType_t = uint8_t;
};

struct Precision16Bit {
  using DataType_t = uint16_t;
};

struct Precision32Bit {
  using DataType_t = uint32_t;
};

struct PrecisionFloat32Bit {
  using DataType_t = float;
};

//! \brief 8-bit per channel integer RGBA color
using RGBA_8Bit = peColor<4, Precision8Bit>;
//! \brief 32-bit per channel float RGBA color
using RGBA_32BitFloat = peColor<4, PrecisionFloat32Bit>;
//! \brief 8-bit per channel integer RGB color
using RGB_8Bit = peColor<3, Precision8Bit>;
//! \brief 32-bit per channel float RGB color
using RGB_32BitFloat = peColor<3, PrecisionFloat32Bit>;
//! \brief 8-bit integer greyscale color
using Greyscale_8Bit = peColor<1, Precision8Bit>;
//! \brief 32-bit integer greyscale color
using Greyscale_32Bit = peColor<1, Precision32Bit>;

namespace detail {
template <typename T> struct IsColor : std::false_type {};

template <size_t Channels, typename Precision>
struct IsColor<peColor<Channels, Precision>> : std::true_type {};

template <size_t Channels, typename FromPrecision, typename ToPrecision,
          size_t... Is>
constexpr peColor<Channels, ToPrecision>
color_cast_same_dim(const peColor<Channels, FromPrecision> &from,
                    std::index_sequence<Is...>) {
  static_assert(std::is_convertible_v<typename FromPrecision::DataType_t,
                                      typename ToPrecision::DataType_t>,
                "Can't convert from FromPrecision to ToPrecision!");
  return peColor<Channels, ToPrecision>{from.template at<Is>()...};
}

template <size_t FromChannels, size_t ToChannels, typename FromPrecision,
          typename ToPrecision, size_t... Is>
constexpr peColor<ToChannels, ToPrecision>
color_cast_src_dim_bigger(const peColor<FromChannels, FromPrecision> &from,
                          std::index_sequence<Is...>) {
  static_assert(std::is_convertible_v<typename FromPrecision::DataType_t,
                                      typename ToPrecision::DataType_t>,
                "Can't convert from FromPrecision to ToPrecision!");
  // We just drop all entries in 'from' that exceed the destination
  // dimensionality. So converting from RGBA to RG would drop B and A
  return peColor<ToChannels, ToPrecision>{from.template at<Is>()...};
}

template <typename T, size_t> T DefaultConstructHelper() { return T{}; }

template <size_t FromChannels, size_t ToChannels, typename FromPrecision,
          typename ToPrecision, size_t... SrcIs, size_t... DstIs>
constexpr peColor<ToChannels, ToPrecision>
color_cast_dst_dim_bigger(const peColor<FromChannels, FromPrecision> &from,
                          std::index_sequence<SrcIs...>,
                          std::index_sequence<DstIs...>) {
  static_assert(std::is_convertible_v<typename FromPrecision::DataType_t,
                                      typename ToPrecision::DataType_t>,
                "Can't convert from FromPrecision to ToPrecision!");
  // We convert all entries and fill the rest with default constructed values
  return peColor<ToChannels, ToPrecision>{
      from.template at<SrcIs>()...,
      DefaultConstructHelper<typename ToPrecision::DataType_t, DstIs>()...};
}

template <size_t Offset, size_t... Is>
constexpr decltype(auto)
make_index_sequence_helper(std::index_sequence<Is...>) {
  return std::index_sequence<(Offset + Is)...>{};
}
} // namespace detail
} // namespace pe
