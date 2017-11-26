#include "Threading\peTaskSystem.h"

pe::peTaskQueue::peTaskQueue() {}

void pe::peTaskQueue::Enqueue(std::function<void()> task) {
  std::unique_lock<std::mutex> lock{_lock};
  _tasks.push_back(std::move(task));
  _available.notify_one();
}

std::function<void()> pe::peTaskQueue::WaitAndDequeue() {
  std::unique_lock<std::mutex> lock{_lock};
  while (_tasks.empty()) {
    _available.wait(lock);
  }
  auto ret = std::move(_tasks.back());
  _tasks.pop_back();
  return std::move(ret);
}

std::optional<std::function<void()>> pe::peTaskQueue::TryDequeue() {
  std::unique_lock<std::mutex> lock{_lock};
  if (_tasks.empty())
    return {};
  auto ret = std::move(_tasks.back());
  _tasks.pop_back();
  return std::move(ret);
}

size_t pe::peTaskQueue::Count() const {
  std::unique_lock<std::mutex> lock{_lock};
  return _tasks.size();
}

pe::peTaskSystem::peTaskSystem(const uint32_t concurrency)
    : _concurrency(concurrency) {}

void pe::peTaskSystem::Start() {
  _running = true;
  _nextRunnerIdx = 0;

  _queues.reserve(_concurrency);
  for (uint32_t idx = 0; idx < _concurrency; ++idx) {
    _queues.emplace_back(std::make_unique<peTaskQueue>());
  }

  _runners.reserve(_concurrency);
  for (uint32_t idx = 0; idx < _concurrency; ++idx) {
    _runners.emplace_back([this, idx]() { Run(idx); });
    // Make sure each thread runs on its own core!
    auto &runner = _runners.back();
    const auto mask = (1 << idx);
    SetThreadAffinityMask(runner.native_handle(), mask);
  }
}

void pe::peTaskSystem::Stop() {
  _running = false;
  // Some runners may be stuck waiting on their queues, so we put dummy tasks in
  // the queues to make sure the runners will end
  for (auto &queue : _queues) {
    queue->Enqueue([]() {});
  }

  for (auto &runner : _runners)
    runner.join();

  _queues.clear();
  _runners.clear();
}

void pe::peTaskSystem::AddTask(std::function<void()> task) {
  // Different scheduling strategies are possible, since we have work-stealing
  // we can just do round-robin
  // Also, this will overflow after 2^32 tasks, but we don't really care
  const auto runnerIdx = _nextRunnerIdx.fetch_add(1u) % _concurrency;
  _queues[runnerIdx]->Enqueue(std::move(task));
}

void pe::peTaskSystem::Run(const uint32_t idx) {
  auto &queue = *_queues[idx];
  auto waitForNextTask = false;

  while (_running) {

    // 0) If desired, we wait until a task becomes available (see 3)
    if (waitForNextTask) {
      const auto task = queue.WaitAndDequeue();
      task();
    }

    // 1) Get tasks from this runners dedicated queue
    while (auto task = queue.TryDequeue()) {
      (*task)();
    }

    // 2) Nothing there, so try some work stealing
    auto nextIdx = idx;
    do {
      auto stolenTask = _queues[nextIdx]->TryDequeue();
      if (stolenTask) {
        (*stolenTask)();
        // Only ever steal one item at a time!
        break;
      }
      nextIdx = (nextIdx + 1) % _concurrency;
    } while (nextIdx != idx);

    // 3) Work stealing did not work, so we will wait for the next task of our
    // dedicated queue
    if (nextIdx == idx) {
      waitForNextTask = true;
    }
  }
}
