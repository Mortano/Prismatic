
#include "FileSystem/peFileSystemUtil.h"
#include "Memory/peMemoryUtil.h"

#include "Windows.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace pe {
namespace file {
std::string GetModulePath() {
  char buf[256];
  GetModuleFileNameA(reinterpret_cast<HINSTANCE>(&__ImageBase), buf,
                     ElementCount(buf));
  return {buf};
}

std::string GetFolderFromFile(const std::string &filepath) {
  auto lastFileSeparator = filepath.find_last_of('\\');
  if (lastFileSeparator == std::string::npos)
    return std::string();
  return {filepath.begin(), filepath.begin() + lastFileSeparator +
                                1}; //+1 so that we get the last file separator
}
} // namespace file
} // namespace pe
