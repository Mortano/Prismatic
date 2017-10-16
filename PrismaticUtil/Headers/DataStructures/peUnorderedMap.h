#pragma once

#include "Memory/peStlAllocatorWrapper.h"
#include <unordered_map>

namespace pe
{
   template<
      typename Key,
      typename Val,
      typename Hash = std::hash<Key>,
      typename Eq = std::equal_to<Key>
   >
   using peUnorderedMap = std::unordered_map<
      Key, 
      Val, 
      Hash, 
      Eq, 
      peStlAllocatorWrapper<
         std::pair<const Key, Val>
      >
   >;
}