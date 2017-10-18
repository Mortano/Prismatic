#pragma once
#include "DataStructures/pePool.h"
#include "peCoreDefs.h"
#include <bitset>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

class peEntityManager;
template <typename T> class peComponentHandle;

#pragma region peEntity

//! \brief Main entity class in the engine. Follows a standard ECS design
class PE_CORE_API peEntity {
public:
  //! \brief Handle type for entities. Uses a version counter to detect
  //! destroyed entites
  struct PE_CORE_API Handle {
    constexpr Handle(uint32_t index, uint32_t version);

    bool operator==(const Handle &other) const;
    bool operator!=(const Handle &other) const;

    //! \brief Invalid handle for entities
    static Handle Invalid();
    //! \brief Returns true if the given handle is invalid
    //! \param handle Handle
    //! \returns True if the handle is invalid
    static bool IsInvalid(const Handle &handle);

    const uint32_t index;
    const uint32_t version;
  };

  peEntity();
  peEntity(Handle handle, peEntityManager &entityManager);

  bool operator==(const peEntity &other) const;
  bool operator!=(const peEntity &other) const;

  //! \brief Destroys this entity
  void Destroy() const;

  //! \brief Returns true if this entity is still alive
  bool IsAlive() const;

  //! \brief Returns a handle for this entity
  //! \returns Handle for this entity
  auto GetHandle() const { return _handle; }

  //! \brief Adds a component of the given type to this entity. Will throw if
  //! this entity already has a component of type <paramref name="Component"/>
  //! \param args Constructor arguments to construct a component of type
  //! <paramref name="Component"/>
  //! \returns Handle to the newly created component
  template <typename Component, typename... Args>
  peComponentHandle<Component> AddComponent(Args &&... args);
  //! \brief Returns a handle to the component of type <paramref
  //! name="Component"/> assigned to this entity. Will return an invalid handle
  //! if this entity has no component of such type. \returns Handle to the
  //! componen of type <paramref name="Component"/> for this entity, or invalid
  //! handle if entity has no such component
  template <typename Component>
  peComponentHandle<Component> GetComponent() const;
  //! \brief Returns handles to the components of the given types for this
  //! entity. Will return invalid handles for each component type that is not
  //! present for this entity \returns Handles to the components of the given
  //! types for this entity
  template <typename... Components>
  std::tuple<peComponentHandle<Components>...> Components() const;

private:
  const Handle _handle;
  peEntityManager *_entityManager;
};

static_assert(sizeof(peEntity) == 16, "Entity should be 16 bytes wide!");

#pragma endregion

#pragma region peComponentHandle

//! \brief Handle for components of a given type
//! \tparam Component Component type
template <typename Component> class peComponentHandle {
public:
  peComponentHandle(peEntity::Handle entityHandle,
                    peEntityManager *entityManager)
      : _entityHandle(entityHandle), _entityManager(entityManager) {}

  Component *operator->() { return Dereference(); }
  const Component *operator->() const { return Dereference(); }

  Component &operator*() {
    if (!IsValid())
      throw std::runtime_error{"Can't dereference invalid component handle!"};
    return *Dereference();
  }
  const Component &operator*() const {
    if (!IsValid())
      throw std::runtime_error{"Can't dereference invalid component handle!"};
    return *Dereference();
  }

  //! \brief Returns a reference to the underlying component. Throws if the
  //! handle is invalid! \returns Reference to underlying component \throws
  //! runtime_error if handle is invalid
  Component &GetComponent() { return *this; }
  //! \brief Returns a const reference to the underlying component. Throws if
  //! the handle is invalid! \returns Const reference to underlying component
  //! \throws runtime_error if handle is invalid
  const Component &GetComponent() const { return *this; }

  //! \brief Returns true if this handle points to a valid component
  //! \returns True if this handle points to a valid component
  bool IsValid() const;

  //! \brief Invalid handle
  //! \returns Invalid handle
  constexpr static peComponentHandle<Component> Invalid() {
    return {peEntity::Handle::Invalid(), nullptr};
  }

private:
  friend class peEntityManager;
  Component *Dereference() const;

  const peEntity::Handle _entityHandle;
  peEntityManager *const _entityManager;
};

#pragma endregion

#pragma region Components

//! \brief Component base class, only used for the component type counter
struct PE_CORE_API peBaseComponent {
  using Family_t = std::size_t;
  constexpr static Family_t MAX_COMPONENTS =
      sizeof(Family_t) * 8; // TODO Done this way so that masking is easy,
                            // however this limits us to a relatively small
                            // number of components...

protected:
  // Counts up types of components
  static Family_t s_familyCounter;
};

//! \brief Component base class from which all component implementations should
//! inherit
template <typename T> class peComponent : public peBaseComponent {
public:
  //! \brief Returns the component family of this component
  static Family_t Family();
};

