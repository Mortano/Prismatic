#pragma once
#include <type_traits>
#include <tuple>

namespace mdv
{

namespace meta
{

#pragma region Misc

   //! \brief Always false meta type. This is needed for static asserts in some cases so that the compiler won't 
   //!        evaluate the static_assert all the time
	template<typename T> struct AlwaysFalse : std::false_type {};
   //! \brief Always false meta type using size_t. This is needed for static asserts in some cases so that the compiler
   //!        won't evaluate the static_assert all the time
	template<size_t Idx> struct AlwaysFalseNumeric : std::false_type {};

   //! \brief void_t trick
   template<typename...> using void_t = void;

   //! \brief Redirects the first given type
   template<typename First, typename...>
   struct Redirect
   {
      using type = First;
   };

   //! \brief sizeof as a meta function
   template<typename T>
   struct Sizeof :
      std::integral_constant<size_t, sizeof(T)>
   {
   };

   template<size_t Value>
   struct AsIntegralConstant
   {
      using type = std::integral_constant<size_t, Value>;
   };

   //! \brief Helper type alias that converts a size_t into a std::integral_constant that contains this value
   template<size_t Value>
   using AsIntegralConstant_t = typename AsIntegralConstant<Value>::type;

   //! \brief Converts between an actual size_t and the type size_t
   template<size_t Value>
   using AsSize_t = size_t;

   //! \brief Stores the ::value of the given type as integral_constant
   template<typename T>
   struct ToSize_t : std::integral_constant<size_t, T::value> {};

   //! \brief Helpful metafunction that ignores all arguments, can be used to expand a parameter pack (but beware 
   //!        of the unspecified execution order of arguments!)
   //! Shout out to http://florianjw.de/en/variadic_templates.html 
   template<typename... Args>
   void IgnoreReturnvalues(Args&&...) {}

#pragma endregion

#pragma region Typelist

   //! \brief Typelist structure
	template<typename... Args> struct Typelist {};
	
#pragma region At
   template<size_t, typename> struct At;

   template<size_t Idx, typename Only>
   struct At<Idx, Typelist<Only>>
   {
      static_assert(Idx == 0, "Index out of bounds!");
      using type = Only;
   };

	//! \brief Access an element of a typelist
	template<size_t Idx, typename First, typename... Last>
	struct At<Idx, Typelist<First, Last...>>
	{
		static_assert(Idx < (1 + sizeof...(Last)), "Index out of bounds!");
		using type = std::conditional_t<Idx == 0, First, typename At<Idx - 1, Typelist<Last...>>::type>;
	};

	template<size_t Idx, typename Typelist>
	using At_t = typename At<Idx, Typelist>::type;
#pragma endregion

#pragma region Size
	template<typename> struct Size;

	//! \brief Size of a typelist
	template<typename... Args>
	struct Size<Typelist<Args...>> :
		std::integral_constant<size_t, sizeof...(Args)>
	{
	};
#pragma endregion

#pragma region IndexOf
   template<typename, typename> struct IndexOf;

   //! \brief Index of a type in a typelist
   template<typename What, typename First, typename... Rest>
   struct IndexOf<What, Typelist<First, Rest...>>       
   {
   private:
      constexpr static size_t tmp = IndexOf<What, Typelist<Rest...>>::value;
   public:
#pragma warning(push)
#pragma warning(disable: 4307) //Compiler complains about integral overflow because we *might* add 1 to size_t max value. However, this will never happen because of the if check!
      constexpr static size_t value = std::is_same<What, First>::value ? 0 :
         (tmp == static_cast<size_t>(-1) ? -1 : 1 + tmp);
#pragma warning(pop)
   };

   template<typename What>
   struct IndexOf<What, Typelist<>>
   {
      constexpr static size_t value = static_cast<size_t>(-1);
   };
#pragma endregion

#pragma region First
   template<typename T> 
   struct First
   {
      static_assert( AlwaysFalse<T>::value, "First requires a typelist with at least one argument!" );
   };

