#pragma once
#include "peCoreDefs.h"
#include "Memory\peAllocatable.h"

namespace pe
{

//! Base class for all engine subsystem
class PE_CORE_API ISubsystem : public peAllocatable
{
public:
    virtual                 ~ISubsystem() {}

    //! Initialization method
    virtual void            Init() = 0;
    //! Shutdown method
    virtual void            Shutdown() = 0;
    //! Update method, called once per frame
    virtual void            Update(double deltaTime) = 0;
};

}