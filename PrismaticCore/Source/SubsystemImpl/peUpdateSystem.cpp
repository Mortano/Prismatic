
#include "SubsystemImpl\peUpdateSystem.h"
#include "peEngine.h"
#include <vector>

namespace pe {

peUpdateSystem::peUpdateSystem()
    : _callbacks(WrapAllocator<std::function<void(float)>>(GlobalAllocator)),
      _isRunning(false) {}

peUpdateSystem::~peUpdateSystem() {}

void peUpdateSystem::Init() {}

void peUpdateSystem::Run() {
  PrismaticEngine.GetInputSystem()->GetKeyEvent() +=
      [this](KeyCode_t key, KeyState state) {
        if (key == VK_ESCAPE && state == KeyState::Pressed)
          _isRunning = false;
      };

  _isRunning = true;
  while (_isRunning) {
    double deltaTime = 0;
    PrismaticEngine.GetInputSystem()->Update(deltaTime); // TODO Delta time

    PrismaticEngine.GetRenderer()->Update(deltaTime);
  }
  // TODO Stop and wait for renderer thread / other threads?
}

void peUpdateSystem::Shutdown() {}

void peUpdateSystem::Stop() { _isRunning = false; }

CallbackID
peUpdateSystem::RegisterUpdateCallback(std::function<void(float)> callback) {
  for (size_t i = 0; i < _callbacks.size(); i++) {
    if (_callbacks[i] == nullptr) {
      _callbacks[i] = callback;
      return CallbackID(i);
    }
  }
  _callbacks.push_back(callback);
  return CallbackID(_callbacks.size());
}

void peUpdateSystem::DeregisterUpdateCallback(const CallbackID &callbackID) {
  PE_ASSERT(callbackID.GetID() < _callbacks.size());
  _callbacks[callbackID.GetID()] = nullptr;
}

} // namespace pe
