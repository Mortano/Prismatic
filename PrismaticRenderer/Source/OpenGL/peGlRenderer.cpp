
#include "OpenGL\peGlRenderer.h"

namespace pe
{

peGlRenderer::peGlRenderer()
{
}

peGlRenderer::~peGlRenderer()
{
}

void peGlRenderer::Init()
{
    _backend.Init();
}

void peGlRenderer::Shutdown()
{
    _backend.Shutdown();
}

void peGlRenderer::Update(double delta)
{
}

}