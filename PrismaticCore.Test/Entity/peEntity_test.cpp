#include "CppUnitTest.h"
#include "Entities\Entity.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pe;

struct TestComponent1 : peComponent<TestComponent1> {
  uint32_t testField;
};

struct TestComponent2 : peComponent<TestComponent2> {
  std::string testField;
};

template <>
std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<peEntity>(
    const peEntity &entity) {
  std::wstringstream ss;
  ss << "Entity [ID: " << entity.GetHandle().index
     << "; Version: " << entity.GetHandle().version << "]";
  return ss.str();
}

namespace PrismaticCoreTest {
class EntityManagerTest : public TestClass<EntityManagerTest> {
public:
  TEST_CLASS_CLEANUP(Cleanup) {
    auto alloc = GlobalAllocator;
    if (alloc->GetNumAllocations() != alloc->GetNumFrees()) {
      alloc->LogLeaks([](const char *msg) { Logger::WriteMessage(msg); });
      Assert::Fail(L"Memory leaks!");
    }
  }

  TEST_METHOD(TestAll_ExpectEmpty) {
    peEntityManager entityManager;
    auto all = entityManager.All();
    for (auto &entity : all) {
      Assert::Fail(L"peEntityManager::All() should not yield any entities!");
    }
  }

  TEST_METHOD(TestAll_NonEmpty_ExpectCorrectIteration) {
    peEntityManager entityManager;
    auto e1 = entityManager.CreateEntity();
    auto e2 = entityManager.CreateEntity();
    std::vector<peEntity> expectedEntities{e2, e1};
    for (auto &entity : entityManager.All()) {
      Assert::AreEqual(expectedEntities.back(), entity,
                       L"Wrong iteration order on All()!");
      expectedEntities.pop_back();
    }
    Assert::IsTrue(expectedEntities.empty(),
                   L"All() does not iterate all entities!");
  }

  TEST_METHOD(TestAddComponent_InvalidEntity_ExpectThrow) {
    peEntityManager entityManager;
    Assert::ExpectException<std::runtime_error>([&]() {
      peEntity::Handle handle{0, 0};
      entityManager.AddComponent<TestComponent1>(handle);
    });
  }

  TEST_METHOD(TestCreateEntity_ExpectSuccess) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    Assert::IsTrue(entity.IsAlive(), L"Newly created entity must be alive!");
  }

  TEST_METHOD(TestCreateEntity_ExpectHasNoComponents) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    auto component = entity.GetComponent<TestComponent1>();
    Assert::IsFalse(component.IsValid(),
                    L"Newly created entity must not have any components!");
  }

  TEST_METHOD(TestCreateEntity_ExpectCapacityIncreased) {
    peEntityManager entityManager;
    entityManager.CreateEntity();
    Assert::AreEqual(std::size_t{1}, entityManager.Capacity());
  }

  TEST_METHOD(TestCreateEntity_AfterEntityWithComponents_ExpectSuccess) {
    peEntityManager entityManager;
    auto e1 = entityManager.CreateEntity();
    auto comp = e1.AddComponent<TestComponent1>();
    constexpr uint32_t expectedTestField = 23;
    comp->testField = expectedTestField;
    auto e2 = entityManager.CreateEntity();
    Assert::IsTrue(e2.IsAlive(), L"Expected valid entity!");

    // This should also preserve all existing components
    auto e1TestComponent = e1.GetComponent<TestComponent1>();
    Assert::IsTrue(e1TestComponent.IsValid(), L"Component was not preserved!");
    Assert::AreEqual(expectedTestField, e1TestComponent->testField,
                     L"Component data was not preserved!");
  }

  TEST_METHOD(TestCapacity_ExpectEmpty) {
    peEntityManager entityManager;
    Assert::AreEqual(std::size_t{0}, entityManager.Capacity(),
                     L"EntityManager capacity was wrong!");
  }

  TEST_METHOD(TestCreateEntity_AfterDestroyedEntity_ExpectSuccess) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    entityManager.DestroyEntity(entity.GetHandle());
    auto newEntity = entityManager.CreateEntity();
    Assert::IsTrue(newEntity.IsAlive(), L"Entity should be alive!");
    Assert::AreEqual(0u, newEntity.GetHandle().index, L"Wrong index!");
    Assert::AreEqual(1u, newEntity.GetHandle().version,
                     L"Version not incremented after delete/create combo!");
  }

  TEST_METHOD(TestDestroyEntity_ExpectIsDead) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    entityManager.DestroyEntity(entity.GetHandle());
    Assert::IsFalse(entity.IsAlive(), L"Destroy does not destroy entity!");
  }

  TEST_METHOD(TestDestroyEntity_WithAllComponents_ExpectComponentsDestroyed) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    auto component1 = entity.AddComponent<TestComponent1>();
    auto component2 = entity.AddComponent<TestComponent2>();
    entityManager.DestroyEntity(entity.GetHandle());
    Assert::IsFalse(component1.IsValid(),
                    L"Component1 not destroyed when entity was destroyed!");
    Assert::IsFalse(component2.IsValid(),
                    L"Component2 not destroyed when entity was destroyed!");
  }

  TEST_METHOD(TestDestroyEntity_WithSomeComponents_ExpectComponentsDestroyed) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    auto component1 = entity.AddComponent<TestComponent1>();
    entityManager.DestroyEntity(entity.GetHandle());
    Assert::IsFalse(component1.IsValid(),
                    L"Component1 not destroyed when entity was destroyed!");
  }

  TEST_METHOD(TestGetEntity_ExpectInvalidHandle) {
    peEntityManager entityManager;
    auto entity = entityManager.GetEntityAt(0);
    Assert::IsFalse(entity.IsAlive(), L"Expected invalid entity!");
  }

  TEST_METHOD(TestGetEntity_ExpectValidHandle) {
    peEntityManager entityManager;
    entityManager.CreateEntity();
    auto entity = entityManager.GetEntityAt(0);
    Assert::IsTrue(entity.IsAlive(), L"Expected valid entity!");
  }

  TEST_METHOD(TestIsAlive_InvalidHandle_ExpectFalse) {
    peEntityManager entityManager;
    Assert::IsFalse(entityManager.IsAlive(peEntity::Handle::Invalid()),
                    L"Invalid handle must not point to valid entity!");
  }

  TEST_METHOD(TestIsAlive_OutOfBoundsIndex_ExpectFalse) {
    peEntityManager entityManager;
    Assert::IsFalse(entityManager.IsAlive(0),
                    L"Entity at out-of-bounds index must not be alive!");
  }

  TEST_METHOD(TestIsAlive_OnValidEntity_ExpectTrue) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    Assert::IsTrue(entityManager.IsAlive(entity.GetHandle()),
                   L"Entity should be alive!");
  }

  TEST_METHOD(TestIsAlive_OnValidEntityIndex_ExpectTrue) {
    peEntityManager entityManager;
    auto entity = entityManager.CreateEntity();
    Assert::IsTrue(entityManager.IsAlive(0),
                   L"Entity at index 0 should be alive!");
  }
};
} // namespace PrismaticCoreTest
