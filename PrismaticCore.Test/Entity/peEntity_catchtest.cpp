#include "catch.hpp"

#include "Entities\Entity.h"
#include <iostream>

using namespace pe;

struct TestComponent1 : peComponent<TestComponent1> {
  uint32_t testField;
};

struct TestComponent2;

struct TestComponent2System : peComponentSystem<TestComponent2System> {
  static uint32_t s_createCalls;
  static uint32_t s_destroyCalls;
  static uint32_t s_updateCalls;
  static std::function<void(TestComponent2 &, const peEntity &)> s_updateHook;

  static void ClearCallCount();

  using Component_t = TestComponent2;

  void OnCreate(TestComponent2 &component, const peEntity &entity) {
    ++s_createCalls;
  }

  void OnDestroy(TestComponent2 &component, const peEntity &entity) {
    ++s_destroyCalls;
  }

  void OnUpdate(TestComponent2 &component, const peEntity &entity) {
    ++s_updateCalls;
    if (s_updateHook)
      s_updateHook(component, entity);
  }
};

static_assert(
    detail::HasOnCreate<TestComponent2System>::value,
    "OnCreate method on TestComponent2System is not recognized correctly!");
static_assert(
    detail::HasOnDestroy<TestComponent2System>::value,
    "OnDestroy method on TestComponent2System is not recognized correctly!");
static_assert(
    detail::HasOnUpdate<TestComponent2System>::value,
    "OnUpdate method on TestComponent2System is not recognized correctly!");

uint32_t TestComponent2System::s_createCalls = 0;
uint32_t TestComponent2System::s_destroyCalls = 0;
uint32_t TestComponent2System::s_updateCalls = 0;
std::function<void(TestComponent2 &, const peEntity &)>
    TestComponent2System::s_updateHook;

void TestComponent2System::ClearCallCount() {
  s_createCalls = s_destroyCalls = s_updateCalls = 0;
}

struct TestComponent2 : peComponent<TestComponent2> {
  using System_t = TestComponent2System;
  std::string testField;
};

struct CleanupHandler {
  ~CleanupHandler() {
    auto alloc = GlobalAllocator;
    if (alloc->GetNumAllocations() != alloc->GetNumFrees()) {
      std::cerr << "Found memory leaks:" << std::endl;
      alloc->LogLeaks([](const char *msg) { std::cerr << msg << std::endl; });
    }
  }
};

CleanupHandler g_cleanupHandler;

TEST_CASE("All() yields no entities on empty entity manager",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto all = entityManager.All();
  for (auto &entity : all) {
    FAIL("peEntityManager::All() should not yield any entities!");
  }
}

TEST_CASE("All() iterates in correct order", "[peEntityManager]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  auto e2 = entityManager.CreateEntity();
  std::vector<peEntity> expectedEntities{e2, e1};
  for (auto &entity : entityManager.All()) {
    REQUIRE(expectedEntities.back() == entity);
    expectedEntities.pop_back();
  }
  REQUIRE(expectedEntities.empty());
}

TEST_CASE("Adding component to invalid entity throws exception",
          "[peEntityManager]") {
  peEntityManager entityManager;
  peEntity::Handle handle{0, 0};
  REQUIRE_THROWS_AS(entityManager.AddComponent<TestComponent1>(handle),
                    std::runtime_error);
}

TEST_CASE("CreateEntity succeeds", "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  REQUIRE(entity.IsAlive());
}

TEST_CASE("CreateEntity does not create any components", "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  auto c1 = entity.GetComponent<TestComponent1>();
  REQUIRE_FALSE(c1.IsValid());

  auto c2 = entity.GetComponent<TestComponent2>();
  REQUIRE_FALSE(c2.IsValid());
}

TEST_CASE("CreateEntity increases capacity", "[peEntityManager]") {
  peEntityManager entityManager;
  entityManager.CreateEntity();
  REQUIRE(entityManager.Capacity() == std::size_t{1});
}

TEST_CASE("CreateEntity does not disturb previous entities and components",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  auto comp = e1.AddComponent<TestComponent1>();
  constexpr uint32_t expectedTestField = 23;
  comp->testField = expectedTestField;
  auto e2 = entityManager.CreateEntity();
  SECTION("The new entity is valid") { REQUIRE(e2.IsAlive()); }

  // This should also preserve all existing components
  SECTION("All existing components were preserved") {
    auto e1TestComponent = e1.GetComponent<TestComponent1>();
    REQUIRE(e1TestComponent.IsValid());
    REQUIRE(expectedTestField == e1TestComponent->testField);
  }
}

