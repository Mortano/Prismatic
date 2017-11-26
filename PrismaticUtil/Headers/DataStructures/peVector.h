#pragma once

#include "Memory/peStlAllocatorWrapper.h"
#include <algorithm>
#include <type_traits>
#include <vector>

namespace pe {

namespace detail {
/// <summary>
/// back_insert_iterator that uses emplace_back with rvalue references. More or
/// less copied from the STL implementation of back_insert_iterator
/// </summary>
template <typename Container> class back_emplace_iterator {
public:
	using iterator_category = std::output_iterator_tag;
	using value_type = typename Container::value_type;
	using difference_type = typename Container::difference_type;
	using pointer = typename Container::pointer;
	using reference = typename Container::reference; 

  using container_type = Container;
  using _Unchecked_type = back_emplace_iterator<Container>;

  explicit back_emplace_iterator(Container &_Cont)
      : container(_STD addressof(_Cont)) {}

  back_emplace_iterator &operator=(
      typename Container::value_type &&_Val) { // push value into container
    container->emplace_back(_STD move(_Val));
    return (*this);
  }

  back_emplace_iterator &operator*() { // pretend to return designated value
    return (*this);
  }

  back_emplace_iterator &operator++() { // pretend to preincrement
    return (*this);
  }

  back_emplace_iterator operator++(int) { // pretend to postincrement
    return (*this);
  }

protected:
  Container *container;
};

/// <summary>
/// Returns a new back_emplace_iterator that wraps the given container
/// </summary>
template <typename Container>
back_emplace_iterator<Container> back_emplacer(Container &cont) {
  return back_emplace_iterator<Container>(cont);
}

} // namespace detail

template <typename T> using peVector = std::vector<T, peStlAllocatorWrapper<T>>;

/// <summary>
/// In-place transform algorithm for peVector. Creates a new vector and fills it
/// with the transformed values
/// </summary>
template <typename T, typename Func>
decltype(auto) Transform(const peVector<T> &src, Func &&func) {
  using Result_t = std::decay_t<decltype(func(std::declval<T>()))>;
  peVector<Result_t> ret;
  ret.reserve(src.size());
#pragma warning(push)
#pragma warning(disable : 4996)
  std::transform(src.begin(), src.end(), detail::back_emplacer(ret),
                 std::forward<Func>(func));
#pragma warning(pop)
  return ret;
}

} // namespace pe
