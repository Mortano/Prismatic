#pragma once
#include "peUtilDefs.h"

namespace pe
{

class IAllocator;

//! Baseclass for all objects that get allocated via an allocator
class PE_UTIL_API peAllocatable
{
public:
                  peAllocatable();
   IAllocator*    GetAllocator() const { return _allocator; }
   void           SetAllocator(IAllocator* allocator) { _allocator = allocator; }
protected:
    IAllocator*   _allocator;
};

}
