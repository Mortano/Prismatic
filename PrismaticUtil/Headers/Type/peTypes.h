#pragma once
#include "peUtilDefs.h"

namespace pe {

struct PE_UTIL_API peNonMovable {
  peNonMovable() = default;
  peNonMovable(peNonMovable &&) = delete;
  peNonMovable &operator=(peNonMovable &&) = delete;
};

struct PE_UTIL_API peNonCopyable : peNonMovable {
  peNonCopyable() = default;
  peNonCopyable(const peNonCopyable &) = delete;
  peNonCopyable &operator=(const peNonCopyable &) = delete;
};

} // namespace pe
