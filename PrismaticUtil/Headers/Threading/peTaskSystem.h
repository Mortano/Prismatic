#pragma once
#include "DataStructures/peVector.h"
#include "peSemaphore.h"
#include <atomic>
#include <mutex>
#include <optional>
#include <thread>

#pragma warning(push)
#pragma warning(disable : 4251)

namespace pe {

//! \brief Task queue
struct PE_UTIL_API peTaskQueue {
  peTaskQueue();

  void Enqueue(std::function<void()> task);
  std::function<void()> WaitAndDequeue();
  std::optional<std::function<void()>> TryDequeue();

  size_t Count() const;

private:
  peVector<std::function<void()>> _tasks;
  mutable std::mutex _lock;
  mutable std::condition_variable _available;
};

//! \brief Task system
class PE_UTIL_API peTaskSystem {
public:
  explicit peTaskSystem(
      const uint32_t concurrency = std::thread::hardware_concurrency());

  peTaskSystem(const peTaskSystem &) = delete;
  peTaskSystem(peTaskSystem &&) = delete;

  void Start();
  void Stop();

  void AddTask(std::function<void()> task);

  auto Concurrency() const { return _concurrency; }

private:
  void Run(const uint32_t idx);

  const uint32_t _concurrency;
  std::atomic_bool _running;
  std::atomic<uint32_t> _nextRunnerIdx;
  peVector<std::thread> _runners;
  peVector<std::unique_ptr<peTaskQueue>> _queues;
};

} // namespace pe

#pragma warning(pop)
