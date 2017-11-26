#pragma once
#include "Events\peDelegate.h"
#include "peUtilDefs.h"

#include <Windows.h>

namespace pe {

//! Native window that hosts the OpenGL rendering context
class peGlWindow {
  typedef peDelegate<uint32_t, uint32_t> ResizeEvent_t;

public:
  peGlWindow();
  ~peGlWindow();

  //! Create the native window
  void Create(uint32_t width, uint32_t height);
  //! Create the OpenGL context for the native window
  void CreateRenderContext();
  //! Destroy the native window and OpenGL rendering context
  void Destroy();

  //! Swap the buffers and present the rendered image to the user
  void Present() const;
  //! Sets this window as active render target
  void SetActive() const;

  //! Height of this window in pixels
  inline uint32_t GetHeight() const { return _height; }
  //! Returns the OpenGL rendering context created for this window
  inline HGLRC GetRenderContext() const { return _glRenderContext; }
  //! Returns the resize event for this window
  inline const ResizeEvent_t &GetResizeEvent() const { return _resizeCallback; }
  //! Width of this window in pixels
  inline uint32_t GetWidth() const { return _width; }

  auto GetHandle() const { return _windowHandle; }

private:
  // Windows callback for window messages
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);

  HINSTANCE _moduleHandle;
  HWND _windowHandle;
  HDC _deviceContext;
  HGLRC _glRenderContext;

  uint32_t _width;
  uint32_t _height;
  ResizeEvent_t _resizeCallback;
};

} // namespace pe
