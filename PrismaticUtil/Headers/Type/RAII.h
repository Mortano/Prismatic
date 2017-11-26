#pragma once
#include <functional>

namespace pe {

//! \brief Simple RAII wrapper that wraps an object and executes a function if
//! it goes out of scope
template <typename T> struct RAII {

  RAII(T &ref, std::function<void(T &)> func);
  ~RAII();

  T *operator->();
  T &operator=(const T &other);
  T &operator=(T &&other);

private:
  T &_ref;
  std::function<void(T &)> _func;
};

#pragma region Impl

template <typename T>
RAII<T>::RAII(T &ref, std::function<void(T &)> func) : _ref(ref), _func(func) {}

template <typename T> RAII<T>::~RAII() { _func(_ref); }

template <typename T> T *RAII<T>::operator->() { return &_ref; }

template <typename T> T &RAII<T>::operator=(const T &other) {
  _ref = other;
  return _ref;
}

template <typename T> T &RAII<T>::operator=(T &&other) {
  _ref = std::move(other);
  return _ref;
}

#pragma endregion

} // namespace pe
