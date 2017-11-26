#pragma once
#include "Events\peDelegate.h"
#include "ISubsystem.h"
#include "peCoreDefs.h"
#include "peUtilDefs.h"

#include <glm/common.hpp>

namespace pe {

//! State of a mouse or keyboard key
enum class KeyState : uint8_t { NotPressed, Pressed };

//! Different mouse buttons
enum class MouseButton : uint8_t { Left = 0, Middle, Right, NUM_MOUSEBUTTONS };

typedef uint8_t KeyCode_t;

//! Interface for the engine input system that captures mouse and keyboard input
class IInputSystem : public ISubsystem {
public:
  virtual ~IInputSystem() {}
  //! Returns the state of the given key
  virtual KeyState GetKeyState(KeyCode_t key) const = 0;
  //! Returns the last mouse position
  virtual glm::ivec2 GetMouseDelta() const = 0;
  //! Returns the state of the given mouse button
  virtual KeyState GetMouseButtonState(MouseButton mouseButton) const = 0;

  //! Returns the key event. Add a callback here to get notified when a key
  //! state change occurs
  virtual const peDelegate<KeyCode_t, KeyState> &GetKeyEvent() const = 0;
  //! Returns the mouse event. Add a callback here to get notified when a mouse
  //! state change occurs
  virtual const peDelegate<MouseButton, KeyState> &GetMouseEvent() const = 0;
};

} // namespace pe
