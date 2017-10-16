#pragma once
#include "OpenGL\peGlWindow.h"
#include <thread>
#include <atomic>

namespace pe {

class peGlRendererBackend;

//! Renderer backend that executes OpenGL commands
class peGlRendererBackend {
  friend class peGlBackendThread;

public:
  peGlRendererBackend();
  peGlRendererBackend(const peGlRendererBackend &) = delete;
  peGlRendererBackend &operator=(const peGlRendererBackend &) = delete;

  void Init();
  void Shutdown();

private:
   void Run();

  peGlWindow _window;
  std::unique_ptr<std::thread> _backendThread;
  std::atomic<bool> _isRunning;
};

} // namespace pe