   template<typename _First, typename... Rest>
   struct First<Typelist<_First, Rest...>>
   {
      using type = _First;
   };

   //! \brief Metafunction that returns the first argument of the given Typelist
   template<typename TList>
   using First_t = typename First<TList>::type;
#pragma endregion

#pragma region Last
   template<typename T>
   struct Last
   {
      static_assert( AlwaysFalse<T>::value, "Last requires a typelist with at least one argument!" );
   };

   template<typename... Front, typename Back>
   struct Last<Typelist<Front..., Back>>
   {
      using type = Back;
   };

   //! \brief Metafuntion that returns the last argument of the given Typelist
   template<typename TList>
   using Last_t = typename Last<TList>::type;
#pragma endregion

#pragma region Concat
   template<typename, typename> struct Concat {};

   template<typename... Left, typename... Right>
   struct Concat<Typelist<Left...>, Typelist<Right...>>
   {
      using type = Typelist<Left..., Right...>;
   };

   //! \brief Concatenation of two Typelists by appending Right at the end of Left
   template<typename Left, typename Right>
   using Concat_t = typename Concat<Left, Right>::type;
#pragma endregion

#pragma region Contains
   template<typename, typename> struct Contains;

   //! \brief Is a type contained within a typelist?
   template<typename What, typename First, typename... Rest>
   struct Contains<What, Typelist<First, Rest...>> :
      std::bool_constant<
         std::is_same<What, First>::value | Contains<What, Typelist<Rest...>>::value
      >
   {
   };

   //! \brief Recursion stop case for empty type list
   template<typename What>
   struct Contains<What, Typelist<>> :
      std::bool_constant<false>
   {
   };
#pragma endregion

#pragma region PushBack

   template<typename, typename> struct PushBack;

   template<typename Value, typename... Args>
   struct PushBack<Value, Typelist<Args...>>
   {
      using type = Typelist<Args..., Value>;
   };

   //! \brief PushBack metafunction for typelists
   template<typename Value, typename TList>
   using PushBack_t = typename PushBack<Value, TList>::type;

#pragma endregion

#pragma region PushFront

   template<typename, typename> struct PushFront;

   template<typename Value, typename... Args>
   struct PushFront<Value, Typelist<Args...>>
   {
      using type = Typelist<Value, Args...>;
   };

   //! \brief PushFront metafunction for typelists
   template<typename Value, typename TList>
   using PushFront_t = typename PushFront<Value, TList>::type;

#pragma endregion

#pragma region PopBack
   template<typename T> struct PopBack
   {
      static_assert( AlwaysFalse<T>::value, "PopBack requires a Typelist with at least one element!" );
   };
   
   template<typename... First, typename Last>
   struct PopBack<Typelist<First..., Last>>
   {
      using type = Typelist<First...>;
   };

   //! \brief PopBack metafunction for typelists
   template<typename TList>
   using PopBack_t = typename PopBack<TList>::type;
#pragma endregion

#pragma region PopFront
   template<typename T> 
   struct PopFront
   {
      static_assert( AlwaysFalse<T>::value, "PopFront requires a Typelist with at least one element!" );
   };

   template<typename First, typename... Rest>
   struct PopFront<Typelist<First, Rest...>>
   {
      using type = Typelist<Rest...>;
   };

   //! \brief PopFront metafunction for typelists
   template<typename TList>
   using PopFront_t = typename PopFront<TList>::type;
#pragma endregion

#pragma region Reverse

   template<typename> struct Reverse;

   template<>
   struct Reverse<Typelist<>>
   {
      using type = Typelist<>;
   };

   template<typename First, typename... Rest>
   struct Reverse<Typelist<First, Rest...>>
   {
      using type = PushBack_t < First, Reverse<Typelist<Rest...>> >;
   };

   //! \brief Reverse the given typelist
   template<typename TList>
   using Reverse_t = typename Reverse<TList>::type;

#pragma endregion

#pragma region Take
   template<size_t, typename> struct Take;

