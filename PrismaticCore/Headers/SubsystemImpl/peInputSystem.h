#pragma once
#include "Subsystems\IInputSystem.h"

namespace pe {

//! Default implementation of the input system
class peInputSystem : public IInputSystem {
public:
  peInputSystem() = default;
  ~peInputSystem() = default;

  void Init() override;
  void Shutdown() override;
  void Update(double deltaTime) override;

  KeyState GetKeyState(KeyCode_t key) const override;
  glm::ivec2 GetMouseDelta() const override;
  KeyState GetMouseButtonState(MouseButton mouseButton) const override;
  const peDelegate<KeyCode_t, KeyState> &GetKeyEvent() const override {
    return _keyEvent;
  }
  const peDelegate<MouseButton, KeyState> &GetMouseEvent() const override {
    return _mouseEvent;
  }

private:
  static const size_t MAX_KEYS = 256;

  KeyState _keyStates[MAX_KEYS];
  KeyState _mouseStates[static_cast<uint8_t>(MouseButton::NUM_MOUSEBUTTONS)];
  glm::ivec2 _mouseDelta;

  peDelegate<KeyCode_t, KeyState> _keyEvent;
  peDelegate<MouseButton, KeyState> _mouseEvent;
};

} // namespace pe
