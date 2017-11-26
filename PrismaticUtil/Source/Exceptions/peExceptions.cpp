
#include "Exceptions\peExceptions.h"

#include "Memory\peLeakDetection.h"

namespace pe {

peBaseException::peBaseException(const char *what) : std::runtime_error(what) {
  peStackWalker::Init();
  size_t adresses[20];
  auto numFrames = peStackWalker::GetCallstack(20, adresses);

  constexpr size_t NameBufferSize = 1024;
  auto functionNames = static_cast<char *>(malloc(numFrames * NameBufferSize));
  peStackWalker::ResolveCallstack(adresses, numFrames, functionNames,
                                  NameBufferSize);

  std::stringstream ss;
  // Print the callstack separately
  char buffer[NameBufferSize];
  for (size_t j = 0; j < numFrames; j++) {
    memcpy(buffer, functionNames + (j * NameBufferSize), NameBufferSize);
    ss << buffer << "\n";
  }
  free(functionNames);

  _callTrace = ss.str();
}

#pragma region DllLoadFailedException

peDllLoadFailedException::peDllLoadFailedException(const char *what)
    : peBaseException(what) {}

peDllLoadFailedException::~peDllLoadFailedException() {}

#pragma endregion

#pragma region WindowCreationFailedException

peWindowCreationFailedException::peWindowCreationFailedException(
    const char *what)
    : peBaseException(what) {}

#pragma endregion

#pragma region WinApiErrorException

char *WinErrorCodeToMsg(DWORD errorCode) {
  static constexpr size_t BufferSize = 512;
  auto msg = static_cast<char *>(malloc(BufferSize * sizeof(TCHAR)));
  TCHAR buffer[BufferSize] = {0};
  ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, // It´s a system error
                  nullptr,   // No string to be formatted needed
                  errorCode, // Hey Windows: Please explain this error!
                  MAKELANGID(LANG_NEUTRAL,
                             SUBLANG_DEFAULT), // Do it in the standard language
                  buffer,                      // Put the message here
                  ((BufferSize - 1) *
                   sizeof(TCHAR)), // Number of bytes to store the message
                  nullptr);

  auto size_needed = WideCharToMultiByte(CP_ACP, 0, buffer, BufferSize, nullptr,
                                         0, nullptr, nullptr);
  PE_ASSERT(size_needed < (BufferSize * sizeof(TCHAR)));
  WideCharToMultiByte(CP_ACP, 0, buffer, BufferSize, msg, size_needed, nullptr,
                      nullptr);
  return msg;
}

peWinApiErrorException::peWinApiErrorException(DWORD errorCode)
    : peBaseException(_message = WinErrorCodeToMsg(errorCode)) {}

peWinApiErrorException::~peWinApiErrorException() {
  free(const_cast<char *>(_message));
  _message = nullptr;
}

#pragma endregion

#pragma region GlProcMissingException

peGlProcMissingException::peGlProcMissingException(const char *what)
    : peBaseException(what) {}

#pragma endregion

} // namespace pe
