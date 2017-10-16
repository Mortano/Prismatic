#include "peEngine.h"
#include "SubsystemImpl\peInputSystem.h"
#include "SubsystemImpl\peUpdateSystem.h"


#include <cstdlib>

namespace pe {

peEngine &peEngine::GetInstance() {
  static peEngine s_instance;
  return s_instance;
}

peEngine::peEngine()
    : _logger(nullptr), _inputSystem(nullptr), _renderer(nullptr),
      _updateSystem(nullptr) {}

peEngine::~peEngine() {
  PE_ASSERT(_logger == nullptr);
  PE_ASSERT(_renderer == nullptr);
}

void peEngine::Init() {
  _logger = New<peLogging>(GlobalAllocator);
  _logger->RegisterLogChannel(New<peConsoleLogChannel>(GlobalAllocator));

  _logger->LogInfo("Initializing renderer...");
  _renderer = _dllLoader.GetRenderer();
  _renderer->Init();
  _logger->LogInfo("Renderer initialized!");

  _logger->LogInfo("Initializing input system...");
  _inputSystem = New<peInputSystem>(GlobalAllocator);
  _inputSystem->Init();
  _logger->LogInfo("Input system initialized!");

  _logger->LogInfo("Initializing update system...");
  _updateSystem = New<peUpdateSystem>(GlobalAllocator);
  _updateSystem->Init();
  _logger->LogInfo("Update system initialized!");
}

void peEngine::Shutdown() {
  if (_updateSystem != nullptr) {
    _updateSystem->Shutdown();
    DeleteAndNull(_updateSystem);
  }

  if (_inputSystem != nullptr) {
    _inputSystem->Shutdown();
    DeleteAndNull(_inputSystem);
  }

  if (_renderer != nullptr) {
    _renderer->Shutdown();
    delete _renderer;
    _renderer = nullptr;
  }

  if (_logger != nullptr) {
    DeleteAndNull(_logger);
  }
}

} // namespace pe
