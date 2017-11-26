#pragma once
#include <bitset>
#include <type_traits>

namespace pe {

//! \brief Struct that can be specialized on an enum type to enable bitmask-like
//! operations
template <typename T> struct EnableEnumBitmask : std::false_type {};

namespace detail {
template <typename T>
constexpr bool EnableEnumBitmask_v =
    EnableEnumBitmask<T>::value &&std::is_enum_v<T>;
}

//! \brief Bitwise OR operator for bitmask enums
template <typename T>
std::enable_if_t<detail::EnableEnumBitmask_v<T>, T> operator|(const T &l,
                                                              const T &r) {
  using Data_t = std::underlying_type_t<T>;
  return static_cast<T>(static_cast<Data_t>(l) | static_cast<Data_t>(r));
}

//! \brief Bitwise AND operator for bitmask enums
template <typename T>
std::enable_if_t<detail::EnableEnumBitmask_v<T>, T> operator&(const T &l,
                                                              const T &r) {
  using Data_t = std::underlying_type_t<T>;
  return static_cast<T>(static_cast<Data_t>(l) & static_cast<Data_t>(r));
}

//! \brief Bitflip operator for bitmask enums
template <typename T>
std::enable_if_t<detail::EnableEnumBitmask_v<T>, T> operator~(const T &val) {
  using Data_t = std::underlying_type_t<T>;
  return static_cast<T>(~static_cast<Data_t>(val));
}

template <typename T> uint32_t BitsSet(const T &val) {
  constexpr int Bitwidth =
      std::numeric_limits<T>::digits + std::numeric_limits<T>::is_signed;
  auto asBitset =
      std::bitset<Bitwidth>(static_cast<std::underlying_type_t<T>>(val));
  return asBitset.count();
}

template <typename T>
std::enable_if_t<detail::EnableEnumBitmask_v<T>, bool> operator!(const T &val) {
  return !static_cast<std::underlying_type_t<T>>(val);
}

} // namespace pe