TEST_CASE("Capacity is zero on default-constructed entity manager",
          "[peEntityManager]") {
  peEntityManager entityManager;
  REQUIRE(entityManager.Capacity() == std::size_t{0});
}

TEST_CASE("Creating entity after destroying previous entity",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  entityManager.DestroyEntity(entity.GetHandle());
  auto newEntity = entityManager.CreateEntity();
  REQUIRE(newEntity.IsAlive());
  SECTION("The new entity gets created in the slot of the previous entity") {
    REQUIRE(newEntity.GetHandle().index == 0u);
    REQUIRE(newEntity.GetHandle().version == 1u);
  }
}

TEST_CASE("DestroyEntity actually destroys the entity", "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  entityManager.DestroyEntity(entity.GetHandle());
  REQUIRE_FALSE(entity.IsAlive());
}

TEST_CASE("DestroyEntity also destroys its components", "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  auto component1 = entity.AddComponent<TestComponent1>();
  auto component2 = entity.AddComponent<TestComponent2>();
  entityManager.DestroyEntity(entity.GetHandle());
  REQUIRE_FALSE(component1.IsValid());
  REQUIRE_FALSE(component2.IsValid());
}

TEST_CASE("DestroyEntity destroys the correct components",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  auto component1 = entity.AddComponent<TestComponent1>();
  entityManager.DestroyEntity(entity.GetHandle());
  REQUIRE_FALSE(component1.IsValid());
}

TEST_CASE("DestroyEntity does not affect other entities", "[peEntityManager]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  e1.AddComponent<TestComponent1>();
  auto e2 = entityManager.CreateEntity();
  auto component = e2.AddComponent<TestComponent2>();
  entityManager.DestroyEntity(e2.GetHandle());
  REQUIRE_FALSE(component.IsValid());
  SECTION("Other entities components are not affected") {
    REQUIRE(e1.GetComponent<TestComponent1>().IsValid());
  }
}

TEST_CASE("DestroyEntity on dead entity throws", "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  entityManager.DestroyEntity(entity.GetHandle());
  REQUIRE_THROWS_AS(entityManager.DestroyEntity(entity.GetHandle()),
                    std::runtime_error);
}

TEST_CASE("GetEntityAt yields invalid handle when no entities exist",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.GetEntityAt(0);
  REQUIRE_FALSE(entity.IsAlive());
}

TEST_CASE("GetEntityAt yields valid handle for existing entity",
          "[peEntityManager]") {
  peEntityManager entityManager;
  entityManager.CreateEntity();
  auto entity = entityManager.GetEntityAt(0);
  REQUIRE(entity.IsAlive());
}

TEST_CASE("IsAlive yields false for invalid entity handle",
          "[peEntityManager]") {
  peEntityManager entityManager;
  REQUIRE_FALSE(entityManager.IsAlive(peEntity::Handle::Invalid()));
}

TEST_CASE("IsAlive yields false for out-of-bounds index", "[peEntityManager]") {
  peEntityManager entityManager;
  REQUIRE_FALSE(entityManager.IsAlive(0));
}

TEST_CASE("IsAlive yields true for living entity", "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  REQUIRE(entityManager.IsAlive(entity.GetHandle()));
}

TEST_CASE("IsAlive yields true for index of living entity",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto entity = entityManager.CreateEntity();
  REQUIRE(entityManager.IsAlive(0));
}

TEST_CASE("IsAlive yields false for dead entity, even when other entities are "
          "present",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  for (auto idx = 0; idx < 5; idx++)
    entityManager.CreateEntity();
  e1.Destroy();
  REQUIRE_FALSE(entityManager.IsAlive(0));
}

