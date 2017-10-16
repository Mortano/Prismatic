#pragma once
#include "peUtilDefs.h"

#include <Windows.h>

namespace pe
{

typedef LONGLONG    peTime_t;

//! Timer class that provides access to high-precision timing
class PE_UTIL_API peTimer
{
public:
                    peTimer();
                    peTimer(const peTimer& other);
                    peTimer(peTimer&& other);

    //! Returns the cycles since the start of this timer
    peTime_t        GetCyclesSinceStart() const;
    //! Returns the seconds since the start of this timer
    double          GetSecondsSinceStart() const;
    //! Returns the milliseconds since the start of this timer
    double          GetMillisSinceStart() const;

    peTimer&        operator=(const peTimer& other);
    peTimer&        operator=(peTimer&& other);
private:
    peTime_t        _frequency;
    peTime_t        _startCycles;
};

}