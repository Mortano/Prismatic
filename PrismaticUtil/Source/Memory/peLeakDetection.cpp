#include "Memory\peLeakDetection.h"
#include "Memory\peMemoryUtil.h"

#include <DbgHelp.h>
#include <Tlhelp32.h>
#include <Windows.h>
#include <stdio.h>

namespace pe {

bool peStackWalker::s_initialized = false;

uint32_t peStackWalker::GetCallstack(uint32_t maxFrames, size_t addresses[20]) {
  return RtlCaptureStackBackTrace(2, maxFrames, (void **)addresses, nullptr);
}

void peStackWalker::ResolveCallstack(size_t addresses[20], size_t numFrames,
                                     char *names, size_t maxNameLength) {
  HANDLE hProcess = GetCurrentProcess();
  size_t symbolSize = sizeof(IMAGEHLP_SYMBOL64) + maxNameLength;
  IMAGEHLP_SYMBOL64 *symbol = (IMAGEHLP_SYMBOL64 *)calloc(symbolSize, 1);
  PE_ASSERT(symbol != nullptr); // calloc may return null?

  symbol->SizeOfStruct = (DWORD)symbolSize;
  symbol->MaxNameLength = (DWORD)maxNameLength;

  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

  IMAGEHLP_MODULE64 moduleInfo;
  moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

  for (size_t i = 0; i < numFrames; i++) {
    char *funcName = names + (maxNameLength * i);
    *funcName = '\0';

    DWORD64 offset = 0;
    DWORD displacement = 0;

    if (addresses[i] != 0) {
      if (SymGetSymFromAddr64(hProcess, addresses[i], &offset, symbol) ==
          TRUE) {
        char undecoratedName[512];
        char *symbolName = symbol->Name;
        if (SymUnDName64(symbol, undecoratedName,
                         static_cast<DWORD>(ElementCount(undecoratedName))) ==
            TRUE) {
          symbolName = undecoratedName;
        }

        if (SymGetLineFromAddr64(hProcess, addresses[i], &displacement,
                                 &line) == TRUE) {
          sprintf_s(funcName, maxNameLength, "%s(%d): %s", line.FileName,
                    line.LineNumber, symbolName);
        } else {
          sprintf_s(funcName, maxNameLength, "%s", symbolName);
        }
      } else {
        sprintf_s(funcName, maxNameLength, "unknown %llu", addresses[i]);
      }
      funcName[maxNameLength - 1] = '\0';
    } else {
      strcpy_s(funcName, maxNameLength, "unknown");
    }
  }

  free(symbol);
}

void peStackWalker::Init() {
  HANDLE hProcess = GetCurrentProcess();
  if (!s_initialized) {
    s_initialized = true;

    DWORD symOptions = SymGetOptions();
    symOptions |= SYMOPT_LOAD_LINES;
    symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
    symOptions |= SYMOPT_DEFERRED_LOADS;
    symOptions = SymSetOptions(symOptions);

    std::string searchPath = GenerateSearchPath();
    auto res = SymInitialize(hProcess, searchPath.c_str(), TRUE);
    PE_ASSERT(res);
  }
}

std::string peStackWalker::GenerateSearchPath() {
  const char *defaultPathList[] = {"_NT_SYMBOL_PATH",
                                   "_NT_ALTERNATE_SYMBOL_PATH", "SYSTEMROOT"};

  std::string path;
  char temp[512] = {'\0'};
  DWORD len;

  for (auto i = 0; i < ElementCount(defaultPathList); i++) {
    if ((len = GetEnvironmentVariableA(
             defaultPathList[i], temp,
             static_cast<DWORD>(ElementCount(temp)))) > 0) {
      path += temp;
    }
  }
  return path;
}

} // namespace pe
