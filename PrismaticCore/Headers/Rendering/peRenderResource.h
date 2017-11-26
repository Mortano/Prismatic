#pragma once
#include "DataStructures\peWeakTable.h"
#include "Type/peTypes.h"
#include "peEngine.h"

#include "Type/RAII.h"
#include <stdint.h>

namespace pe {

template <typename T, typename DataStorage> class peRenderResourceBase;

namespace detail {
//! \brief Helper class that has the DataStorage as member only if the
//! DataStorage is not void
template <typename DataStorage> struct DataStorageHelper {
  DataStorage _dataStorage;
};

template <> struct DataStorageHelper<void> {};

} // namespace detail

//! \brief Base class for all resources relevant to rendering. Instances of this
//! class will be available outside of the renderer to all other subsystems
class PE_CORE_API peRenderResource : peNonCopyable {
public:
  auto IsDirty() const { return _dirty; }

protected:
  peRenderResource() = default;
  void MarkDirty();

  template <typename T> static peWeakTable<T> &WeakTable();

private:
  bool _dirty;
};

//! \brief Actual base class for render resources. Use this instead of
//! peRenderResource!
//! \tparam T The class inheriting from this class (as by CRTP)
//! \tparam DataStorage A class that holds the data for this render resource
template <typename T, typename DataStorage = void>
class peRenderResourceBase : public peRenderResource,
                             detail::DataStorageHelper<DataStorage> {
public:
  //! \brief Handle type through which render resources should be accessed
  using Handle_t = peWeakPtr<T>;

  //! \brief Creates a new render resource
  //! \param args Constructor arguments
  //! \returns Weak handle to new resource
  template <typename... Args> static peWeakPtr<T> New(Args &&... args) {
    auto inst = WeakTable<T>().Insert(std::forward<Args>(args)...);
    Renderer()->RegisterRenderResource(inst);
    return inst;
  }

  //! \brief Destroys the given render resource
  //! \param resource Resource
  static void Destroy(const peWeakPtr<T> &resource) {
    Renderer()->DeregisterRenderResource(resource);
    WeakTable<T>().Destroy(resource);
  }

protected:
  //! \brief Call this method to obtain a mutable reference to the data storage
  //! of this render resource. This will make sure that the rendering subsystem
  //! will see all changes made to this resource \returns Mutable reference to
  //! data storage
  std::enable_if_t<!std::is_same_v<DataStorage, void>, RAII<DataStorage>>
  GetMutableDataStorage();

  //! \brief Returns a const refrence to the data storage
  //! \returns Const reference to data storage
  std::enable_if_t<!std::is_same_v<DataStorage, void>, const DataStorage &>
  GetDataStorage() const;

private:
  peRenderResourceBase() = default;

  friend T;
};

#pragma region peRenderResourceBaseImpl

template <typename T> peWeakTable<T> &peRenderResource::WeakTable() {
  static peWeakTable<T> s_instance;
  return s_instance;
}

template <typename T, typename DataStorage>
std::enable_if_t<!std::is_same_v<DataStorage, void>, RAII<DataStorage>>
peRenderResourceBase<T, DataStorage>::GetMutableDataStorage() {
  return {_dataStorage, [this](DataStorage &) { MarkDirty(); }};
}

template <typename T, typename DataStorage>
std::enable_if_t<!std::is_same_v<DataStorage, void>, const DataStorage &>
peRenderResourceBase<T, DataStorage>::GetDataStorage() const {
  return _dataStorage;
}

#pragma endregion

} // namespace pe
