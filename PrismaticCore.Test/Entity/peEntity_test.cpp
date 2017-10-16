#include "CppUnitTest.h"
#include "Entities\Entity.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pe;

struct TestComponent : peComponent<TestComponent> {
  uint32_t testField;
};

namespace PrismaticCoreTest {
class EntityManagerTest : public TestClass<EntityManagerTest> {
public:
  TEST_METHOD(TestAddComponent_InvalidEntity_ExpectThrow) {
    peEntityManager entityManager;
    Assert::ExpectException<std::runtime_error>([&]() {
      peEntity::Handle handle{0, 0};
      entityManager.AddComponent<TestComponent>(handle);
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
    auto component = entity.GetComponent<TestComponent>();
    Assert::IsFalse(component.IsValid(),
                    L"Newly created entity must not have any components!");
  }

  TEST_METHOD(TestCreateEntity_ExpectCapacityIncreased) {
    peEntityManager entityManager;
    entityManager.CreateEntity();
    Assert::AreEqual(std::size_t{1}, entityManager.Capacity());
  }

  TEST_METHOD(TestCapacity_ExpectEmpty) {
    peEntityManager entityManager;
    Assert::AreEqual(std::size_t{0}, entityManager.Capacity(),
                     L"EntityManager capacity was wrong!");
  }

  TEST_METHOD(TestGetEntity_ExpectInvalidHandle) {
    peEntityManager entityManager;
    auto entity = entityManager.GetEntityAt(0);
    Assert::IsFalse(entity.IsAlive(), L"Expected invalid entity!");
  }
};
} // namespace PrismaticCoreTest
