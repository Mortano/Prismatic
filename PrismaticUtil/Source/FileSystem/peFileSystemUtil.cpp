
#include "FileSystem/peFileSystemUtil.h"
#include "Memory/peMemoryUtil.h"

#include "FileSystem/lodepng.h"
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
void DumpPNG(const peVector<RGBA_8Bit> &pixels, const uint32_t width,
             const uint32_t height, const std::string &path) {
  std::vector<unsigned char> buf;
  buf.resize(width * height * 4);
  std::memcpy(buf.data(), pixels.data(),
              width * height * sizeof(pe::RGBA_8Bit));

  std::vector<unsigned char> out;

  auto err = lodepng::encode(out, buf, width, height);
  if (!err)
    lodepng::save_file(out, path);
}
} // namespace pe
