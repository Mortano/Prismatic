#pragma once

#include <algorithm>
#include <optional>

namespace pe
{
   
   namespace
   {
      template<typename T>
      struct IsOptional : std::false_type {};

      template<typename T>
      struct IsOptional<std::optional<T>> : std::true_type {};
   }

   //! \brief Simplified version for std::find that just takes a container and an element
   template<typename Cont, typename T>
   bool Contains(const Cont& container, const T& elem)
   {
      return std::find(std::begin(container), std::end(container), elem) != std::end(container);
   }   

   /// <summary>
   /// find_if implementation that takes a predicate that returns optional<T>, where T can be some arbitrary type
   /// possibly different from the container element type
   /// </summary>
   template<typename Cont, typename Func>
   auto FindFirst(const Cont& container, Func&& pred) -> std::decay_t<decltype(pred(*std::begin(container)))>
   {
      //Result type of predicate must be optional<T> for some T
      using Result_t = std::decay_t<decltype(pred(*std::begin(container)))>;
      static_assert(IsOptional<Result_t>::value, "Predicate function must return optional<T> for some T!");
      for(auto begin = std::begin(container), end = std::end(container); begin != end; ++begin)
      {
         auto res = pred(*begin);
         if (res) return res;
      }
      return std::nullopt;
   }

}
