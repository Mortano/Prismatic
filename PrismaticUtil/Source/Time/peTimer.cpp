
#include "Time\peTimer.h"

#define SecondToMillis 1000

namespace pe {

peTimer::peTimer() {
  LARGE_INTEGER clockValues;
  QueryPerformanceFrequency(&clockValues);
  _frequency = clockValues.QuadPart;

  QueryPerformanceCounter(&clockValues);
  _startCycles = clockValues.QuadPart;
}

peTimer::peTimer(const peTimer &other) {
  _frequency = other._frequency;
  _startCycles = other._startCycles;
}

peTimer::peTimer(peTimer &&other) {
  _frequency = other._frequency;
  _startCycles = other._startCycles;
  other._frequency = 0;
  other._startCycles = 0;
}

peTimer &peTimer::operator=(const peTimer &other) {
  _frequency = other._frequency;
  _startCycles = other._startCycles;
  return (*this);
}

peTimer &peTimer::operator=(peTimer &&other) {
  _frequency = other._frequency;
  _startCycles = other._startCycles;
  other._frequency = 0;
  other._startCycles = 0;
  return (*this);
}

peTime_t peTimer::GetCyclesSinceStart() const {
  LARGE_INTEGER clockValues;
  QueryPerformanceCounter(&clockValues);
  peTime_t curCycles = clockValues.QuadPart;
  return (curCycles - _startCycles);
}

double peTimer::GetMillisSinceStart() const {
  peTime_t cycles = GetCyclesSinceStart();
  return SecondToMillis * (cycles / static_cast<double>(_frequency));
}

double peTimer::GetSecondsSinceStart() const {
  peTime_t cycles = GetCyclesSinceStart();
  return (cycles / static_cast<double>(_frequency));
}

} // namespace pe
