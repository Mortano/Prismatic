#pragma once
#include "peUtilDefs.h"

#include <Windows.h>
#include <exception>
#include <sstream>
#include <string>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace std {
class PE_UTIL_API runtime_error;
}

namespace pe {

struct PE_UTIL_API peBaseException : std::runtime_error {
  explicit peBaseException(const char *what);

  const auto &GetCallTrace() const { return _callTrace; }

private:
  std::string _callTrace;
};

//! Exception for a failed dll load operation
class PE_UTIL_API peDllLoadFailedException : public peBaseException {
public:
  explicit peDllLoadFailedException(const char *what);
  ~peDllLoadFailedException();
};

//! Exception for a failed window creation process
class PE_UTIL_API peWindowCreationFailedException : public peBaseException {
public:
  explicit peWindowCreationFailedException(const char *what);
};

//! Exception for WinAPI errors
class PE_UTIL_API peWinApiErrorException : public peBaseException {
public:
  explicit peWinApiErrorException(DWORD errorCode);
  ~peWinApiErrorException();

private:
  char *_message;
};

//! Exception for missing OpenGL functions!
class PE_UTIL_API peGlProcMissingException : public peBaseException {
public:
  explicit peGlProcMissingException(const char *what);
};

} // namespace pe

#pragma warning(pop)
