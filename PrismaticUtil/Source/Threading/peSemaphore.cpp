#include "Threading\peSemaphore.h"

namespace pe {

peSemaphore::peSemaphore(int maxCount) {
  m_handle = CreateSemaphore(nullptr, 0, maxCount, nullptr);
}

peSemaphore::peSemaphore(int startCount, int maxCount) {
  m_handle = CreateSemaphore(nullptr, startCount, maxCount, nullptr);
}

peSemaphore::~peSemaphore() { CloseHandle(m_handle); }

void peSemaphore::Increment() const { ReleaseSemaphore(m_handle, 1, nullptr); }

void peSemaphore::IncrementToCount(unsigned int count) const {
  ReleaseSemaphore(m_handle, count, nullptr);
}

void peSemaphore::WaitAndDecrement() const {
  WaitForSingleObject(m_handle, INFINITE);
}

bool peSemaphore::TryDecrement() const {
  const auto res = WaitForSingleObject(m_handle, 0);
  return res == WAIT_OBJECT_0;
}
} // namespace pe
