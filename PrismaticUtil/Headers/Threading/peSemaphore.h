#pragma once
#include "peUtilDefs.h"

#include <Windows.h>

namespace pe {

//! Semaphore for thread synchronization
class PE_UTIL_API peSemaphore {
public:
  explicit peSemaphore(int maxCount);
  peSemaphore(int startCount, int maxCount);
  ~peSemaphore();
  //! Try to decrement the semaphore count. This waits if the current
  //! count is zero,
  void WaitAndDecrement() const;
  //! Increments the semaphore count by one
  void Increment() const;
  //! Increments the semaphore count to the given value
  void IncrementToCount(unsigned int count) const;

private:
  HANDLE m_handle;
};

}; // namespace pe
