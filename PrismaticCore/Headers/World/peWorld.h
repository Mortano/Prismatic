#pragma once
#include "Entities/Entity.h"

namespace pe {

//! \brief Encapsulates the world with all entites in it
class peWorld : public peAllocatable {
public:
  peWorld() = default;
  ~peWorld() = default;

  auto &EntityManager() { return _entityManager; }

private:
  peEntityManager _entityManager;
};

} // namespace pe
