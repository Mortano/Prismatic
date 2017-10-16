#pragma once
#include "peAllocator.h"
#include "peUtilDefs.h"

namespace pe
{

   //! Tracks and resolves callstacks for memory leak detection
   class PE_UTIL_API peStackWalker
   {
   public:
      static void		   Init();
      static uint32_t   GetCallstack(uint32_t maxFrames, size_t addresses[20]);
      static void		   ResolveCallstack(size_t addresses[20], size_t numFrames, char* functionNames, size_t maxNameLength);
   private:
      static bool		s_initialized;

      static std::string GenerateSearchPath();
   };

}