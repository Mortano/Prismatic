#pragma once
#include <stdint.h>

#include "DataStructures\peUnorderedMap.h"
#include "Memory\peAllocators.h"

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief A handle to an element inside a weak table
struct PE_UTIL_API peWeakHandle {
  peWeakHandle();
  peWeakHandle(uint32_t index, uint32_t version);

  static peWeakHandle Invalid();

  uint32_t index;
  uint32_t version;
};

template <typename T> struct peWeakPtr;

//! \brief Weak table base class. Used for access of the peWeakPtr
class PE_UTIL_API peWeakTableBase {
protected:
  template <typename T> friend struct peWeakPtr;

  peWeakTableBase(size_t elementSize, size_t chunkSize,
                  IAllocator *parentAllocator);

  void *Deref(peWeakHandle handle) const;

  struct Entry {
    void *instance;
    uint32_t version;
  };
  pePoolAllocator _allocator;
  peUnorderedMap<uint32_t, Entry> _entries;
  peVector<uint32_t> _freeList;
};

//! \brief A weak table that stores objects that may expire
template <typename T> class peWeakTable : public peWeakTableBase {
public:
  peWeakTable();
  ~peWeakTable();

  template <typename... Args> peWeakPtr<T> Insert(Args &&... args);
  void Insert(T &&obj);

  void Destroy(const peWeakHandle &handle);

  template <typename U> void Destroy(const peWeakPtr<U> &ptr);

private:
  bool IsEntryAlive(const Entry &entry) const;
};

namespace {
template <typename T, typename U> struct WeakPtrToBaseHelper {
  constexpr static bool value =
      std::is_base_of<T, U>::value && !std::is_same<T, U>::value;
};
} // namespace

//! \brief Pointer to an object inside a weak table
template <typename T> struct peWeakPtr {
  using Underlying_t = std::decay_t<T>;

  peWeakPtr();
  peWeakPtr(peWeakHandle handle, peWeakTable<Underlying_t> *weakTable);

  peWeakPtr(const peWeakPtr<T> &other) = default;
  peWeakPtr<T> &operator=(const peWeakPtr<T> &other) = default;

  template <typename U, typename = std::enable_if_t<
                            WeakPtrToBaseHelper<T, std::decay_t<U>>::value>>
  peWeakPtr(const peWeakPtr<U> &other)
      : _weakHandle(other._weakHandle), _weakTable(other._weakTable) {}

  T *operator->();
  const T *operator->() const;

  T &operator*();
  const T &operator*() const;

  bool IsAlive() const;

  T *Get();
  const T *Get() const;

private:
  template <typename U> friend struct peWeakPtr;
  friend class peWeakTable<Underlying_t>;

  peWeakHandle _weakHandle;
  peWeakTableBase *_weakTable;
};

#pragma region peWeakTableImpl
template <typename T>
peWeakTable<T>::peWeakTable()
    : peWeakTableBase(sizeof(T), 8192, GlobalAllocator) {}

template <typename T> peWeakTable<T>::~peWeakTable() {
  for (auto &keyVal : _entries) {
    auto &entry = keyVal.second;
    if (!IsEntryAlive(entry))
      continue;
    reinterpret_cast<T *>(entry.instance)->~T();
    _allocator.Free(entry.instance);
  }
}

template <typename T>
template <typename... Args>
peWeakPtr<T> peWeakTable<T>::Insert(Args &&... args) {
  if (_freeList.size()) {
    auto index = _freeList.back();
    _freeList.pop_back();

    auto &entry = _entries[index];
    entry.instance = pe::New<T>(&_allocator, std::forward<Args>(args)...);

    return {{index, entry.version}, this};
  }

  const auto index = _entries.size();
  Entry entry;
  entry.version = 0;
  entry.instance = pe::New<T>(&_allocator, std::forward<Args>(args)...);
  _entries.emplace(static_cast<uint32_t>(index), entry);

  return {{static_cast<uint32_t>(index), entry.version}, this};
}

template <typename T> void peWeakTable<T>::Insert(T &&obj) {
  if (_freeList.size()) {
    auto index = _freeList.back();
    _freeList.pop_back();

    auto &entry = _entries[index];
    entry.instance = pe::New<T>(&_allocator, std::move(obj));

    return {{index, entry.version}, this};
  }

  auto index = _entries.size();
  auto iter = _entries.emplace(index);
  auto &entry = iter.first->second;
  entry.version = 0;
  entry.instance = pe::New<T>(&_allocator, std::move(obj));

  return {{index, entry.version}, this};
}

template <typename T> void peWeakTable<T>::Destroy(const peWeakHandle &handle) {
  auto iter = _entries.find(handle.index);
  if (iter == _entries.end())
    throw std::runtime_error{"Invalid handle!"};
  auto &entry = iter->second;
  if (entry.version != handle.version)
    return; // Already destroyed
  if (!IsEntryAlive(entry))
    return;

  Delete<T>(reinterpret_cast<T *>(entry.instance), &_allocator);
  ++entry.version;

  _freeList.push_back(handle.index);
}

template <typename T>
template <typename U>
void peWeakTable<T>::Destroy(const peWeakPtr<U> &ptr) {
  Destroy(ptr._weakHandle);
}

template <typename T>
bool peWeakTable<T>::IsEntryAlive(const Entry &entry) const {
  return entry.instance != nullptr;
}
#pragma endregion

#pragma region peWeakPtrImpl
template <typename T> peWeakPtr<T>::peWeakPtr() : _weakTable(nullptr) {}

template <typename T>
peWeakPtr<T>::peWeakPtr(peWeakHandle handle,
                        peWeakTable<Underlying_t> *weakTable)
    : _weakHandle(handle), _weakTable(weakTable) {}

template <typename T> T *peWeakPtr<T>::operator->() { return Get(); }

template <typename T> const T *peWeakPtr<T>::operator->() const {
  return Get();
}

template <typename T> T &peWeakPtr<T>::operator*() {
  if (!IsAlive())
    throw std::runtime_error{"WeakPtr points to dead object!"};
  auto ptr = reinterpret_cast<T *>(_weakTable->Deref(_weakHandle));
  return *ptr;
}

template <typename T> const T &peWeakPtr<T>::operator*() const {
  if (!IsAlive())
    throw std::runtime_error{"WeakPtr points to dead object!"};
  auto ptr = reinterpret_cast<T *>(_weakTable->Deref(_weakHandle));
  return *ptr;
}

template <typename T> bool peWeakPtr<T>::IsAlive() const {
  return _weakTable != nullptr && _weakTable->Deref(_weakHandle) != nullptr;
}

template <typename T> T *peWeakPtr<T>::Get() {
  if (!_weakTable)
    return nullptr;
  auto ptr = _weakTable->Deref(_weakHandle);
  return reinterpret_cast<T *>(ptr);
}

template <typename T> const T *peWeakPtr<T>::Get() const {
  if (!_weakTable)
    return nullptr;
  auto ptr = _weakTable->Deref(_weakHandle);
  return reinterpret_cast<T *>(ptr);
}
#pragma endregion

} // namespace pe

#pragma warning(pop)
