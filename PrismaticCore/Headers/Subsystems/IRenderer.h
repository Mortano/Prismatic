#pragma once
#include "peCoreDefs.h"

namespace pe {

//! Interface for the renderer
struct IRenderer {
  virtual ~IRenderer() {}
  //! Initialize the renderer. Call this once during startup
  virtual void Init() = 0;
  //! Destroy the renderer. Call this once during shutdown
  virtual void Shutdown() = 0;
  //! Updates the renderer with the given delta time (in seconds)
  virtual void Update(double deltaTime) = 0;
};

} // namespace pe
