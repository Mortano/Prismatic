
#include "OpenGL\peGlRendererBackend.h"
#include "peRendererDefs.h"
#include "peEngine.h"

#include "OpenGL\peGlContext.h"

namespace pe
{

#pragma region Backend

peGlRendererBackend::peGlRendererBackend()    
{
}

void peGlRendererBackend::Init()
{
    _window.Create(800, 600);
    _backendThread = std::make_unique<std::thread>([this]()
    {
       _isRunning = true;

       _window.CreateRenderContext();
       _window.SetActive();
       GL_CONTEXT.Init();

       while (_isRunning) { Run(); }

       _window.Destroy();
    });
}

void peGlRendererBackend::Shutdown()
{
   _isRunning = false;
   _backendThread->join();
   _backendThread = nullptr;
}

void peGlRendererBackend::Run()
{
   _window.SetActive();

   GL_CONTEXT.glViewport(0, 0, _window.GetWidth(), _window.GetHeight());

   GL_CONTEXT.glClearColor(1, 0, 0, 1);
   GL_CONTEXT.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   GL_CONTEXT.glFlush();

   _window.Present();
}

#pragma endregion

}