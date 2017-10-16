#pragma once
#include "ISubsystem.h"
#include <functional>

namespace pe {

//! ID to identify update callbacks
class CallbackID {
public:
  inline CallbackID(size_t id) : _id(id) {}
  inline size_t GetID() const { return _id; }

private:
  size_t _id;
};

//! Main update system that manages the updating process of all other subsystems
class IUpdateSystem : public ISubsystem {
public:
  virtual ~IUpdateSystem() {}
  //! Is the game running?
  virtual bool IsRunning() const = 0;
  //! Runs the game
  virtual void Run() = 0;
  //! Stops the game
  virtual void Stop() = 0;

  //! Registers a custom update callback for the game loop
  virtual void DeregisterUpdateCallback(const CallbackID &id) = 0;
  //! Deregisters the update callback with the given ID
  virtual CallbackID
  RegisterUpdateCallback(std::function<void(float delta)> callback) = 0;
};

} // namespace pe
