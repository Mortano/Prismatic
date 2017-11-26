#pragma once
#include <sstream>
#include <string>

#include "peUtilDefs.h"

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief Utility for inline formatting
class PE_UTIL_API peFormatter {
public:
  peFormatter() = default;

  template <typename T> peFormatter &operator<<(const T &elem);

  operator std::string() const;

private:
  std::stringstream _stream;
};

template <typename T> peFormatter &peFormatter::operator<<(const T &elem) {
  _stream << elem;
  return *this;
}

PE_UTIL_API peFormatter Format();

} // namespace pe

#pragma warning(pop)
