#pragma once

#pragma warning(push)
#pragma warning(disable : 4251)

#include "DataStructures/peVector.h"
#include "Memory\peAllocatable.h"
#include "peUtilDefs.h"
#include <fstream>
#include <mutex>

namespace pe {

enum PE_UTIL_API LogLevel { Error, Warning, Info };

class PE_UTIL_API ILogChannel : public peAllocatable {
public:
  virtual ~ILogChannel() {}

  virtual void Log(LogLevel level, const char *msg) = 0;
};

class PE_UTIL_API peLogging : public peAllocatable {
public:
  peLogging();
  ~peLogging();

  void DeregisterLogChannel(ILogChannel *channel);
  void RegisterLogChannel(ILogChannel *channel);

  void LogError(_Printf_format_string_ const char *msg, ...);
  void LogWarning(_Printf_format_string_ const char *msg, ...);
  void LogInfo(_Printf_format_string_ const char *msg, ...);

private:
  peVector<ILogChannel *> _logChannels;
  std::mutex _channelsLock;
};

class PE_UTIL_API peConsoleLogChannel : public ILogChannel {
public:
  virtual void Log(LogLevel level, const char *msg) override;
};

class PE_UTIL_API peFileLogChannel : public ILogChannel {
public:
  explicit peFileLogChannel(const std::string &filePath);
  void Log(LogLevel level, const char *msg) override;

private:
  std::ofstream _fstream;
};

} // namespace pe

#pragma warning(pop)