template <typename T> peBaseComponent::Family_t peComponent<T>::Family() {
  static auto s_family = s_familyCounter++;
  return s_family;
}

//! \brief Helper function to get the family for a given component type
//! \returns Family for <paramref name="Component"/>
//! \tparam Component Subclass of Component<T>
template <typename Component> peBaseComponent::Family_t GetFamilyOf() {
  static_assert(
      std::is_base_of<peComponent<Component>, Component>::value,
      "GetFamily() can only be called with a subclass of peComponent<T>!");
  return typename Component::Family();
}

//! \brief Bitmask where each set bit indicates that the component whose family
//! matches the bit index exists for a specific entity
using peComponentBitmask = std::bitset<peBaseComponent::MAX_COMPONENTS>;

namespace {
template <typename... Components> struct MakeBitmask_Helper;

template <typename First, typename... Rest>
struct MakeBitmask_Helper<First, Rest...> {
  constexpr static peBaseComponent::Family_t value =
      (peBaseComponent::Family_t{1} << GetFamilyOf<First>()) |
      MakeBitmask_Helper<Rest...>::value;
};

template <> struct MakeBitmask_Helper<> {
  constexpr static peBaseComponent::Family_t value = 0;
};
} // namespace

//! \brief Creates a bitmask that has the corresponding bits for each component
//! type set
//! \returns Component bitmask for the given components
//! \tparam Components Zero or more component types
template <typename... Components>
constexpr peComponentBitmask MakeBitmaskForComponents() {
  // TODO Easier with if constexpr or C++17 fold expressions...
  return {MakeBitmask_Helper<Components...>::value};
}

#pragma endregion

#pragma region IteratorPredicates
//! \brief Iterator predicate that checks that an entity has all of the given
//! components
template <typename... Components> struct AllOfPredicate {
  bool operator()(uint32_t index, const peEntityManager &entityManager) const;
};
#pragma endregion

#pragma region peEntityManager

namespace detail {
struct AlwaysTrue {
  template <typename... Args> constexpr bool operator()(Args &&...) const {
    return true;
  }
};
} // namespace detail

//! \brief Manages lifecycle of entities and their components. Stores all
//! component references in containers, one for each type of component. Stores a
//! bitmask which indicates which entity has which components assigned.
class PE_CORE_API peEntityManager {
public:
  //! \brief Basic iterator
  template <typename Predicate = detail::AlwaysTrue> class BaseIterator {
  public:
    BaseIterator(uint32_t startIndex, peEntityManager &entityManager)
        : _entityManager(entityManager), _index(startIndex) {}

    BaseIterator &operator++() {
      if (IsAtEnd())
        return *this;
      Next();
      return *this;
    }
    bool operator==(const BaseIterator &other) const {
      return (&other._entityManager == &_entityManager) &&
             other._index == _index;
    }

    bool operator!=(const BaseIterator &other) const {
      return !operator==(other);
    }

    bool IsAtEnd() const { return _index == _entityManager.Capacity(); }

  protected:
    peEntityManager &_entityManager;
    uint32_t _index;

  private:
    void Next() {
      // Skip all dead entities and all entities that don't match the predicate
      do {
        ++_index;
      } while (!IsAtEnd() && (!_entityManager.IsAlive(_index) ||
                              !Predicate{}(_index, _entityManager)));
    }
  };

  //! \brief Iterator for entites
  template <typename Predicate = detail::AlwaysTrue>
  class EntityIterator : public BaseIterator<Predicate> {
  public:
    EntityIterator(uint32_t startIndex, peEntityManager &entityManager)
        : BaseIterator<Predicate>(startIndex, entityManager) {}

    peEntity operator*() const {
      auto version = _entityManager._entityVersions[_index];
      return {{_index, version}, _entityManager};
    }

    auto begin() const { return *this; }
    auto end() const {
      return EntityIterator<Predicate>{
          static_cast<uint32_t>(_entityManager.Capacity()), _entityManager};
    }
  };

  //! \brief Iterator for components of a given type
  //! \tparam Component Component type
  template <typename Component, typename Predicate = detail::AlwaysTrue>
  class ComponentIterator : public BaseIterator<Predicate> {
  public:
    ComponentIterator(uint32_t startIndex, peEntityManager &entityManager)
        : BaseIterator<Predicate>(startIndex, entityManager) {}

    peComponentHandle<Component> operator*() const {
      // Assumes that the BaseIterator takes care of invalid components etc.
      auto entityHandle =
          peEntity::Handle(_index, _entityManager._entityVersions[_index]);
      return _entityManager.GetComponentForEntity<Component>(entityHandle);
    }

    auto begin() const { return *this; }
    auto end() const {
      return ComponentIterator<Component, Predicate>{_entityManager.Capacity(),
                                                     _entityManager};
    }
  };