TEST_CASE("AllWith iterates all components in the correct order",
          "[peEntityManager]") {
  peEntityManager entityManager;

  auto e1 = entityManager.CreateEntity();
  e1.AddComponent<TestComponent1>();
  auto e2 = entityManager.CreateEntity();
  e2.AddComponent<TestComponent2>();
  auto e3 = entityManager.CreateEntity();
  e3.AddComponent<TestComponent1>();
  e3.AddComponent<TestComponent2>();

  std::vector<peEntity> expectedWithComponent1 = {e3, e1};
  std::vector<peEntity> expectedWithComponent2 = {e3, e2};
  std::vector<peEntity> expectedWithBothComponents = {e3};

  auto verifyIteration = [](auto iter, std::vector<peEntity> &expected) {
    for (auto &entity : iter) {
      REQUIRE(expected.back() == entity);
      expected.pop_back();
    }
    SECTION("All components were iterated") { REQUIRE(expected.empty()); }
  };

  verifyIteration(entityManager.AllWith<TestComponent1>(),
                  expectedWithComponent1);
  verifyIteration(entityManager.AllWith<TestComponent2>(),
                  expectedWithComponent2);
  verifyIteration(entityManager.AllWith<TestComponent1, TestComponent2>(),
                  expectedWithBothComponents);
}

TEST_CASE(
    "AddComponent creates the components system when called for the first time",
    "[peEntityManager]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  e1.AddComponent<TestComponent2>();
  REQUIRE_NOTHROW(entityManager.GetComponentSystem<TestComponent2>());
}

TEST_CASE("AddComponent calls OnCreate method on component system",
          "[peEntityManager]") {
  TestComponent2System::ClearCallCount();
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  e1.AddComponent<TestComponent2>();
  REQUIRE(TestComponent2System::s_createCalls == 1u);
  REQUIRE(TestComponent2System::s_destroyCalls == 0u);
}

TEST_CASE("RemoveComponent calls OnDestroy method on component system",
          "[peEntityManager]") {
  TestComponent2System::ClearCallCount();
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  e1.AddComponent<TestComponent2>();
  entityManager.RemoveComponent<TestComponent2>(e1.GetHandle());
  REQUIRE(TestComponent2System::s_destroyCalls == 1u);
}

TEST_CASE("GetComponentSystem on empty entity manager will throw",
          "[peEntityManager]") {
  peEntityManager entityManager;
  // Using TestComponent1 here would result in a compile error because this
  // component type doesn't define a system
  REQUIRE_THROWS_AS(entityManager.GetComponentSystem<TestComponent2>(),
                    std::runtime_error);
}

TEST_CASE("UpdateAll calls update on all relevant entities",
          "[peEntityManager]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  auto comp = e1.AddComponent<TestComponent2>();
  const std::string expectedTestField = "ExpectedTestField";
  comp->testField = expectedTestField;

  TestComponent2System::s_updateHook = [&](auto &c, const auto &entity) {
    REQUIRE(entity == e1);
    REQUIRE(c.testField == expectedTestField);
  };
  TestComponent2System::ClearCallCount();
  entityManager.GetComponentSystem<TestComponent2>().OnUpdateAll();
  REQUIRE(TestComponent2System::s_updateCalls == 1u);
}

#pragma region EntityHandle

TEST_CASE("Equality operators work when handles are equal",
          "[peEntity::Handle]") {
  peEntity::Handle h1{23, 42};
  peEntity::Handle h2{23, 42};
  REQUIRE(h1 == h2);
  REQUIRE_FALSE(h1 != h2);
}

TEST_CASE("Equality operators work when handle indices are not equal",
          "[peEntity::Handle]") {
  peEntity::Handle h1{23, 42};
  peEntity::Handle h2{5, 42};
  REQUIRE_FALSE(h1 == h2);
  REQUIRE(h1 != h2);
}

TEST_CASE("Equality operators work when handle versions are not equal",
          "[peEntity::Handle]") {
  peEntity::Handle h1{23, 42};
  peEntity::Handle h2{23, 17};
  REQUIRE_FALSE(h1 == h2);
  REQUIRE(h1 != h2);
}

#pragma endregion

#pragma region Entity

TEST_CASE("Equality operators work on equal entities", "[peEntity]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  auto e1Copy = entityManager.GetEntityAt(0);
  REQUIRE(e1 == e1Copy);
  REQUIRE_FALSE(e1 != e1Copy);
}

TEST_CASE("Equality operators work on entities that are not equal",
          "[peEntity]") {
  peEntityManager entityManager;
  auto e1 = entityManager.CreateEntity();
  auto e2 = entityManager.CreateEntity();
  REQUIRE_FALSE(e1 == e2);
  REQUIRE(e1 != e2);
}

#pragma endregion
