
#include "peDllLoader.h"

#include "Exceptions/peExceptions.h"
#include "FileSystem/peFileSystemUtil.h"
#include "Memory/peMemoryUtil.h"
#include <Windows.h>

#define UnloadAndNull(dllInstance)                                             \
  if (dllInstance != nullptr) {                                                \
    ::FreeLibrary(dllInstance);                                                \
    dllInstance = nullptr;                                                     \
  }

namespace pe {

typedef pe::IRenderer *(*GetRendererFunc)();

peDllLoader::peDllLoader() : _rendererDll(nullptr) {}

peDllLoader::~peDllLoader() { UnloadAndNull(_rendererDll); }

IRenderer *peDllLoader::GetRenderer() {
  if (!_rendererDll) {
    auto curDllPath = file::GetFolderFromFile(file::GetModulePath());
    curDllPath += "PrismaticPathTracer.dll";
    _rendererDll = ::LoadLibraryA(curDllPath.c_str());
    if (_rendererDll == nullptr) {
      throw peWinApiErrorException(GetLastError());
    }
  }
  auto getRendererFunc = reinterpret_cast<GetRendererFunc>(
      ::GetProcAddress(_rendererDll, "peCreateRenderer"));
  if (getRendererFunc == nullptr) {
    UnloadAndNull(_rendererDll);
    throw peWinApiErrorException(GetLastError());
  }
  return getRendererFunc();
}

} // namespace pe
