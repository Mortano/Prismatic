#include "OpenGL\peGlContext.h"

namespace pe {

peGlContext &peGlContext::GetInstance() {
  static peGlContext s_instance;
  return s_instance;
}

peGlContext::peGlContext() {}

template <typename TRet, typename... Args> TRet MissingGlProc(Args...) {
  throw peGlProcMissingException("OpenGL function not available!");
}

template <typename FPtr, typename TRet, typename... Args>
FPtr LoadGlFunction_Impl(const char *funcName, TRet (*)(Args...)) {
  auto ret = wglGetProcAddress(funcName);
  if (!ret)
    return reinterpret_cast<FPtr>(&MissingGlProc<TRet, Args...>);
  return reinterpret_cast<FPtr>(ret);
}

template <typename FPtr> decltype(auto) LoadGlFunction(const char *funcName) {
  return LoadGlFunction_Impl<FPtr>(funcName, static_cast<FPtr>(nullptr));
}

void peGlContext::Init() {
  auto glVersion = glGetString(GL_VERSION);
  if (glVersion == nullptr) {
    throw peGlProcMissingException("No valid OpenGL context found!");
  }

  // TODO Set binary flags for supported features!

  _glViewportProc = LoadGlFunction<ViewportProc>("glViewport");
  _glFlushProc = LoadGlFunction<FlushProc>("glFlush");
  _glClearColorProc = LoadGlFunction<ClearColorProc>("glClearColor");
  _glClearProc = LoadGlFunction<ClearProc>("glClear");
}

} // namespace pe
