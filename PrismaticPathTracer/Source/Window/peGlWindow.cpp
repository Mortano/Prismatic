
#include "Window\peGlWindow.h"
#include "Exceptions\peExceptions.h"
#include "peEngine.h"

#include <GL\glew.h>

namespace pe {

peGlWindow::peGlWindow()
    : _moduleHandle(nullptr), _windowHandle(nullptr), _deviceContext(nullptr),
      _glRenderContext(nullptr) {}

peGlWindow::~peGlWindow() {
  Destroy(); // Destroy checks on validity of the window handle!
}

void peGlWindow::Create(uint32_t width, uint32_t height) {
  _width = width;
  _height = height;

  _moduleHandle = GetModuleHandle(nullptr);

  // Create window class
  WNDCLASSEX wcex;
  auto szWinClass = "Prismatic";

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = _moduleHandle;
  wcex.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_WINLOGO));
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = 0;
  wcex.lpszClassName = szWinClass;
  wcex.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_WINLOGO));

  if (!RegisterClassEx(&wcex)) {
    throw peWindowCreationFailedException("Window class could not be created!");
  }

  // Create window
  RECT winRect = {0, 0, _width, _height};
  AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW, FALSE);
  _windowHandle = CreateWindow(szWinClass, "Prismatic", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, _width, _height,
                               NULL, NULL, _moduleHandle, NULL);
  if (!_windowHandle) {
    throw peWindowCreationFailedException(
        "Window handle could not be obtained!");
  }
  SetWindowLong(_windowHandle, GWL_STYLE, 0);
  ShowWindow(_windowHandle, SW_SHOW);
}

void peGlWindow::CreateRenderContext() {
  // Now create the OpenGL render context
  _deviceContext = GetDC(_windowHandle);

  // Set up pixelformat
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;
  int iFormat = ChoosePixelFormat(_deviceContext, &pfd);
  SetPixelFormat(_deviceContext, iFormat, &pfd);

  // Create render context
  _glRenderContext = wglCreateContext(_deviceContext);
  if (_glRenderContext == nullptr) {
    DWORD lastError = GetLastError();
    throw peWinApiErrorException(lastError);
  }
  SetActive();

  // Init glew library
  GLenum glewErr = glewInit();
  if (glewErr != GLEW_OK) {
    auto errorString = glewGetErrorString(glewErr);
    throw peWindowCreationFailedException((char *)errorString);
  }

  auto glVersion = glGetString(GL_VERSION);
  PrismaticEngine.GetLogging()->LogInfo(
      "Initialized OpenGL context with version %s", glVersion);
}

void peGlWindow::Destroy() {
  if (_windowHandle != nullptr) {
    ReleaseDC(_windowHandle, _deviceContext);

    DestroyWindow(_windowHandle);
    _windowHandle = nullptr;
    _deviceContext = nullptr;
    _glRenderContext = nullptr;
    _moduleHandle = nullptr;
  }
}

void peGlWindow::Present() const {
  SwapBuffers(_deviceContext);
  wglMakeCurrent(nullptr, nullptr);
}

void peGlWindow::SetActive() const {
  wglMakeCurrent(_deviceContext, _glRenderContext);
}

LRESULT CALLBACK peGlWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                     LPARAM lParam) {
  PAINTSTRUCT ps;
  HDC hdc;

  switch (message) {
  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

} // namespace pe
