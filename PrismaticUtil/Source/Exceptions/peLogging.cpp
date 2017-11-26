#include "Exceptions\peLogging.h"

#include "Algorithms/peAlgorithms.h"
#include <stdio.h>

namespace pe {

peLogging::peLogging()
    : _logChannels(WrapAllocator<ILogChannel *>(GlobalAllocator)) {}

peLogging::~peLogging() {
  // Delete all remaining log channels so we get no leaks!
  for (auto logChannel : _logChannels) {
    Delete(logChannel);
  }
}

void peLogging::DeregisterLogChannel(ILogChannel *channel) {
  std::lock_guard<std::mutex> lock{_channelsLock};
  auto find = std::find(_logChannels.begin(), _logChannels.end(), channel);
  if (find == _logChannels.end())
    return;
  _logChannels.erase(find);
}

void peLogging::RegisterLogChannel(ILogChannel *channel) {
  std::lock_guard<std::mutex> lock(_channelsLock);
  PE_ASSERT(!Contains(_logChannels, channel));
  _logChannels.push_back(channel);
}

void peLogging::LogError(const char *msg, ...) {
  char buffer[2048];
  va_list argptr;
  va_start(argptr, msg);
  vsprintf_s(buffer, msg, argptr);
  va_end(argptr);

  // Append newline
  char *pBuffer = &buffer[0];
  while ((*pBuffer) != 0) {
    pBuffer++;
  }
  (*pBuffer) = '\n';
  pBuffer++;
  (*pBuffer) = 0;

  std::lock_guard<std::mutex> lock(_channelsLock);
  for (auto channel : _logChannels) {
    channel->Log(Error, buffer);
  }
}

void peLogging::LogWarning(const char *msg, ...) {
  char buffer[2048];
  va_list argptr;
  va_start(argptr, msg);
  vsprintf_s(buffer, msg, argptr);
  va_end(argptr);

  // Append newline
  char *pBuffer = &buffer[0];
  while ((*pBuffer) != 0) {
    pBuffer++;
  }
  (*pBuffer) = '\n';
  pBuffer++;
  (*pBuffer) = 0;

  std::lock_guard<std::mutex> lock(_channelsLock);
  for (auto channel : _logChannels) {
    channel->Log(Warning, buffer);
  }
}

void peLogging::LogInfo(const char *msg, ...) {
  char buffer[2048];
  va_list argptr;
  va_start(argptr, msg);
  vsprintf_s(buffer, msg, argptr);
  va_end(argptr);

  // Append newline
  char *pBuffer = &buffer[0];
  while ((*pBuffer) != 0) {
    pBuffer++;
  }
  (*pBuffer) = '\n';
  pBuffer++;
  (*pBuffer) = 0;

  std::lock_guard<std::mutex> lock(_channelsLock);
  auto iter = _logChannels.begin();
  for (auto channel : _logChannels) {
    channel->Log(Info, buffer);
  }
}

//--------ConsoleLogChannel--------

void peConsoleLogChannel::Log(LogLevel level, const char *msg) {
  // Set color
  HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  WORD color = 0x07;
  switch (level) {
  case Error:
    color = 0x0C;
    break;
  case Warning:
    color = 0x0E;
    break;
  case Info:
    break;
  default:
    break;
  }
  if (color != 0x07)
    SetConsoleTextAttribute(stdOutHandle, color);
  // Print message
  printf(msg);
  OutputDebugStringA(msg); // For VisualStudio
  if (color != 0x07)
    SetConsoleTextAttribute(stdOutHandle, 0x07);
}

peFileLogChannel::peFileLogChannel(const std::string &filePath)
    : _fstream(filePath.c_str()) {}

void peFileLogChannel::Log(LogLevel level, const char *msg) { _fstream << msg; }

} // namespace pe
