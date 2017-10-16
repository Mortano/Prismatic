#pragma once
#include "DataStructures\peVector.h"
#include "peUtilDefs.h"

#include <functional>

namespace pe {

#pragma warning(push)
#pragma warning(disable : 4251)

//! Templated delegate that can store multiple function objects at once
template <typename... Args> class peDelegate {
  typedef std::function<void(Args...)> func_t;

public:
  peDelegate() : _functions(WrapAllocator<func_t>(GlobalAllocator)) {}

  const peDelegate &operator+=(const func_t &function) const {
    _functions.push_back(function);
    return *this;
  }

  void operator()(Args... args) {
    for (auto function : _functions) {
      function(std::forward<Args>(args)...);
    }
  }

private:
  mutable peVector<func_t> _functions;
  // Mutable and const += allows classes to return a const version of the
  // delegate  so that others may add functions but can't call the delegate
  // function!
};

#pragma warning(pop)

} // namespace pe
