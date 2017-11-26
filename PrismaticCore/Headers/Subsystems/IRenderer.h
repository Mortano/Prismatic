#pragma once
#include "DataStructures/peWeakTable.h"
#include "peCoreDefs.h"

namespace pe {
class peRenderResource;
class peEntity;

//! Interface for the renderer
struct IRenderer {
  virtual ~IRenderer() {}
  //! Initialize the renderer. Call this once during startup
  virtual void Init() = 0;
  //! Destroy the renderer. Call this once during shutdown
  virtual void Shutdown() = 0;
  //! Updates the renderer with the given delta time (in seconds)
  virtual void Update(double deltaTime) = 0;

  //! \brief Register an entity that shall be drawn
  //! \param entity Entity that contains exactly one renderable component
  virtual void RegisterDrawableEntity(const peEntity &entity) = 0;
  //! \brief Deregisters an entity that was previously registered
  //! \param entity Entity to deregister
  virtual void DeregisterDrawableEntity(const peEntity &entity) = 0;

private:
  template <typename T, typename DataStorage> friend class peRenderResourceBase;

  //! \brief Registers a new resource that is relevant to the renderer. This
  //! will
  //!        create an internal proxy object that mirrors the resource. The
  //!        renderer will perform all relevant rendering operations using this
  //!        proxy object so that rendering operations can happen on multiple
  //!        threads
  //! \param res Resource
  virtual void
  RegisterRenderResource(const peWeakPtr<peRenderResource> &res) = 0;

  //! \brief Deregisters a resource that was previously registered
  //! \param res Resource
  virtual void
  DeregisterRenderResource(const peWeakPtr<peRenderResource> &res) = 0;
};

} // namespace pe
