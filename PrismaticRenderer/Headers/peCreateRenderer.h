#pragma once
#include "OpenGL\peGlRenderer.h"
#include "peRendererDefs.h"

extern "C"
{
    PE_RENDERER_API pe::IRenderer* peCreateRenderer()
    {
        return new pe::peGlRenderer();
    }
}