  peEntityManager() = default;

  peEntityManager(const peEntityManager &) = delete;
  peEntityManager &operator=(const peEntityManager &) = delete;

  //! \brief Creates a new entity
  //! \returns New entity
  peEntity CreateEntity();

  //! \brief Destroys the given entity
  //! \param entityHandle Handle to an entity
  void DestroyEntity(peEntity::Handle entityHandle);

  //! \brief Returns true if the entity at the given index is alive
  //! \param index Index to an entity slot
  //! \returns True if the entitiy is alive
  bool IsAlive(uint32_t index) const;
  //! \brief Returns true if the entity with the given handle is alive
  //! \param entityHandle Handle to an entity
  //! \returns True if the entity is alive
  bool IsAlive(peEntity::Handle entityHandle) const;

  //! \brief Returns the total capacity of this entity manager
  //! \returns Capacity
  std::size_t Capacity() const;

  //! \brief Adds a component of type <paramref name="Component"/> to the given
  //! entity. Will throw if the entity already contains such a component or the
  //! entity is invalid \param entityHandle Handle to the entity \param args
  //! Optional constructor arguments for component \returns Handle to the newly
  //! created component.
  template <typename Component, typename... Args>
  peComponentHandle<Component> AddComponent(peEntity::Handle entityHandle,
                                            Args &&... args) {
    // Check if entity is valid
    if (!IsAlive(entityHandle))
      throw std::runtime_error{"Can't add component to invalid entity!"};
    if (GetComponentForEntity<Component>(entityHandle).IsValid())
      throw std::runtime_error{
          "Entity already contains a component of this type!"};
    auto family = GetFamilyOf<Component>();
    EnsureComponentPoolExists<Component>(family);
    auto &pool = _componentPools[family];
    // TODO By doing it this way, each entity occupies space for ALL possible
    // components, even if they are not used...
    // Create the new component...
    auto componentMemory = pool->Get(entityHandle.index);
    new (componentMemory) Component{std::forward<Args>()...};
    //... and set the bit in the components mask
    auto &componentsMask = _entityComponentMasks[entityHandle.index];
    componentsMask.set(family, true);
    return {entityHandle, this};
  }
  //! \brief Tries to remove the component of type <paramref name="Component"/>
  //! from the given entity \param entityHandle Handle to the entity
  template <typename Component>
  void RemoveComponent(peEntity::Handle entityHandle) {
    if (!IsAlive(entityHandle))
      throw std::runtime_error{"Can't add component to invalid entity!"};
    auto componentHandle = GetComponentForEntity<Component>(entityHandle);
    if (!componentHandle.IsValid())
      return;

    auto component = DerefComponentHandle(componentHandle);
    component->~Component();

    auto &componentsMask = _entityComponentMasks[entityHandle.index];
    componentsMask.flip(GetFamilyOf<Component>());
  }

  //! \brief Returns a handle to the component of type <paramref
  //! name="Component"/> for the given entity \param entityHandle Handle to an
  //! entity \returns Handle to the component for the entity
  template <typename Component>
  peComponentHandle<Component>
  GetComponentForEntity(peEntity::Handle entityHandle) {
    if (!IsAlive(entityHandle))
      return peComponentHandle<Component>::Invalid();
    auto &componentsMask = _entityComponentMasks[entityHandle.index];
    if (!componentsMask.test(GetFamilyOf<Component>()))
      return peComponentHandle<Component>::Invalid();
    return {entityHandle, this};
  }
  //! \brief Returns all components of the given types for the given entity
  //! \param entityHandle Handle to an entity
  //! \tparam Components Zero or more component types
  //! \returns Tuple containing handle to each component for the given entity
  template <typename... Components>
  std::tuple<peComponentHandle<Components>...>
  GetComponentsForEntity(peEntity::Handle entityHandle) const {
    return std::make_tuple(GetComponentForEntity<Components>(entityHandle)...);
  }

  //! \brief Returns an iterator to iterate over all entities
  //! \returns Iterator to iterate over all entities
  EntityIterator<> All();
  //! \brief Returns an iterator to iterate over all entites that have all of
  //! the given components assigned \tparam Components Zero or more component
  //! types \returns Iterator to iterate over all entities that have the given
  //! components assigned
  template <typename... Components> decltype(auto) AllWith() {
    return EntityIterator<AllOfPredicate<Components...>>{0, *this};
  }

  //! \brief Returns an iterator to iterate over all components of the given
  //! type \tparam Component Component type \returns Iterator to iterate over
  //! all components of type <paramref name="Component"/>
  template <typename Component>
  ComponentIterator<Component> AllComponents() const {
    return {0, *this};
  }

