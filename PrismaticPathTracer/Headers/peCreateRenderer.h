#pragma once
#include "pePathTracingRenderer.h"
#include "peRendererDefs.h"

extern "C" {
PE_RENDERER_API pe::IRenderer *peCreateRenderer() {
  return new pe::pePathTracingRenderer{};
}
}
