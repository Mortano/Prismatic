#pragma once
#include "DataStructures/peVector.h"
#include "Type\peColor.h"
#include "peUtilDefs.h"
#include <string>

namespace pe {
namespace file {

//! \brief Returns the path to the current module
//! \returns Module path
PE_UTIL_API std::string GetModulePath();

//! \brief Returns the folder in which the given file resides
//! \param filepath Full file path
//! \returns Path of the folder in which the file resides
PE_UTIL_API std::string GetFolderFromFile(const std::string &filepath);

} // namespace file

void PE_UTIL_API DumpPNG(const peVector<RGBA_8Bit> &pixels,
                         const uint32_t width, const uint32_t height,
                         const std::string &path);

} // namespace pe