  //! \brief Applies the given function to each entity that has the given set of
  //! components assigned. If <paramref name="Components"/> contains no types,
  //! all entities will be enumerated \param func Function to call on each
  //! matching entity \tparam Components Zero or more component types
  template <typename... Components>
  void
  ForEach(const std::function<void(peEntity, peComponentHandle<Components>...)>
              &func) const {
    auto iter = AllWith<Components...>();
    for (auto entity : iter) {
      func(entity, GetComponentForEntity<Components>(entity.GetHandle())...);
    }
  }

  //!\brief Returns true if the given entity has all of the given components
  //! \param entityHandle Handle to an entity
  //! \returns True if the entity has all components
  //! \tparam Components Zero or more component types
  template <typename... Components>
  bool HasAllOf(peEntity::Handle entityHandle) {
    if (!IsAlive(entityHandle))
      return false;
    auto targetMask = MakeBitmaskForComponents<Components...>();
    auto &srcMask = _entityComponentMasks[entityHandle.index];
    auto combined = srcMask & targetMask;
    return combined == targetMask;
  }

  //! \brief Returns the entity at the given index
  //! \param index Entity index
  //! \returns Entity at given index, or invalid entity if it
  //! does not exist
  peEntity GetEntityAt(uint32_t index);

private:
  template <typename Component> friend class peComponentHandle;

  //! \brief Dereferences a component handle, returning a pointer to the actual
  //! component \param handle Component handle \returns Pointer to actual
  //! component or null if component does not exist
  template <typename Component>
  Component *DerefComponentHandle(const peComponentHandle<Component> &handle) {
    auto entityHandle = handle._entityHandle;
    // Check validity of entity
    if (_entityVersions[entityHandle.index] != entityHandle.version)
      return nullptr;
    auto family = GetFamilyOf<Component>();
    // Check if entity has this component
    auto &entityMask = _entityComponentMasks[entityHandle.index];
    if (!entityMask.test(family))
      return nullptr;
    auto &pool = _componentPools[family];
    return reinterpret_cast<Component *>(pool->Get(entityHandle.index));
  }

  //! \brief Makes sure that the component pool for the component of type
  //! <paramref name="Component"/> has been
  //!        created
  //! \param family Component family. We pass this in so that we don't have to
  //! look it up again
  template <typename Component>
  void EnsureComponentPoolExists(peBaseComponent::Family_t family) {
    if (_componentPools.size() <= family)
      _componentPools.resize(family + 1);
    auto &pool = _componentPools[family];
    if (!pool) {
      pool = std::make_unique<pePool<Component>>();
    }
    pool->Reserve(_entityVersions.size());
  }

  // One object pool for each type of components
  peVector<std::unique_ptr<peBasePool>> _componentPools;
  // Bitmasks for each entity that indicate which components the entity has
  peVector<peComponentBitmask> _entityComponentMasks;
  // Version indices for the entity handles. Each time an entity gets destroyed
  // the index is incremented by one, this invalidates all the handles
  peVector<uint32_t> _entityVersions;
  // List that stores the indices of all slots that are free (i.e. contain
  // destroyed entities)
  peVector<uint32_t> _freeSlots;
};

#pragma endregion

#pragma region peEntityTemplates
template <typename Component, typename... Args>
peComponentHandle<Component> peEntity::AddComponent(Args &&... args) {
  if (!IsAlive())
    throw std::runtime_error{"Can't add component to invalid entity!"};
  return _entityManager->AddComponent<Component>(_handle,
                                                 std::forward<Args>(args)...);
}

template <typename Component>
peComponentHandle<Component> peEntity::GetComponent() const {
  if (!IsAlive())
    throw std::runtime_error{"Can't add component to invalid entity!"};
  return _entityManager->GetComponentForEntity<Component>(_handle);
}

template <typename... Components>
std::tuple<peComponentHandle<Components>...> peEntity::Components() const {
  if (!IsAlive())
    throw std::runtime_error{"Can't add component to invalid entity!"};
  return _entityManager->GetComponentsForEntity<Components...>(_handle);
}
#pragma endregion

#pragma region peComponentHandleTemplates
template <typename Component>
bool peComponentHandle<Component>::IsValid() const {
  if (peEntity::Handle::IsInvalid(_entityHandle))
    return false;
  if (!_entityManager)
    return false;
  return _entityManager->IsAlive(_entityHandle);
}

template <typename Component>
Component *peComponentHandle<Component>::Dereference() const {
  return _entityManager->DerefComponentHandle(*this);
}
#pragma endregion

#pragma region IteratorPredicateTemplates
template <typename... Components>
bool AllOfPredicate<Components...>::
operator()(uint32_t index, const peEntityManager &entityManager) const {
  return entityManager.HasAllOf<Components...>(
      entityManager.GetEntityAt(index));
}
#pragma endregion

} // namespace pe

#pragma warning(pop)
