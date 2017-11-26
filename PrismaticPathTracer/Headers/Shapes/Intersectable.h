#pragma once
#include "Math/AABB.h"

namespace pe {
class peCoordSys;
struct Ray;

//! \brief Type-erased structure for intersectable objects
struct Intersectable {

  template <typename T, typename = std::enable_if_t<
                            !std::is_same_v<std::decay_t<T>, Intersectable>>>
  explicit Intersectable(T intersectable) {
    Emplace(std::forward<T>(intersectable));
  }

  Intersectable(const Intersectable &other);
  ~Intersectable();

  Intersectable(Intersectable &&) = delete;
  Intersectable &operator=(Intersectable &&) = delete;

  bool Intersects(const Ray &ray, glm::vec3 *hitPos, glm::vec3 *hitNormal,
                  peCoordSys *shadingCoordinateSystem) const;
  AABB GetBounds() const;

private:
  struct HolderBase {
    virtual ~HolderBase() {}

    virtual void Clone(void *mem) const = 0;

    virtual bool Intersects(const Ray &ray, glm::vec3 *hitPos,
                            glm::vec3 *hitNormal,
                            peCoordSys *shadingCoordinateSystem) const = 0;
    virtual AABB GetBounds() const = 0;
  };

  template <typename T> struct HolderImpl : HolderBase {
    explicit HolderImpl(T obj) : _obj(obj) {
      int i = 5;
      i = 7;
    }
    ~HolderImpl() override {}

    bool Intersects(const Ray &ray, glm::vec3 *hitPos, glm::vec3 *hitNormal,
                    peCoordSys *shadingCoordinateSystem) const override {
      return _obj.Intersects(ray, hitPos, hitNormal, shadingCoordinateSystem);
    }
    AABB GetBounds() const override { return _obj.GetBounds(); }
    void Clone(void *mem) const override { new (mem) HolderImpl<T>(_obj); }

    T _obj;
  };

  HolderBase &GetHolder() const;

  template <typename T> void Emplace(T &&obj) {
    static_assert(sizeof(HolderImpl<T>) <= SBOSize,
                  "Object is too big to be type-erased!");
    new (std::addressof(_storage)) HolderImpl<T>(std::forward<T>(obj));
  }

  constexpr static size_t SBOSize = 32;
  mutable std::aligned_storage_t<SBOSize> _storage;
  bool _deleted = false;
};
} // namespace pe
