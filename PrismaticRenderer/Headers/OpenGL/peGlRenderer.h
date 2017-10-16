#pragma once
#include "Subsystems/IRenderer.h"

#include "OpenGL\peGlRendererBackend.h"

namespace pe
{

//! OpenGL implementation of the renderer
class peGlRenderer : public IRenderer
{
public:
                        peGlRenderer();
                        ~peGlRenderer();

    void                Init() override;
    void                Shutdown() override;
    void                Update(double delta) override;

private:
    peGlRendererBackend _backend;
};

}