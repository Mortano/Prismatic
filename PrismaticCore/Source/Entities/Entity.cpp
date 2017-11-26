#include "Entities\Entity.h"

namespace pe {

peBaseComponent::Family_t peBaseComponent::s_familyCounter = 0;

#pragma region peEntity

constexpr peEntity::Handle::Handle(uint32_t index, uint32_t version)
    : index(index), version(version) {}

bool peEntity::Handle::operator==(const Handle &other) const {
  return index == other.index && version == other.version;
}

bool peEntity::Handle::operator!=(const Handle &other) const {
  return !operator==(other);
}

peEntity::Handle peEntity::Handle::Invalid() {
  return {(std::numeric_limits<uint32_t>::max)(),
          (std::numeric_limits<uint32_t>::max)()};
}

bool peEntity::Handle::IsInvalid(const Handle &handle) {
  return handle.index == Invalid().index && handle.version == Invalid().version;
}

peEntity::peEntity() : _handle(Handle::Invalid()), _entityManager(nullptr) {}

peEntity::peEntity(Handle handle, peEntityManager &entityManager)
    : _handle(handle), _entityManager(&entityManager) {}

bool peEntity::operator==(const peEntity &other) const {
  return _entityManager == other._entityManager && (_handle == other._handle);
}

bool peEntity::operator!=(const peEntity &other) const {
  return !operator==(other);
}

void peEntity::Destroy() const { _entityManager->DestroyEntity(_handle); }

bool peEntity::IsAlive() const {
  if (!_entityManager)
    return false;
  return _entityManager->IsAlive(_handle);
}

#pragma endregion

#pragma region peEntityManager
peEntity peEntityManager::CreateEntity() {
  // If there are no free slots, we accomodate a new entity
  if (_freeSlots.empty()) {
    const auto version = 0u;
    _entityVersions.push_back(version);
    _entityComponentMasks.emplace_back();
    auto entityCount = static_cast<uint32_t>(_entityComponentMasks.size());
    for (auto &pool : _componentPools) {
      if (!pool)
        continue;
      pool->Reserve(entityCount);
    }

    return {{entityCount - 1, version}, *this};
  }

  auto freeIdx = _freeSlots.back();
  _freeSlots.pop_back();
  auto version = _entityVersions[freeIdx];
  return {{freeIdx, version}, *this};
}

void peEntityManager::DestroyEntity(peEntity::Handle entityHandle) {
  if (!IsAlive(entityHandle))
    throw std::runtime_error{"Entity is already destroyed!"};
  // Increment the version number, destroy all components and clear the
  // components mask
  ++_entityVersions[entityHandle.index];
  auto &mask = _entityComponentMasks[entityHandle.index];
  for (std::size_t idx = 0; idx < _componentPools.size(); ++idx) {
    if (!mask.test(idx))
      continue;
    auto &pool = _componentPools[idx];
    pool->DestructAt(entityHandle.index);
  }

  mask.reset();
  _freeSlots.push_back(entityHandle.index);
}

bool peEntityManager::IsAlive(uint32_t index) const {
  if (index >= Capacity())
    return false;
  // Here we don't have a version number, so we have to check if this index is
  // in the _freeSlots list
  return !std::binary_search(_freeSlots.begin(), _freeSlots.end(), index);
}

bool peEntityManager::IsAlive(peEntity::Handle entityHandle) const {
  if (peEntity::Handle::IsInvalid(entityHandle))
    return false;
  if (entityHandle.index >= _entityVersions.size())
    return false;
  auto version = _entityVersions[entityHandle.index];
  return version == entityHandle.version;
}

std::size_t peEntityManager::Capacity() const {
  return _entityComponentMasks.size();
}

peEntityManager::EntityIterator<> peEntityManager::All() {
  return EntityIterator<>{0, *this};
}

peEntity peEntityManager::GetEntityAt(uint32_t index) {
  if (!IsAlive(index))
    return {};
  return peEntity{{index, _entityVersions[index]}, *this};
}
#pragma endregion

} // namespace pe