   template<size_t Count>
   struct Take<Count, Typelist<>>
   {
      using type = Typelist<>;
   };

   template<size_t Count, typename First, typename... Rest>
   struct Take<Count, Typelist<First, Rest...>>
   {
      using type = std::conditional_t<
         Count == 0,
         Typelist<>,
         Concat_t<
         Typelist<First>,
         typename Take<Count - 1, Typelist<Rest...>>::type
         >
      >;
   };

   //! \brief Take metafunction that takes the first Count elements out of the given list
   template<size_t Count, typename List>
   using Take_t = typename Take<Count, List>::type;
#pragma endregion

#pragma region MaxOf
   template<template <typename, typename> class, typename> struct MaxOf;

   //! \brief Maximum element of a typelist using a custom comparator
   template<template <typename, typename> class Comp, typename First, typename... Rest>
   struct MaxOf<Comp, Typelist<First, Rest...>>
   {
      using type = typename Comp<First, typename MaxOf<Comp, Typelist<Rest...>>::type>::type;
   };

   template<template <typename, typename> class Comp, typename Only>
   struct MaxOf<Comp, Typelist<Only>>
   {
      using type = Only;
   };

   //! \brief Type alias to get the maximum element of a typelist using a compare metafunction
   template<template <typename, typename> class Comp, typename TList>
   using MaxOf_t = typename MaxOf<Comp, TList>::type;
#pragma endregion

#pragma region Foldl

#pragma region Fold-Helpers

   template<typename, typename> struct And;

   //! \brief And template metafunction
   template<bool L, bool R>
   struct And<std::bool_constant<L>, std::bool_constant<R>>
   {
      using type = std::bool_constant<L && R>;
   };

   template<typename, typename> struct Or;

   //! \brief Or template metafunction
   template<bool L, bool R>
   struct Or<std::bool_constant<L>, std::bool_constant<R>>
   {
      using type = std::bool_constant<L || R>;
   };

#pragma endregion

   template<template<typename, typename> class, typename, typename> struct Foldl;

   template<
      template<typename, typename> class Func,
      typename Initial
   >
   struct Foldl<Func, Initial, Typelist<>>
   {
      using type = Initial;
   };

   template<
      template<typename, typename> class Func,
      typename Initial,
      typename First,
      typename... Rest
   >
   struct Foldl<Func, Initial, Typelist<First, Rest...>>
   {
      using type = typename Foldl<
                                    Func,
                                    typename Func<Initial, First>::type, //Fold from the left, the new initial value is the fold of the initial and the first value
                                    Typelist<Rest...>                    //Then we simply fold with the rest, if there is no rest the result is the initial value
                                 >::type;
   };

   //! \brief Left fold metafunction. Applies a metafunction to all elements of a typelist and folds 
   //!        them from the left, using a given initial value
   template <
      template <typename, typename> class Func, 
      typename Initial, 
      typename TList
   >
   using Foldl_t = typename Foldl<Func, Initial, TList>::type;

#pragma endregion

#pragma region Transform

   template<template<typename> class, typename> struct Transform;

   template<
      template<typename> class Func
   >
   struct Transform<Func, Typelist<>>
   {
      using type = Typelist<>;
   };

   template<
      template<typename> class Func,
      typename First,
      typename... Rest
   >
   struct Transform<Func, Typelist<First, Rest...>>
   {
      using type = PushFront_t<
         typename Func<First>::type, 
         typename Transform<Func, Typelist<Rest...>>::type
      >;
   };

   //! \brief Transform a typelist with the given metafunction
   template<
      template<typename> class Func,
      typename TList
   >
   using Transform_t = typename Transform<Func, TList>::type;

#pragma endregion

#pragma endregion

#pragma region Numberlist

   //! \brief Numberlist structure. This is the number-equivalent of Typelist. In principle, most of what Numberlist
   //!        is capable of can be achieved by using Typelist with integral_constant, however Numberlist has a nicer
   //!        syntax!
   template<size_t... Values> struct Numberlist {};

#pragma region At
   template<size_t Idx, size_t Only>
   struct At<Idx, Numberlist<Only>> :
      std::integral_constant<size_t, Only>
   {
      static_assert(Idx == 0, "Index out of bounds!");
   };

