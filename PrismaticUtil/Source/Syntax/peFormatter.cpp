#include "Syntax\peFormatter.h"

pe::peFormatter::operator std::basic_string<char>() const {
  return _stream.str();
}

pe::peFormatter pe::Format() { return peFormatter{}; }
