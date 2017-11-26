#pragma once
#include "Subsystems\IInputSystem.h"
#include "Subsystems\IRenderer.h"
#include "Subsystems\IUpdateSystem.h"

#include "DataStructures/peUniquePtr.h"
#include "Exceptions\peLogging.h"
#include "World/peWorld.h"
#include "peCoreDefs.h"
#include "peDllLoader.h"

#pragma warning(push)
#pragma warning(disable : 4251)

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
  peWorld *GetWorld() const { return _world.get(); }

private:
  peEngine();
  ~peEngine();

  peDllLoader _dllLoader;
#pragma region Subsystems
  peLogging *_logger;
  IInputSystem *_inputSystem;
  IRenderer *_renderer;
  IUpdateSystem *_updateSystem;
  peUniquePtr<peWorld> _world;
#pragma endregion
};

PE_CORE_API IRenderer *Renderer();
PE_CORE_API peLogging *Log();
PE_CORE_API IInputSystem *InputSystem();
PE_CORE_API IUpdateSystem *UpdateSystem();

} // namespace pe

#define PrismaticEngine (pe::peEngine::GetInstance())

#pragma warning(pop)