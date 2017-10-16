#pragma once
#include "peUtilDefs.h"

#include <Windows.h>
#include <exception>
#include <string>

namespace std {
class PE_UTIL_API runtime_error;
}

namespace pe {

//! Exception for a failed dll load operation
class PE_UTIL_API peDllLoadFailedException : public std::runtime_error {
public:
  explicit peDllLoadFailedException(const char *what);
  ~peDllLoadFailedException();
};

//! Exception for a failed window creation process
class PE_UTIL_API peWindowCreationFailedException : public std::runtime_error {
public:
  explicit peWindowCreationFailedException(const char *what);
};

//! Exception for WinAPI errors
class PE_UTIL_API peWinApiErrorException : public std::runtime_error {
public:
  explicit peWinApiErrorException(DWORD errorCode);
  ~peWinApiErrorException();

private:
  char *_message;
};

//! Exception for missing OpenGL functions!
class PE_UTIL_API peGlProcMissingException : public std::runtime_error {
public:
  explicit peGlProcMissingException(const char *what);
};

} // namespace pe
