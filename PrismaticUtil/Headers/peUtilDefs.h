#pragma once
#include <cstdio>

#ifdef PE_UTIL_EXPORT
#define PE_UTIL_API __declspec(dllexport)
#else
#define PE_UTIL_API __declspec(dllimport)
#endif

#pragma region Assert

namespace peAssertion {

inline void FailedAssert(const char *msg, const char *file, int line) {
  // TODO Log error with logging system!
  printf("Failed assert: %s (%s, line %d)\n", msg, file, line);
}

} // namespace peAssertion

#ifdef _DEBUG
#define PE_ASSERT(cond)                                                        \
  do {                                                                         \
    if (!(cond)) {                                                             \
      peAssertion::FailedAssert(#cond, __FILE__, __LINE__);                    \
      __debugbreak();                                                          \
    }                                                                          \
  } while (0)
#define PE_FAIL(msg)                                                           \
  do {                                                                         \
    peAssertion::FailedAssert(msg, __FILE__, __LINE__);                        \
    __debugbreak();                                                            \
  } while (0)
#else
#define PE_ASSERT(cond)                                                        \
  do {                                                                         \
    (void)sizeof(cond);                                                        \
  } while (0)
#define PE_FAIL(msg)                                                           \
  do {                                                                         \
    (void)sizeof(msg);                                                         \
  } while (0)
#endif

#pragma endregion
