#pragma once

#include "Rendering/Utility/peBxDF.h"
#include "Type/peColor.h"
#include <gsl.h>

namespace pe {

enum class ToneMapping { Saturate, Linear, Log };

//! \brief Performs tone mapping from the given input image to a displayable
//! 8-bit RGBA output image \param result Result image \param input Input image
//! \param imageWidth Width of the image
//! \param imageHeight Height of the image
//! \param strategy The tone-mapping strategy to use
void ToneMap(gsl::span<RGBA_8Bit> result, const gsl::span<Spectrum_t> input,
             const uint32_t imageWidth, const uint32_t imageHeight,
             const ToneMapping strategy);

} // namespace pe
