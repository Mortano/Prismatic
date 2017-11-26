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
  //! \brief Tries to decrement the count by one. Immediately returns false if
  //! it can't be decremented
  //! \returns True if decrement was succesfull
  bool TryDecrement() const;
  //! Increments the semaphore count by one
  void Increment() const;
  //! Increments the semaphore count to the given value
  void IncrementToCount(unsigned int count) const;

private:
  HANDLE m_handle;
};

}; // namespace pe