   //! \brief Access an element of a Numberlist
   template<size_t Idx, size_t First, size_t... Rest>
   struct At<Idx, Numberlist<First, Rest...>> :
      std::integral_constant<size_t, Idx == 0 ? First : At<Idx == 0 ? 0 : Idx - 1, Numberlist<Rest...>>::value>
   {
      static_assert(Idx < (1 + sizeof...(Rest)), "Index out of bounds!");
   };
#pragma endregion

#pragma region Size
   //! \brief Number of elements of a Numberlist
   template<size_t... Values>
   struct Size<Numberlist<Values...>> :
      std::integral_constant<size_t, sizeof...(Values)>
   {
   };
#pragma endregion

#pragma region Sum
   template<typename> struct Sum;

   //! \brief Sum of the elements of an empty Numberlist, which of course is zero
   template<>
   struct Sum<Numberlist<>> : std::integral_constant<size_t, 0> {};

   //! \brief Sum of the elements of a Numberlist
   template<size_t First, size_t... Rest>
   struct Sum<Numberlist<First, Rest...>> :
      std::integral_constant<size_t, First + Sum<Numberlist<Rest...>>::value>
   {
   };
#pragma endregion

#pragma region PushBack
   template<size_t, typename> struct NumberlistPushBack {};

   //! \brief PushBack for numberlists. Sadly, since the first argument has to be a size_t, we can't reuse the
   //!        ordinary PushBack here. It would kinda work with integral_constant, however we can't redefine the
   //!        type alias PushBack_t with a size_t as first argument
   template<size_t Value, size_t... Args>
   struct NumberlistPushBack<Value, Numberlist<Args...>>
   {
      using type = Numberlist<Args..., Value>;
   };

   //! \brief PushBack metafunction for Numberlists
   template<size_t Value, typename Numberlist>
   using NumberlistPushBack_t = typename NumberlistPushBack<Value, Numberlist>::type;
#pragma endregion

#pragma region Concat
   template<size_t... Left, size_t... Right>
   struct Concat<Numberlist<Left...>, Numberlist<Right...>>
   {
      using type = Numberlist<Left..., Right...>;
   };
#pragma endregion

#pragma region Take
   template<size_t Count>
   struct Take<Count, Numberlist<>>
   {
      using type = Numberlist<>;
   };

   template<size_t Count, size_t First, size_t... Rest>
   struct Take<Count, Numberlist<First, Rest...>>
   {
      using type = std::conditional_t<
         Count == 0,
         Numberlist<>,
         Concat_t<
            Numberlist<First>, 
            typename Take<Count - 1, Numberlist<Rest...>>::type
         >
      >;
   };
#pragma endregion

#pragma region AsTuple
   template<typename> struct AsTuple {};

   template<size_t... Values>
   struct AsTuple<Numberlist<Values...>>
   {
      using type = std::tuple<AsSize_t<Values>...>;
   };

   //! \brief Converts between a parameter pack of size_t into a tuple of size_t
   //!
   //! Example: The parameter pack is <1,2,3>. The resulting tuple is std::tuple<size_t, size_t, size_t>
   template<size_t... Values>
   using AsTuple_t = typename AsTuple<Numberlist<Values...>>::type;
#pragma endregion

#pragma region FromTypelist
   template<typename> struct NumberlistFromTypelist {};

   template<typename... Args>
   struct NumberlistFromTypelist<Typelist<Args...>>
   {
      using type = Numberlist<ToSize_t<Args>::value...>;
   };

   //! \brief Creates a Numberlist from a Typelist by creating an instance from it and using the implicit conversion
   //!        to size_t.
   template<typename Typelist>
   using NumberlistFromTypelist_t = typename NumberlistFromTypelist<Typelist>::type;
#pragma endregion

#pragma endregion

}

}