#pragma once
#include "Subsystems\IInputSystem.h"
#include "Subsystems\IRenderer.h"
#include "Subsystems\IUpdateSystem.h"

#include "Exceptions\peLogging.h"
#include "peCoreDefs.h"
#include "peDllLoader.h"

namespace pe {

//! Main engine class through which access to all other engine
//! subsystems happens!
class PE_CORE_API peEngine {
public:
  static peEngine &GetInstance();

  peEngine(const peEngine &) = delete;
  peEngine &operator=(const peEngine &) = delete;

  void Init();
  void Shutdown();

  //! Returns the engine log system
  peLogging *GetLogging() const { return _logger; }
  IInputSystem *GetInputSystem() const { return _inputSystem; }
  IRenderer *GetRenderer() const { return _renderer; }
  IUpdateSystem *GetUpdateSystem() const { return _updateSystem; }

private:
  peEngine();
  ~peEngine();

  peDllLoader _dllLoader;
#pragma region Subsystems
  peLogging *_logger;
  IInputSystem *_inputSystem;
  IRenderer *_renderer;
  IUpdateSystem *_updateSystem;
#pragma endregion
};

} // namespace pe

#define PrismaticEngine (pe::peEngine::GetInstance())
