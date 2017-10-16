#pragma once
#include "Subsystems\IRenderer.h"
#include "peCoreDefs.h"

#include <Windows.h>

namespace pe
{

//! Class that loads the relevant dlls and returns the pointers to
//! the subsystems that are created in the dlls
class PE_CORE_API peDllLoader
{
public:
                    peDllLoader();
                    ~peDllLoader();

    IRenderer*      GetRenderer();
private:
    HINSTANCE       _rendererDll;
};

}