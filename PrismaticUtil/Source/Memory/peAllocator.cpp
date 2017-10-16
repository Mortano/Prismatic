
#include "Memory\peAllocator.h"
#include "Memory\NewDelete.inl"
#include "Memory\peLeakDetection.h"

#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>

namespace pe
{

   //------StdAllocator------

   peStdAllocator::peStdAllocator() :
      m_numAllocations(0),
      m_numFrees(0),
      m_reservedMemory(0)
   {
   }

   peStdAllocator::~peStdAllocator()
   {
      PE_ASSERT(m_numFrees == m_numAllocations);
      m_numFrees = 0;
      m_numAllocations = 0;
      m_reservedMemory = 0;
   }

   peStdAllocator* peStdAllocator::GetInstance()
   {
      static peStdAllocator s_instance;
      return &s_instance;
   }

   void* peStdAllocator::Allocate(size_t numBytes)
   {
      m_numAllocations++;
      m_reservedMemory += numBytes;
      return std::malloc(numBytes);
   }

   void peStdAllocator::Free(void* mem)
   {
      if (mem != nullptr)
      {
         m_reservedMemory -= _msize(mem);
         m_numFrees++;
         free(mem);
      }
      else
      {
         PE_FAIL("Trying to free memory that is null!");
      }
   }

   //------LeakDetectionAllocator------
   peLeakDetectionAllocator::peLeakDetectionAllocator() :
      m_numAllocations(0),
      m_numFrees(0),
      m_reservedMemory(0)
   {
      for (size_t i = 0; i < MAX_ALLOCATIONS; i++)
      {
         AllocationInfo& info = m_allocations[i];
         std::memset(info.backtrace, 0, 20 * sizeof(size_t));
         info.mem = nullptr;
         info.numTraceFrames = 0;
         info.size = 0;
      }
   }

   peLeakDetectionAllocator::~peLeakDetectionAllocator()
   {
      if (m_numFrees != m_numAllocations)
      {
         LogLeaks();
         getchar();
      }
      m_numFrees = 0;
      m_numAllocations = 0;
      m_reservedMemory = 0;
   }

   void* peLeakDetectionAllocator::Allocate(size_t size)
   {
      void* mem = malloc(size);
      PE_ASSERT(mem != nullptr);
      TrackAllocation(size, mem);
      return mem;
   }

   void peLeakDetectionAllocator::Free(void* mem)
   {
      PE_ASSERT(mem != nullptr);
      TrackFree(mem);
      free(mem);
   }

   void peLeakDetectionAllocator::TrackAllocation(size_t size, void* mem)
   {
      //Check if already allocated
      size_t hash = MemHash(mem);
      for (size_t i = hash; i < MAX_ALLOCATIONS; i++)
      {
         if (m_allocations[i].mem == nullptr) break;
         if (m_allocations[i].mem == mem)
         {
            PE_FAIL("Allocating memory that was already allocated!");
         }
      }
      constexpr uint32_t NumCallstackFrames = 16;
      AllocationInfo info;
      info.size = size;
      info.mem = mem;
      info.numTraceFrames = static_cast<uint8_t>(peStackWalker::GetCallstack(NumCallstackFrames, info.backtrace));
      //Find free index
      size_t freeIdx = hash;
      size_t reserved = 0;
      while (reserved < MAX_ALLOCATIONS)
      {
         if (m_allocations[freeIdx].mem == nullptr)
         {
            break;
         }
         freeIdx = (freeIdx + 1) % MAX_ALLOCATIONS;
         reserved++;
      }
      if (reserved == MAX_ALLOCATIONS)
      {
         PE_FAIL("Allocations list is full!");
      }
      m_allocations[freeIdx] = info;
      m_numAllocations++;
      m_reservedMemory += size;
   }

   void peLeakDetectionAllocator::TrackFree(void* mem)
   {
      size_t hash = MemHash(mem);
      size_t pos = hash;
      do
      {
         if (m_allocations[pos].mem == mem)
         {
            break;
         }
         pos = (pos + 1) % MAX_ALLOCATIONS;
         if (pos == hash)
         {
            return;
            //ASSERT(false, "Freeing memory that was never allocated!");
         }
      } while (true);
      m_allocations[pos].mem = nullptr;
      m_numFrees++;
      m_reservedMemory -= m_allocations[pos].size;
   }

   IAllocatorStatistics* peLeakDetectionAllocator::GetInstance()
   {
      static peLeakDetectionAllocator s_instance;
      return &s_instance;
   }

   size_t peLeakDetectionAllocator::MemHash(void* mem)
   {
      return reinterpret_cast<uintptr_t>(mem) % MAX_ALLOCATIONS;
   }

   void peLeakDetectionAllocator::LogLeaks()
   {
      peStackWalker::Init();
      
      //Print the leaks
      auto stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
      SetConsoleTextAttribute(stdOutHandle, 0x0C);
      for (size_t i = 0; i < MAX_ALLOCATIONS; i++)
      {
         auto& info = m_allocations[i];
         if (info.mem == nullptr) continue;
         constexpr size_t NameBufferSize = 1024;
         auto functionNames = static_cast<char*>(malloc(info.numTraceFrames * NameBufferSize));
         peStackWalker::ResolveCallstack(info.backtrace, info.numTraceFrames, functionNames, NameBufferSize);
         //GlobalMgr->GetLogging()->LogError("Leaked %i bytes! Callstack: \n%s", info.size, functionNames);
         printf("Leaked %llu bytes! \nCallstack: \n", info.size);
         OutputDebugStringA("Memory leak!\n");
         //Print the callstack separately
         char buffer[NameBufferSize];
         for (size_t j = 0; j < info.numTraceFrames; j++)
         {
            memcpy(buffer, functionNames + (j * NameBufferSize), NameBufferSize);
            printf("%s\n", buffer);
            OutputDebugStringA(buffer);
            OutputDebugStringA("\n");
         }
         free(functionNames);
      }
      SetConsoleTextAttribute(stdOutHandle, 0x07);
   }

}
