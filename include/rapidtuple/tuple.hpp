/* The MIT License (MIT)

Copyright (c) 2015 jonathan poelen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef RAPIDTUPLE_TUPLE_HPP
#define RAPIDTUPLE_TUPLE_HPP

#include <tuple> //std::ignore_t, std::tuple_size, std::tuple_element
#include <utility>
#include <type_traits>
#include <initializer_list>

#include <falcon/cxx/cxx.hpp>
#include <brigand/brigand.hpp>


namespace falcon {

using std::size_t;

struct as_const_fn
{
  template<class T>
  T const &
  operator()(T const & x) const noexcept
  { return x; }
};

#ifndef IN_IDE_PARSER
constexpr as_const_fn as_const;
#else
  int const & as_const(int const & x) noexcept;
#endif

using allocator_arg_t = std::allocator_arg_t;
constexpr allocator_arg_t allocator_arg {};

template<class T>
using unref_t = std::remove_reference_t<T>;

template<class T>
using uncv_t = std::remove_cv_t<T>;

template<class T>
using uncvref_t = uncv_t<unref_t<T>>;

// template<class...>
// using void_t = void;

#if __cplusplus < FALCON_CXX_STD_14
template<size_t i, class Tuple>
using tuple_element_t = typename std::tuple_element<i, Tuple>::type;
#else
using std::tuple_element_t;
#endif

#if __cplusplus > FALCON_CXX_STD_14
using std::is_nothrow_swappable;
#else
namespace detail_
{
  namespace swappable_details
  {
    using std::swap;

    template<class T>
    static brigand::bool_<
      noexcept(swap(std::declval<T&>(), std::declval<T&>()))
    > test(int);

    template<class>
    static std::false_type test(...);
  }
}

template<class T>
using is_nothrow_swappable = decltype(detail_::swappable_details::test<T>(0));
#endif

namespace detail_
{
  template<class T, class U>
  using is_same = typename std::is_same<T, U>::type;

  template<class T, class U>
  using is_convertible = typename std::is_convertible<T, U>::type;

  template<class T, class... U>
  using is_constructible
    = typename std::is_constructible<T, U...>::type;

  template<class T, class U>
  using is_nothrow_constructible
    = typename std::is_nothrow_constructible<T, U>::type;

  template<class T, class U>
  using is_nothrow_assignable
    = typename std::is_nothrow_constructible<T, U>::type;

  template<class T>
  using is_default_constructible
    = typename std::is_default_constructible<T>::type;

  template<class T>
  using is_nothrow_default_constructible
    = typename std::is_nothrow_default_constructible<T>::type;

  template<class T>
  using is_copy_assignable
    = typename std::is_copy_assignable<T>::type;

  template<class T>
  using is_copy_constructible
    = typename std::is_copy_constructible<T>::type;

  template<class T>
  using is_nothrow_copy_assignable
    = typename std::is_nothrow_copy_assignable<T>::type;

  template<class T>
  using is_nothrow_copy_constructible
    = typename std::is_nothrow_copy_constructible<T>::type;

  template<class T>
  using is_move_assignable
    = typename std::is_move_assignable<T>::type;

  template<class T>
  using is_nothrow_move_assignable
    = typename std::is_nothrow_move_assignable<T>::type;

  template<size_t n>
  struct is_same_as_pack_impl
  {
    template<class...> using is_same = std::false_type;
    template<class...> using is_same_uncvref = std::false_type;
  };

  template<>
  struct is_same_as_pack_impl<1>
  {
    template<class T, class U>
    using is_same = is_same<T, U>;

    template<class T, class U>
    using is_same_uncvref = is_same<T, uncvref_t<U>>;
  };

  template<class T, class... Us>
  using is_same_as_pack = typename is_same_as_pack_impl<sizeof...(Us)>
    ::template is_same<T, Us...>;

  template<class T, class... Us>
  using is_same_as_uncvref_pack = typename is_same_as_pack_impl<sizeof...(Us)>
    ::template is_same_uncvref<T, Us...>;
}

namespace detail_
{
  using std::get;

  template<class... Bool>
  using mpl_all = is_same<
    brigand::list<Bool...>,
    brigand::filled_list<std::true_type, sizeof...(Bool)>
  >;

  template<bool b, class T, class U>
  using mpl_if_else_c = typename std::conditional<b, T, U>::type;

  template<class B, class T, class U>
  using mpl_if_else = mpl_if_else_c<B::value, T, U>;

  template<size_t... Ints>
  struct tuple_indexes {};

  template<class... Ts>
  using range_for = brigand::range<size_t, 0, sizeof...(Ts)>;

  template<class>
  struct mpl_list_to_tuple_indexes;

  template<class... Ints>
  struct mpl_list_to_tuple_indexes<brigand::list<Ints...>>
  { using type = tuple_indexes<Ints::value...>; };

  template<class... Ts>
  using tuple_indexes_for = typename
    mpl_list_to_tuple_indexes<range_for<Ts...>>::type;

  template<class T, class U>
  struct add_const_if_copy_reference
  { using type = U; };

  template<class T>
  struct add_const_if_copy_reference<T, T&>
  { using type = T const &; };

  template<class T, class U>
  using add_const_if_copy_reference_t
    = typename add_const_if_copy_reference<T, U>::type;

  template<class Ints, class... Ts>
  struct tuple_impl;

  template<
    size_t I, class T,
    bool = std::is_empty<T>::value && !std::is_final<T>::value
  >
  struct tuple_leaf;

  template<size_t I, class T>
  struct tuple_leaf<I, T, true>
  : private T
  {
    constexpr tuple_leaf() = default;

    template<
      class Alloc,
      std::enable_if_t<
        !std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    tuple_leaf(allocator_arg_t, Alloc const &)
    : T()
    {}

    template<
      class Alloc,
      std::enable_if_t<
        std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    tuple_leaf(allocator_arg_t, Alloc const & a)
    : T(a)
    {}

    tuple_leaf(tuple_leaf const &) = default;
    tuple_leaf(tuple_leaf &&) = default;

    template<
      class U,
      std::enable_if_t<
        !std::is_same<uncvref_t<U>, tuple_leaf>::value,
        bool
      > = true
    >
    constexpr explicit tuple_leaf(U && v)
    noexcept(std::is_nothrow_constructible<
      T, add_const_if_copy_reference_t<T, U>>::value
    )
    : T(std::forward<add_const_if_copy_reference_t<T, U>>(v))
    {}

    template<
      class Alloc, class U,
      std::enable_if_t<
        !std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    explicit tuple_leaf(allocator_arg_t, Alloc const &, U && v)
    : T(std::forward<add_const_if_copy_reference_t<T, U>>(v))
    {}

    template<
      class Alloc, class U,
      std::enable_if_t<
        std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    explicit tuple_leaf(allocator_arg_t, Alloc const & a, U && v)
    : T(std::forward<add_const_if_copy_reference_t<T, U>>(v), a)
    {}


    tuple_leaf & operator=(tuple_leaf const & t) = delete;
    tuple_leaf & operator=(tuple_leaf && t) = delete;

    template<class U>
    void operator=(U && v)
    noexcept(std::is_nothrow_assignable<
      T&, add_const_if_copy_reference_t<T, U>
    >::value)
    { T::operator=(std::forward<add_const_if_copy_reference_t<T, U>>(v)); }


    int
    swap(tuple_leaf & t)
    noexcept(is_nothrow_swappable<tuple_leaf>::value)
    {
      using std::swap;
      swap(get(), t);
      return 0;
    }

    constexpr T       & get()       noexcept { return static_cast<T       &>(*this); }
    constexpr T const & get() const noexcept { return static_cast<T const &>(*this); }
  };

  template<size_t I, class T>
  class tuple_leaf<I, T, false>
  {
    T value;

public:
    constexpr tuple_leaf()
    noexcept(std::is_nothrow_default_constructible<T>::value)
    : value()
    {}

    tuple_leaf(tuple_leaf const &) = default;
    tuple_leaf(tuple_leaf &&) = default;

    template<
      class Alloc,
      std::enable_if_t<
        !std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    tuple_leaf(allocator_arg_t, Alloc const &)
    : value()
    {}

    template<
      class Alloc,
      std::enable_if_t<
        std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    tuple_leaf(allocator_arg_t, Alloc const & a)
    : value(a)
    {}

    template<
      class U,
      std::enable_if_t<
        !std::is_same<uncvref_t<U>, tuple_leaf>::value,
        bool
      > = true
    >
    constexpr explicit tuple_leaf(U && v)
    noexcept(std::is_nothrow_constructible<
      T, add_const_if_copy_reference_t<T, U>>::value
    )
    : value(std::forward<add_const_if_copy_reference_t<T, U>>(v))
    {}

    template<
      class Alloc, class U,
      std::enable_if_t<
        !std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    explicit tuple_leaf(allocator_arg_t, Alloc const &, U && v)
    : value(std::forward<add_const_if_copy_reference_t<T, U>>(v))
    {}

    template<
      class Alloc, class U,
      std::enable_if_t<
        std::uses_allocator<T, Alloc>::value,
        bool
      > = true
    >
    explicit tuple_leaf(allocator_arg_t, Alloc const & a, U && v)
    : value(std::forward<add_const_if_copy_reference_t<T, U>>(v), a)
    {}


    tuple_leaf & operator=(tuple_leaf const & t) = delete;
    tuple_leaf & operator=(tuple_leaf && t) = delete;

    template<class U>
    void operator=(U && v)
    noexcept(std::is_nothrow_assignable<
      T&, add_const_if_copy_reference_t<T, U>>::value
    )
    { value = std::forward<add_const_if_copy_reference_t<T, U>>(v); }

    int swap(tuple_leaf & t)
    noexcept(is_nothrow_swappable<tuple_leaf>::value)
    {
      using std::swap;
      swap(*this, t);
      return 0;
    }

    constexpr T       & get()       noexcept { return value; }
    constexpr T const & get() const noexcept { return value; }
  };

  template<size_t... Ints, class... Ts>
  class tuple_impl<tuple_indexes<Ints...>, Ts...>
  : public tuple_leaf<Ints, Ts>...
  {
    template<class... Us>
    using pack_expands_to_this_tuple
      = is_same_as_uncvref_pack<tuple_impl, Us...>;

  public:
    tuple_impl() = default;
    tuple_impl(tuple_impl const &) = default;
    tuple_impl(tuple_impl &&) = default;

    template<
      class... Us,
      std::enable_if_t<
        !pack_expands_to_this_tuple<Us...>::value,
        bool
      > = true
    >
    explicit constexpr tuple_impl(Us && ... v)
    noexcept(mpl_all<is_nothrow_constructible<Ts, Us>...>::value)
    : tuple_leaf<Ints, Ts>(std::forward<Us>(v))...
    {}

    template<class Alloc,class... Us>
    explicit constexpr tuple_impl(allocator_arg_t, Alloc const & a, Us && ... v)
    : tuple_leaf<Ints, Ts>(allocator_arg_t(), a, std::forward<Us>(v))...
    {}

    template<class Tuple>
    // TODO PERF specialize for tuple
    explicit constexpr tuple_impl(tuple_indexes<Ints...>, Tuple && t)
    noexcept(mpl_all<
      is_nothrow_constructible<Ts, decltype(get<Ints>(
        std::forward<Tuple>(t)
      ))>...
    >::value)
    : tuple_leaf<Ints, Ts>(get<Ints>(std::forward<Tuple>(t)))...
    {}

    template<class Alloc,class Tuple>
    // TODO PERF specialize for tuple
    explicit constexpr tuple_impl(
      allocator_arg_t, Alloc const & a,
      tuple_indexes<Ints...>, Tuple && t)
    : tuple_leaf<Ints, Ts>(
      allocator_arg_t(), a,
      get<Ints>(std::forward<Tuple>(t))
    )...
    {}

    tuple_impl & operator=(tuple_impl const &) = delete;
    tuple_impl & operator=(tuple_impl &&) = delete;

    template<class Tuple>
    // TODO PERF specialize for tuple
    FALCON_CONSTEXPR_AFTER_CXX11
    void assign(tuple_indexes<Ints...>, Tuple && t)
    noexcept(noexcept(FALCON_UNPACK(
      std::declval<tuple_leaf<Ints, Ts>&>()
      = get<Ints>(std::forward<Tuple>(t))
    )))
    {
      FALCON_UNPACK(
        static_cast<tuple_leaf<Ints, Ts>&>(*this)
        = get<Ints>(std::forward<Tuple>(t))
      );
    }

    void swap(tuple_impl & t)
    noexcept(detail_::mpl_all<is_nothrow_swappable<Ts>...>::value)
    {
      using std::swap;
      FALCON_UNPACK(swap(
        static_cast<tuple_leaf<Ints, Ts>&>(*this).get(),
        static_cast<tuple_leaf<Ints, Ts>&>(t).get()
      ));
    }
  };

  template<class Tuple, class Indexes>
  struct tuple_to_list;

  template<class Tuple, class... Ints>
  struct tuple_to_list<Tuple, brigand::list<Ints...>>
  { using type = brigand::list<std::tuple_element_t<Ints::value, Tuple>...>; };

  template<class Tuple, class... Ints>
  struct tuple_to_list<Tuple &, brigand::list<Ints...>>
  { using type = brigand::list<std::tuple_element_t<Ints::value, Tuple> & ...>; };

  template<class Tuple, class... Ints>
  struct tuple_to_list<Tuple &&, brigand::list<Ints...>>
  { using type = brigand::list<std::tuple_element_t<Ints::value, Tuple> && ...>; };

  template<class Tuple>
  using range_for_tuple = brigand::range<
    size_t, 0, std::tuple_size<uncvref_t<Tuple>>::value
  >;

  template<class Tuple,class Indexes = range_for_tuple<Tuple>>
  using tuple_to_list_t = typename tuple_to_list<Tuple, Indexes>::type;
}

namespace detail_
{
  template<class T, size_t I>
  brigand::size_t<I>
  tuple_index_of_impl(detail_::tuple_leaf<I, T> const &);


  template<class... i>
  struct index_sequence_from
  { using type = std::integer_sequence<std::size_t, i::value...>; };

  template<class T, class Tuple>
  struct tuple_indexes_of_impl;

  template<class T, std::size_t... ints, class... Ts>
  struct tuple_indexes_of_impl<T, tuple_impl<tuple_indexes<ints...>, Ts...>>
  : brigand::wrap<
    brigand::append<
      detail_::mpl_if_else<
        detail_::is_same<T, Ts>,
        brigand::list<brigand::size_t<ints>>,
        brigand::list<>
      >...
    >,
    detail_::index_sequence_from
  >
  {};


  template<class Ints, class Tuple>
  struct tuple_to_tuple_impl;

  template<class... Ints, class Tuple>
  struct tuple_to_tuple_impl<brigand::list<Ints...>, Tuple>
  {
    using type = detail_::tuple_impl<
      tuple_indexes<Ints::value...>,
      tuple_element_t<Ints::value, Tuple>...
    >;
  };
}

template<class T, class Tuple>
struct tuple_index_of
: decltype(detail_::tuple_index_of_impl<T>(std::declval<
  typename detail_::tuple_to_tuple_impl<
    brigand::range<size_t, 0, std::tuple_size<Tuple>::value>,
    Tuple
  >::type const &
>()))
{};

template<class T, class Tuple>
struct tuple_indexes_of
: detail_::tuple_indexes_of_impl<
  T,
  typename detail_::tuple_to_tuple_impl<
    brigand::range<size_t, 0, std::tuple_size<Tuple>::value>,
    Tuple
  >::type
> {};


template<class T, class Tuple>
struct tuple_index_of<T, Tuple const>
: tuple_index_of<T, Tuple>::type
{};

template<class T, class Tuple>
struct tuple_index_of<T, Tuple volatile>
: tuple_index_of<T, Tuple>::type
{};

template<class T, class Tuple>
struct tuple_index_of<T, Tuple const volatile>
: tuple_index_of<T, Tuple>::type
{};

template<class T, class Tuple>
using tuple_index_of_t = typename tuple_index_of<T, Tuple>::type;

#if __cplusplus > FALCON_CXX_STD_14
template<class T>
constexpr size_t tuple_index_of_v = tuple_index_of<T>::value;
#endif


template<class T, class Tuple>
struct tuple_indexes_of<T, Tuple const>
: tuple_indexes_of<T, Tuple>::type
{};

template<class T, class Tuple>
struct tuple_indexes_of<T, Tuple volatile>
: tuple_indexes_of<T, Tuple>::type
{};

template<class T, class Tuple>
struct tuple_indexes_of<T, Tuple const volatile>
: tuple_indexes_of<T, Tuple>::type
{};

template<class T, class Tuple>
using tuple_indexes_of_t = typename tuple_indexes_of<T, Tuple>::type;

#if __cplusplus > FALCON_CXX_STD_14
template<class T>
constexpr size_t tuple_indexes_of_v = tuple_indexes_of<T>::value;
#endif

namespace detail_
{
  template<bool, class T, class Alloc, class... Args>
  struct is_allocator_extended_constructible_impl
  : std::is_constructible<T, Args..., Alloc const &>
  {};

  template<class T, class Alloc, class... Args>
  struct is_allocator_extended_constructible_impl<false, T, Alloc, Args...>
  : std::is_constructible<T, Args...>
  {};

  template<class T, class Alloc>
  struct is_allocator_extended_constructible_impl<false, T, Alloc>
  : std::is_default_constructible<T>
  {};


  template<class T>
  std::true_type is_implicitly_convertible(int, T const &);

  struct any
  {
    template<class... Ts>
    any(Ts const & ...);
  };

  template<class T>
  std::false_type is_implicitly_convertible(char, any);


  template<bool, class T, class Alloc, class... Args>
  struct is_allocator_extended_implicitly_constructible_impl
#ifndef IN_IDE_PARSER
  : decltype(is_implicitly_convertible<T>(
    1, {std::declval<Args>()..., std::declval<Alloc const &>()}
  ))
#endif
  {};

  template<class T, class Alloc>
  struct is_allocator_extended_implicitly_constructible_impl<true, T, Alloc>
  : std::is_convertible<Alloc, T>
  {};

  template<class T, class Alloc, class U>
  struct is_allocator_extended_implicitly_constructible_impl<false, T, Alloc, U>
  : std::is_convertible<U, T>
  {};

  template<class T, class Alloc>
  struct is_allocator_extended_implicitly_constructible_impl<false, T, Alloc>
  : std::is_default_constructible<T>
  {};

  template<class T, class Alloc>
  struct is_allocator_extended_implicitly_constructible_impl<false, T, Alloc, T&>
  : std::is_copy_constructible<T>
  {};

  template<class T, class Alloc, class... Args>
  struct is_allocator_extended_constructible
  : detail_::is_allocator_extended_constructible_impl<
    std::uses_allocator<T, Alloc>::value, T, Alloc, Args...
  >
  {};

  template<class T, class Alloc, class... Args>
  using is_allocator_extended_constructible_t
    = typename is_allocator_extended_constructible<T, Alloc, Args...>::type;

  template<class T, class Alloc, class... Args>
  struct is_allocator_extended_implicitly_constructible
  : detail_::is_allocator_extended_implicitly_constructible_impl<
    std::uses_allocator<T, Alloc>::value, T, Alloc, Args...
  >
  {};

  template<class T, class Alloc, class... Args>
  using is_allocator_extended_implicitly_constructible_t
    = typename is_allocator_extended_implicitly_constructible<
      T, Alloc, Args...>::type;

  template<class T, class... Us>
  struct is_extended_same
  : std::false_type
  {};
  template<class T>
  struct is_extended_same<T, T>
  : std::true_type
  {};
}

template<class... Ts>
class tuple
{
  using indexes_ = detail_::tuple_indexes_for<Ts...>;

  using base = detail_::tuple_impl<indexes_, Ts...>;
  base base_;

  template<class... Us>
  using pack_expands_to_this_tuple
    = detail_::is_same_as_uncvref_pack<tuple, Us...>;

  template<class Tuple>
  using is_tuple_like = brigand::bool_<
    std::tuple_size<Tuple>::value == sizeof...(Ts) &&
    // !detail_::is_extended_same<Tuple, uncvref_t<Ts>...>::value &&
    !std::is_same<Tuple, tuple>::value
  >;

  template<
    class Tuple_, class Lbd,
    size_t tsz = std::tuple_size<uncvref_t<Tuple_>>::value,
    class Tuple = std::enable_if_t<
      brigand::size<detail_::tuple_to_list_t<Tuple_>>::value == sizeof...(Ts),
      Tuple_
    >
  >
  using tuple_is_implicitly_xxx = detail_::is_same<
    typename brigand::lazy::transform<
      detail_::tuple_to_list_t<Tuple>,
      brigand::list<Ts...>,
      Lbd
    >::type,
    brigand::filled_list<std::true_type, tsz>
  >;

  template<class Tuple>
  using tuple_is_implicitly_convertible = tuple_is_implicitly_xxx<
    Tuple,
    brigand::bind<detail_::is_convertible, brigand::_1, brigand::_2>
  >;

public:
  // 1 (implicit)
  template<
    bool Dummy = true,
    std::enable_if_t<
      Dummy &&
      detail_::mpl_all<detail_::is_default_constructible<Ts>...>::value,
      bool
    > = true
  >
  tuple()
  //noexcept(detail_::mpl_all<detail_::is_nothrow_default_constructible<Ts>...>::value)
  noexcept(noexcept(base()))
  : base_()
  {}

  // 1 (explicit)
  template<
    bool Dummy = true,
    std::enable_if_t<
      Dummy &&
      !detail_::mpl_all<detail_::is_default_constructible<Ts>...>::value,
      bool
    > = true
  >
  explicit tuple()
  //noexcept(detail_::mpl_all<detail_::is_nothrow_default_constructible<Ts>...>::value)
  noexcept(noexcept(base()))
  : base_()
  {}

  // 9
  tuple(tuple && other) = default;
  // 8
  tuple(tuple const & other) = default;

  // 2 (implicit)
  template<
    bool Dummy = true,
    std::enable_if_t<
      Dummy &&
      detail_::mpl_all<detail_::is_copy_constructible<Ts>...>::value,
      bool
    > = true
  >
  constexpr tuple(Ts const &... args)
  noexcept(noexcept(base(args...)))
  : base_(args...)
  {}

  // 2 (explicit)
  template<
    bool Dummy = true,
    std::enable_if_t<
      Dummy &&
      !detail_::mpl_all<detail_::is_copy_constructible<Ts>...>::value,
      bool
    > = true
  >
  explicit constexpr tuple(Ts const &... args)
  noexcept(noexcept(base(args...)))
  : base_(args...)
  {}

  // 3 (implicit)
  template<
    class... Us,
    std::enable_if_t<
      !pack_expands_to_this_tuple<Us...>::value &&
      detail_::mpl_all<detail_::is_convertible<Us, Ts>...>::value,
      bool
    > = true
  >
  constexpr tuple(Us &&... args)
  noexcept(noexcept(base(std::forward<Us>(args)...)))
  : base_(std::forward<Us>(args)...)
  {}

  // 3 (explicit)
  template<
    class... Us,
    std::enable_if_t<
      !pack_expands_to_this_tuple<Us...>::value &&
      !detail_::mpl_all<detail_::is_convertible<Us, Ts>...>::value,
      bool
    > = true
  >
  explicit constexpr tuple(Us &&... args)
  noexcept(noexcept(base(std::forward<Us>(args)...)))
  : base_(std::forward<Us>(args)...)
  {}

  // 4, 5, 6, 7 (implicit)
  template<
    class Tuple,
    std::enable_if_t<
      is_tuple_like<uncvref_t<Tuple>>::value &&
      tuple_is_implicitly_convertible<Tuple>::value,
      bool
    > = false
  >
  constexpr tuple(Tuple && t)
  noexcept(noexcept(base(indexes_{}, std::forward<Tuple>(t))))
  : base_(indexes_{}, std::forward<Tuple>(t))
  {}

  // 4, 5, 6, 7 (explicit)
  template<
    class Tuple,
    std::enable_if_t<
      is_tuple_like<uncvref_t<Tuple>>::value &&
      !tuple_is_implicitly_convertible<Tuple>::value,
      bool
    > = false
  >
  explicit constexpr tuple(Tuple && t)
  noexcept(noexcept(base(indexes_{}, std::forward<Tuple>(t))))
  : base_(indexes_{}, std::forward<Tuple>(t))
  {}


  // allocator-extended constructors

  // 10 (implicit)
  template<
    class Alloc,
    std::enable_if_t<
      detail_::mpl_all<
        detail_::is_allocator_extended_implicitly_constructible_t<Ts, Alloc>
      ...>::value,
      bool
    > = true
  >
  tuple(allocator_arg_t, Alloc const & a)
  : base_(allocator_arg_t{}, a)
  {}

  // 10 (explicit)
  template<
    class Alloc,
    std::enable_if_t<
      !detail_::mpl_all<
        detail_::is_allocator_extended_implicitly_constructible_t<Ts, Alloc>
      ...>::value &&
      detail_::mpl_all<
        detail_::is_allocator_extended_constructible_t<Ts, Alloc>
      ...>::value,
      bool
    > = true
  >
  explicit tuple(allocator_arg_t, Alloc const & a)
  : base_(allocator_arg_t{}, a)
  {}

  // 11 (implicit)
  template<
    class Alloc,
    std::enable_if_t<
      detail_::mpl_all<
        detail_::is_allocator_extended_implicitly_constructible_t<
          Ts, Alloc, Ts const &
        >
      ...>::value,
      bool
    > = true
  >
  constexpr tuple(allocator_arg_t, Alloc const & a, Ts const &... args)
  : base_(allocator_arg_t{}, a, args...)
  {}

  // 11 (explicit)
  template<
    class Alloc,
    std::enable_if_t<
      !detail_::mpl_all<
        detail_::is_allocator_extended_implicitly_constructible_t<
          Ts, Alloc, Ts const &
        >
      ...>::value &&
      detail_::mpl_all<
        detail_::is_allocator_extended_constructible_t<
          Ts, Alloc, Ts const &
        >
      ...>::value,
      bool
    > = true
  >
  explicit constexpr tuple(allocator_arg_t, Alloc const & a, Ts const &... args)
  : base_(allocator_arg_t{}, a, args...)
  {}

  // 12 (implicit)
  template<
    class Alloc,
    class... Us,
    std::enable_if_t<
      !pack_expands_to_this_tuple<Us...>::value &&
      detail_::mpl_all<
        detail_::is_allocator_extended_implicitly_constructible_t<
          Ts, Alloc, Us
        >
      ...>::value,
      bool
    > = true
  >
  constexpr tuple(allocator_arg_t, Alloc const & a, Us &&... args)
  : base_(allocator_arg_t{}, a, std::forward<Us>(args)...)
  {}

  // 12 (explicit)
  template<
    class Alloc,
    class... Us,
    std::enable_if_t<
      !pack_expands_to_this_tuple<Us...>::value &&
      !detail_::mpl_all<
        detail_::is_allocator_extended_implicitly_constructible_t<
          Ts, Alloc, Us
        >
      ...>::value &&
      detail_::mpl_all<
        detail_::is_allocator_extended_constructible_t<
          Ts, Alloc, Us
        >
      ...>::value,
      bool
    > = true
  >
  explicit constexpr tuple(allocator_arg_t, Alloc const & a, Us &&... args)
  : base_(allocator_arg_t{}, a, std::forward<Us>(args)...)
  {}

  // 13, 14, 15, 16 (implicit)
  template<
    class Alloc,
    class Tuple,
    std::enable_if_t<
      is_tuple_like<uncvref_t<Tuple>>::value &&
      tuple_is_implicitly_xxx<
        Tuple,
        brigand::bind<
          detail_::is_allocator_extended_implicitly_constructible_t,
          brigand::_2, brigand::pin<Alloc const &>, brigand::_1
        >
      >::value,
      bool
    > = false
  >
  constexpr tuple(allocator_arg_t, Alloc const & a, Tuple && t)
  : base_(allocator_arg_t{}, a, indexes_{}, std::forward<Tuple>(t))
  {}

  // 13, 14, 15, 16 (explicit)
  template<
    class Alloc,
    class Tuple,
    std::enable_if_t<
      is_tuple_like<uncvref_t<Tuple>>::value &&
      !tuple_is_implicitly_xxx<
        Tuple,
        brigand::bind<
          detail_::is_allocator_extended_implicitly_constructible_t,
          brigand::_2, brigand::pin<Alloc const &>, brigand::_1
        >
      >::value &&
      tuple_is_implicitly_xxx<
        Tuple,
        brigand::bind<
          detail_::is_allocator_extended_constructible_t,
          brigand::_2, brigand::pin<Alloc const &>, brigand::_1
        >
      >::value,
      bool
    > = false
  >
  explicit constexpr tuple(allocator_arg_t, Alloc const & a, Tuple && t)
  : base_(allocator_arg_t{}, a, indexes_{}, std::forward<Tuple>(t))
  {}

  // 18
  template<
    class Alloc,
    std::enable_if_t<
      tuple_is_implicitly_xxx<
        tuple &&,
        brigand::bind<
          detail_::is_allocator_extended_constructible_t,
          brigand::_2, brigand::pin<Alloc const &>, brigand::_1
        >
      >::value,
      bool
    > = false
  >
  constexpr tuple(allocator_arg_t, Alloc const & a, tuple && other)
  : base_(allocator_arg_t{}, a, indexes_{}, std::move(other))
  {}

  // 17
  template<
    class Alloc,
    std::enable_if_t<
      tuple_is_implicitly_xxx<
        tuple const &,
        brigand::bind<
          detail_::is_allocator_extended_constructible_t,
          brigand::_2, brigand::pin<Alloc const &>, brigand::_1
        >
      >::value,
      bool
    > = false
  >
  constexpr tuple(allocator_arg_t, Alloc const & a, tuple const & other)
  : base_(allocator_arg_t{}, a, indexes_{}, other)
  {}


  tuple & operator=(tuple const & t)
  noexcept(noexcept(
    std::declval<base&>()
    .assign(indexes_{}, t)
  ))
  {
    base_.assign(indexes_{}, t);
    return *this;
  }

  tuple & operator=(tuple && t)
  noexcept(noexcept(
    std::declval<base&>()
    .assign(indexes_{}, std::move(t))
  ))
  {
    base_.assign(indexes_{}, std::move(t));
    return *this;
  }

  template<class Tuple>
  tuple & operator=(Tuple && t)
  noexcept(noexcept(
    std::declval<base&>()
    .assign(indexes_{}, std::forward<Tuple>(t))
  ))
  {
    base_.assign(indexes_{}, std::forward<Tuple>(t));
    return *this;
  }


  void swap(tuple & t)
  noexcept(detail_::mpl_all<is_nothrow_swappable<Ts>...>::value)
  { base_.swap(t.base_); }


// get
//@{

  template<size_t i, class... Us>
  friend constexpr
  typename std::tuple_element<i, tuple<Us...>>::type &
  get(tuple<Us...>       & t) noexcept;

  template<size_t i, class... Us>
  friend constexpr
  typename std::tuple_element<i, tuple<Us...>>::type const &
  get(tuple<Us...> const & t) noexcept;

  template<size_t i, class... Us>
  friend constexpr
  typename std::tuple_element<i, tuple<Us...>>::type &&
  get(tuple<Us...>       && t) noexcept;

  template<size_t i, class... Us>
  friend constexpr
  typename std::tuple_element<i, tuple<Us...>>::type const &&
  get(tuple<Us...> const && t) noexcept;


  template<class T, class... Us>
  friend constexpr
  T &
  get(tuple<Us...>       & t) noexcept;

  template<class T, class... Us>
  friend constexpr
  T const &
  get(tuple<Us...> const & t) noexcept;

  template<class T, class... Us>
  friend constexpr
  T &&
  get(tuple<Us...>       && t) noexcept;

  template<class T, class... Us>
  friend constexpr
  T const &&
  get(tuple<Us...> const && t) noexcept;

//@}
};

template<>
struct tuple<>
{
  tuple() = default;
  tuple(tuple &&) = default;
  tuple(tuple const &) = default;

  template<
    class Tuple,
    std::enable_if_t<
      0u == std::tuple_size<Tuple>::value,
      bool
    > = false
  >
  constexpr tuple(Tuple const &)
  noexcept
  {}

  template<class Alloc>
  tuple(allocator_arg_t, Alloc const &)
  noexcept
  {}

  template<class Alloc>
  tuple(allocator_arg_t, Alloc const &, tuple const &)
  noexcept
  {}

  template<
    class Alloc,
    class Tuple,
    std::enable_if_t<
      0u == std::tuple_size<Tuple>::value,
      bool
    > = false
  >
  tuple(allocator_arg_t, Alloc const &, Tuple const &)
  noexcept
  {}

  tuple & operator=(tuple const &) = default;

  template<
    class Tuple,
    std::enable_if_t<
      0u == std::tuple_size<Tuple>::value,
      bool
    > = false
  >
  tuple & operator=(Tuple const &)
  noexcept
  { return *this; }

  void swap(tuple&) noexcept {}
};


// get
//@{

template<size_t i, class... Ts>
constexpr typename std::tuple_element<i, tuple<Ts...>>::type &
get(tuple<Ts...>       & t) noexcept
{
  using type = typename std::tuple_element<i, tuple<Ts...>>::type;
  return static_cast<detail_::tuple_leaf<i, type> &>(t.base_).get();
}

template<size_t i, class... Ts>
constexpr typename std::tuple_element<i, tuple<Ts...>>::type const &
get(tuple<Ts...> const & t) noexcept
{
  using type = typename std::tuple_element<i, tuple<Ts...>>::type;
  return static_cast<detail_::tuple_leaf<i, type> const &>(t.base_).get();
}

template<size_t i, class... Ts>
constexpr typename std::tuple_element<i, tuple<Ts...>>::type &&
get(tuple<Ts...>       && t) noexcept
{
  using type = typename std::tuple_element<i, tuple<Ts...>>::type;
  return static_cast<type &&>(
    static_cast<detail_::tuple_leaf<i, type> &&>(t.base_).get()
  );
}

template<size_t i, class... Ts>
constexpr typename std::tuple_element<i, tuple<Ts...>>::type const &&
get(tuple<Ts...> const && t) noexcept
{
  using type = typename std::tuple_element<i, tuple<Ts...>>::type;
  return static_cast<type const &&>(
    static_cast<detail_::tuple_leaf<i, type> const &&>(t.base_).get()
  );
}

template<class T, class... Ts>
constexpr T &
get(tuple<Ts...>       & t) noexcept
{
  constexpr size_t i = tuple_index_of<T, tuple<Ts...>>::value;
  return static_cast<detail_::tuple_leaf<i, T> &>(t.base_).get();
}

template<class T, class... Ts>
constexpr T const &
get(tuple<Ts...> const & t) noexcept
{
  constexpr size_t i = tuple_index_of<T, tuple<Ts...>>::value;
  return static_cast<detail_::tuple_leaf<i, T> const &>(t.base_).get();
}

template<class T, class... Ts>
constexpr T &&
get(tuple<Ts...>       && t) noexcept
{
  constexpr size_t i = tuple_index_of<T, tuple<Ts...>>::value;
  return static_cast<T &&>(
    static_cast<detail_::tuple_leaf<i, T> &&>(t.base_).get()
  );
}

template<class T, class... Ts>
constexpr T const &&
get(tuple<Ts...> const && t) noexcept
{
  constexpr size_t i = tuple_index_of<T, tuple<Ts...>>::value;
  return static_cast<T const &&>(
    static_cast<detail_::tuple_leaf<i, T> const &&>(t.base_).get()
  );
}

//@}

template<class T, class... Ts>
struct tuple_index_of<T, tuple<Ts...>>
: decltype(detail_::tuple_index_of_impl<T>(std::declval<
  detail_::tuple_impl<
    detail_::tuple_indexes_for<Ts...>,
    Ts...
  > const &
>()))
{};

template<class T, class... Ts>
struct tuple_indexes_of<T, tuple<Ts...>>
: detail_::tuple_indexes_of_impl<
  T,
  detail_::tuple_impl<
    detail_::tuple_indexes_for<Ts...>,
    Ts...
  >
>
{};


template<class... Ts>
void swap(tuple<Ts...> & t1, tuple<Ts...> & t2)
noexcept(noexcept(t1.swap(t2)))
{ t1.swap(t2); }


// TODO type_traits/unwrap_refwrapper.hpp
template<class T>
struct unwrap_refwrapper
{ using type = T; };

template<class T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{ using type = T&; };

template<class T>
using unwrap_refwrapper_t = typename unwrap_refwrapper<T>::type;

// TODO type_traits/decay_unwrap_refwrapper_t.hpp
template<class T>
using decay_unwrap_refwrapper_t = unwrap_refwrapper_t<std::decay_t<T>>;


template<class... Ts>
constexpr
tuple<decay_unwrap_refwrapper_t<Ts>...>
make_tuple(Ts && ... args) noexcept(noexcept(
  tuple<decay_unwrap_refwrapper_t<Ts>...>{std::forward<Ts>(args)...}
))
{ return tuple<decay_unwrap_refwrapper_t<Ts>...>{std::forward<Ts>(args)...}; }


template<class... Ts>
constexpr
tuple<Ts & ...>
tie(Ts & ... args) noexcept
{ return tuple<Ts & ...>{args...}; }


template<class... Ts>
constexpr
tuple<Ts && ...>
forward_as_tuple(Ts && ... args) noexcept
{ return tuple<Ts && ...>{std::forward<Ts>(args)...}; }


namespace detail_
{
  template<class T, class n>
  using filled_list_t = brigand::filled_list<T, n::value>;

  template<class... Tuples>
  using tuple_cat_return_type = brigand::wrap<
    brigand::append<detail_::tuple_to_list_t<Tuples>...>,
    tuple
  >;

  template<class T, class... ituple, class... ielem, class Tuple>
  constexpr T
  tuple_cat_impl(brigand::list<ituple...>, brigand::list<ielem...>, Tuple && t)
  FALCON_RETURN_NOEXCEPT(
    T(get<ielem::value>(get<ituple::value>(t))...)
  )
}

template<class... Tuples>
constexpr
detail_::tuple_cat_return_type<Tuples...>
tuple_cat(Tuples && ... tuples)
FALCON_RETURN_NOEXCEPT(
  detail_::tuple_cat_impl<
    detail_::tuple_cat_return_type<Tuples...>
  >(
    /*index_by_tuple*/ brigand::join<brigand::transform<
      /*indexes*/ detail_::range_for<Tuples...>,
      /*size_list*/ brigand::list<
        typename std::tuple_size<uncvref_t<Tuples>>::type...
      >,
      brigand::bind<detail_::filled_list_t, brigand::_1, brigand::_2>
    >>{},
    /*index_by_element*/ brigand::append<
      detail_::range_for_tuple<Tuples>...
    >{},
    falcon::forward_as_tuple(std::forward<Tuples>(tuples)...)
  )
)

// TODO index_sequence_from_tuple
// TODO apply
// TODO apply_on_each
// TODO tuple_transform ?
// TODO tuple_as<T> = apply(maker<T>{}, ...)
// TODO == != < > <= >= -> integral optimization
// TODO tuple_compare
// TODO tuple_slice
// TODO operator[i]
// TODO operator[iseq]
// TODO arity
// TODO uninit ctor
// TODO uninit element
// TODO optimized empty tuple with default ctor, dtor

}

namespace rapidtuple {

namespace detail_ {
  template<class Ints, class... Ts>
  struct tuple_impl;

  template<class T>
  struct is_unique {};

  template<class... Ts>
  struct tuple_set_build : is_unique<Ts>... // error: you have defined a tuple_set with a type present more than once
  { using type = tuple_impl<std::index_sequence_for<Ts...>, Ts...>; };

  template<class T>
  struct remove_reference_wrapper
  { using type = T; };

  template<class T>
  struct remove_reference_wrapper<std::reference_wrapper<T>>
  { using type = T&; };

  template<class T>
  using remove_reference_wrapper_t = typename remove_reference_wrapper<T>::type;
}


template<class... Ts>
using tuple = detail_::tuple_impl<std::index_sequence_for<Ts...>, Ts...>;

template<class... Ts>
using tuple_set = typename detail_::tuple_set_build<Ts...>::type;

FALCON_EMPTY_CLASS(default_element_t);
constexpr default_element_t default_element;


using std::ignore;


namespace detail_ {
  template<class...>
  class pack {};
}

using ignore_t = std::remove_const_t<decltype(std::ignore)>;


template<class Tuple>
using index_sequence_from_tuple = std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>;


namespace detail_{
  template<class T> struct ref { T & x; };
  template<class T> struct cref { T const & x; };

  template<class T>
  struct empty_not_final
  { constexpr static bool value = (!std::is_final<T>::value && std::is_class<T>::value); };

  // disable inaccessible warning
  template<class... Ts>
  struct empty_not_final<tuple_impl<Ts...>>
  { constexpr static bool value = false; };

  template<std::size_t I, class T, bool = empty_not_final<T>::value>
  struct tuple_leaf : private T
  {
    constexpr tuple_leaf() : T{} {}
    constexpr tuple_leaf(tuple_leaf &&) = default;
    constexpr tuple_leaf(tuple_leaf const &) = default;

    constexpr tuple_leaf(ignore_t) {}

    constexpr tuple_leaf(default_element_t) : T{} {}

    constexpr tuple_leaf(T const & arg)
    : T(arg)
    {}

    template<class U>
    constexpr tuple_leaf(U && arg)
    : T{std::forward<U>(arg)}
    {}

    template<class Alloc, class... Ts, class = typename std::enable_if<!std::uses_allocator<T, Alloc>::value>::type>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const &, Ts && ... args)
    : tuple_leaf(std::forward<Ts>(args)...)
    {}

    template<class Alloc>
    using disable_is_not_uses_allocator = typename std::enable_if<std::uses_allocator<T, Alloc>::value>::type;

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const & a)
    : T{a}
    {}

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const & a, ignore_t)
    : T{a}
    {}

    template<class Alloc, class U, class = disable_is_not_uses_allocator<Alloc>>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const & a, U && arg)
    : T{a, std::forward<U>(arg)}
    {}

    tuple_leaf & operator=(tuple_leaf && other)
    noexcept(std::is_nothrow_move_assignable<T>::value) {
      get() = std::forward<T>(other.get());
      return *this;
    }

    tuple_leaf & operator=(tuple_leaf const & other) {
      get() = other.get();
      return *this;
    }

    T & get() noexcept { return static_cast<T&>(*this); }
    T const & get() const { return static_cast<T const&>(*this); }

    operator ref<T> () { return {get()}; }
    operator cref<T> () const { return {get()}; }
  };

  template<std::size_t I, class T>
  struct tuple_leaf<I, T, false>
  {
    constexpr tuple_leaf() : x{} {}
    constexpr tuple_leaf(ignore_t) {}
    constexpr tuple_leaf(tuple_leaf &&) = default;
    constexpr tuple_leaf(tuple_leaf const &) = default;

    template<class U>
    constexpr tuple_leaf(U && arg)
    : x{std::forward<U>(arg)}
    {}

    template<class Alloc, class... Ts, class = typename std::enable_if<!std::uses_allocator<T, Alloc>::value>::type>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const &, Ts && ... args)
    : tuple_leaf(std::forward<Ts>(args)...)
    {}

    template<class Alloc>
    using disable_is_not_uses_allocator = typename std::enable_if<std::uses_allocator<T, Alloc>::value>::type;

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const & a)
    : x{a}
    {}

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const & a, ignore_t)
    : x{a}
    {}

    template<class Alloc, class U, class = disable_is_not_uses_allocator<Alloc>>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const & a, U && arg)
    : x{a, std::forward<U>(arg)}
    {}

    tuple_leaf & operator=(tuple_leaf && other)
    noexcept(std::is_nothrow_move_assignable<T>::value) {
      x = std::forward<T>(other.get());
      return *this;
    }

    tuple_leaf & operator=(tuple_leaf const & other) {
      x = other.get();
      return *this;
    }

    T & get() noexcept { return x; }
    T const & get() const { return x; }

    operator ref<T> () { return {x}; }
    operator cref<T> () const { return {x}; }

  private:
    T x;
  };

  // (Clang) disable ambiguisity
  template<std::size_t I>
  struct tuple_leaf<I, ignore_t, true>
  : ignore_t
  {
    template<class U>
    constexpr tuple_leaf(U const &)
    {}

    template<class Alloc, class... Ts>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const &, Ts const & ...)
    {}

    tuple_leaf & operator=(tuple_leaf &&) noexcept {}
    tuple_leaf & operator=(tuple_leaf const &) {}

    ignore_t & get() noexcept { return static_cast<ignore_t&>(*this); }
    ignore_t const & get() const { return static_cast<ignore_t const&>(*this); }

    operator ref<ignore_t> () { return {get()}; }
    operator cref<ignore_t> () const { return {get()}; }
  };

  template<std::size_t I>
  struct tuple_leaf<I, ignore_t const, false>
  : ignore_t
  {
    template<class U>
    constexpr tuple_leaf(U const &)
    {}

    template<class Alloc, class... Ts>
    constexpr tuple_leaf(std::allocator_arg_t, Alloc const &, Ts const & ...)
    {}

    tuple_leaf & operator=(tuple_leaf &&) noexcept {}
    tuple_leaf & operator=(tuple_leaf const &) {}
    tuple_leaf & operator=(ignore_t) = delete;

    ignore_t const & get() noexcept { return static_cast<ignore_t const&>(*this); }
    ignore_t const & get() const { return static_cast<ignore_t const&>(*this); }

    operator ref<ignore_t> () { return {get()}; }
    operator cref<ignore_t const> () const { return {get()}; }
  };


  using std::swap;

  template<class...> struct types_list;

  template<class T, class U>
  struct enable_if_convertible : std::enable_if<
      std::is_convertible<T, U>::value
  > {};

  template<class U> struct enable_if_convertible<ignore_t, U> { using type = void; };
  template<class U> struct enable_if_convertible<ignore_t&, U> { using type = void; };
  template<class U> struct enable_if_convertible<ignore_t&&, U> { using type = void; };
  template<class U> struct enable_if_convertible<ignore_t const &, U> { using type = void; };

  template<std::size_t... Ints, class... Ts>
  class tuple_impl<std::index_sequence<Ints...>, Ts...>
  : public tuple_leaf<Ints, Ts>...
  {
    using index_sequence_ = std::index_sequence<Ints...>;

  public:
    explicit tuple_impl() = default;
    tuple_impl(tuple_impl && other) = default;
    tuple_impl(tuple_impl const & other) = default;

    explicit constexpr tuple_impl(Ts const &... args)
    : tuple_leaf<Ints, Ts>(args)...
    {}

    template<class... Us, class = types_list<typename enable_if_convertible<Us, Ts>::type...>>
    explicit constexpr tuple_impl(Us &&... args)
    : tuple_leaf<Ints, Ts>(std::forward<Us>(args))...
    {}


    template<class... Us>
    constexpr tuple_impl(tuple_impl<index_sequence_, Us...> const & other)
    : tuple_leaf<Ints, Ts>(
        static_cast<tuple_leaf<Ints, Us> const &>(other).get()
    )...
    {}

    template<class... Us>
    constexpr tuple_impl(tuple_impl<index_sequence_, Us...> & other)
    : tuple_leaf<Ints, Ts>(
        static_cast<tuple_leaf<Ints, Us> const &>(other).get()
    )...
    {}

    template<class... Us>
    constexpr tuple_impl(tuple_impl<index_sequence_, Us...> && other)
    : tuple_leaf<Ints, Ts>(
        static_cast<Us&&>(static_cast<tuple_leaf<Ints, Us> &>(other).get())
    )...
    {}


    template<class U1, class U2>
    constexpr tuple_impl(std::pair<U1, U2> const & p)
    : tuple_leaf<Ints, Ts>(std::get<Ints>(p))...
    {}

    template<class U1, class U2>
    constexpr tuple_impl(std::pair<U1, U2> && p)
    : tuple_leaf<Ints, Ts>(std::get<Ints>(std::move(p)))...
    {}

    template<class U1, class U2>
    constexpr tuple_impl(std::pair<U1, U2> & p)
    : tuple_leaf<Ints, Ts>(std::get<Ints>(static_cast<std::pair<U1, U2> const &>(p)))...
    {}


    template<class Alloc>
    tuple_impl(std::allocator_arg_t, Alloc const & a)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a)...
    {}

    template<class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, Us &&... args)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a, std::forward<Us>(args))...
    {}

    template<class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, tuple_impl<index_sequence_, Us...> const & other)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a,
      static_cast<tuple_leaf<Ints, Us> const &>(other).get()
    )...
    {}

    template<class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, tuple_impl<index_sequence_, Us...> && other)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a, std::move(
      static_cast<tuple_leaf<Ints, Us>&&>(other).get()
    ))...
    {}

    template<class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, tuple_impl<index_sequence_, Us...> & other)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a, std::move(
      static_cast<tuple_leaf<Ints, Us>&>(other).get()
    ))...
    {}


    template<class Alloc, class U1, class U2>
    tuple_impl(std::allocator_arg_t, Alloc const & a, std::pair<U1, U2> const & p)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a, std::get<Ints>(p))...
    {}

    template<class Alloc, class U1, class U2>
    tuple_impl(std::allocator_arg_t, Alloc const & a, std::pair<U1, U2> && p)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a, std::get<Ints>(std::move(p)))...
    {}

    template<class Alloc, class U1, class U2>
    tuple_impl(std::allocator_arg_t, Alloc const & a, std::pair<U1, U2> & p)
    : tuple_leaf<Ints, Ts>(std::allocator_arg_t{}, a, std::get<Ints>(static_cast<std::pair<U1, U2> const &>(p)))...
    {}


    tuple_impl& operator=(tuple_impl const & other) = default;
    tuple_impl& operator=(tuple_impl && other) = default;

    template<class... Us>
    tuple_impl& operator=(tuple_impl<index_sequence_, Us...> const & other) {
      FALCON_UNPACK(
        static_cast<tuple_leaf<Ints, Ts>&>(*this).get()
        = static_cast<tuple_leaf<Ints, Us> const &>(other).get()
      );
      return *this;
    }

    template<class... Us>
    tuple_impl& operator=(tuple_impl<index_sequence_, Us...> && other) {
      FALCON_UNPACK(
        static_cast<tuple_leaf<Ints, Ts>&>(*this).get()
        = std::move(static_cast<tuple_leaf<Ints, Us>&>(other).get())
      );
      return *this;
    }


    template<class U1, class U2>
    tuple_impl& operator=(std::pair<U1, U2> const & p) {
      FALCON_UNPACK(
        static_cast<tuple_leaf<Ints, Ts>&>(*this).get()
        = std::get<Ints>(p)
      );
      return *this;
    }

    template<class U1, class U2>
    tuple_impl& operator=(std::pair<U1, U2> && p) noexcept {
      FALCON_UNPACK(
        static_cast<tuple_leaf<Ints, Ts>&>(*this).get()
        = std::get<Ints>(std::move(p))
      );
      return *this;
    }


    // TODO std::swap -> fn::swap
    void swap(tuple_impl & other)
    noexcept(noexcept(FALCON_UNPACK(std::swap(
      std::declval<Ts&>(), std::declval<Ts&>()
    )))) {
      FALCON_UNPACK(std::swap(
        static_cast<tuple_leaf<Ints, Ts>&>(*this).get(),
        static_cast<tuple_leaf<Ints, Ts>&>(other).get()
      ));
    }
  };

  template<>
  struct tuple_impl<std::index_sequence<>>
  {
    tuple_impl() = default;
    tuple_impl(tuple_impl &&) = default;
    tuple_impl(tuple_impl const &) = default;

    template<class Alloc>
    tuple_impl(std::allocator_arg_t, Alloc const &)
    {}

    void swap(tuple_impl &) noexcept
    {}
  };

  // GCC inexplicable (tuple<int>({1}) -> tuple_impl<std::index_sequence<0>>) ???
  template<>
  struct tuple_impl<std::index_sequence<0>>;

  template<std::size_t... Ints, class... Ts>
  void swap(
    tuple_impl<std::index_sequence<Ints...>, Ts...> & t1,
    tuple_impl<std::index_sequence<Ints...>, Ts...> & t2
  ) noexcept(noexcept(t1.swap(t2))) {
    t1.swap(t2);
  }


  template<std::size_t I, class T>
  T get_i(tuple_leaf<I, T> const &);

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> &
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> & t) {
    return static_cast<tuple_leaf<I, decltype(get_i<I>(t))>&>(t).get();
  }

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> &&
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> && t) {
    using T = decltype(get_i<I>(t));
    return static_cast<T&&>(
      static_cast<tuple_leaf<I, T>&&>(t).get()
    );
  }

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> const &
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> const & t) {
    return static_cast<tuple_leaf<I, decltype(get_i<I>(t))> const &>(t).get();
  }


  template<class T, class Ints, class... Ts>
  constexpr T & get(tuple_impl<Ints, Ts...> & t) {
    return ref<T>(t).x;
  }

  template<class T, class Ints, class... Ts>
  constexpr T&& get(tuple_impl<Ints, Ts...> && t) {
    return static_cast<T&&>(ref<T>(t).x);
  }

  template<class T, class Ints, class... Ts>
  constexpr T const & get(tuple_impl<Ints, Ts...> const & t) {
    return cref<T>(t).x;
  }


  template<class Fn, class Tuple, class TInt, TInt... Ints>
  Fn each_from_tuple(Fn fn, Tuple && t, std::integer_sequence<TInt, Ints...>)
  {
    FALCON_UNPACK(fn(get<Ints>(std::forward<Tuple>(t))));
    return fn;
  }

  template<class Tuple, class Fn>
  Fn each_from_tuple(Fn fn, Tuple && t)
  {
    each_from_tuple<Fn&>(fn, std::forward<Tuple>(t), index_sequence_from_tuple<Tuple>{});
    return fn;
  }


  template<class Fn, class Tuple, class TInt, TInt... Ints>
  decltype(auto) apply_from_tuple(Fn && fn, Tuple && t, std::integer_sequence<TInt, Ints...>)
  {
    return std::forward<Fn>(fn)(get<Ints>(t)...);
  }

  template<class Fn, class Tuple, std::size_t... Ints, class... Ts>
  decltype(auto) apply_from_tuple(Fn && fn, Tuple && t)
  {
    return apply_from_tuple(std::forward<Fn>(fn), std::forward<Tuple>(t), index_sequence_from_tuple<Tuple>{});
  }


  template<class T, std::size_t I>
  constexpr std::integral_constant<std::size_t, I> tuple_index_of_impl(tuple_leaf<I, T> const &);

  template<class T, class Tuple>
  struct tuple_index_of
  : decltype(tuple_index_of_impl<T>(std::declval<Tuple&>()))
  {};


  template<class T>
  struct to_tuple_element
  {
    using type = remove_reference_wrapper_t<std::decay_t<T>>;
  };

  template<>
  struct to_tuple_element<void>
  {
    using type = ignore_t;
  };

  template<class Fn>
  struct silent_fn
  {
    Fn & fn_;

    silent_fn(Fn & fn) : fn_(fn) {}

    template<class Arg>
    ignore_t operator()(Arg&& arg) const {
      fn_(std::forward<Arg>(arg));
      return {};
    }
  };

  template<class T, class Fn>
  using wrap_fn = std::conditional_t<std::is_same<T, void>::value, silent_fn<Fn>, Fn&>;


  template<class Fn, class Tuple, class TInt, TInt... Ints>
  auto transform_from_tuple(Fn fn, Tuple && t, std::integer_sequence<TInt, Ints...>)
  {
     return tuple_impl<
       std::integer_sequence<size_t, Ints...>,
       typename to_tuple_element<decltype(fn(get<Ints>(std::forward<Tuple>(t))))>::type...
     >(
       wrap_fn<decltype(fn(get<Ints>(std::forward<Tuple>(t)))), Fn>(fn)(get<Ints>(std::forward<Tuple>(t)))...
     );
  }

  template<class Fn, class Tuple>
  auto transform_from_tuple(Fn fn, Tuple && t)
  {
     return transform_from_tuple<Fn&>(fn, std::forward<Tuple>(t), index_sequence_from_tuple<Tuple>{});
  }


  template<class Tuple1, class Tuple2>
  constexpr bool eq_impl(pack<>, pack<>, Tuple1 const &, Tuple2 const &) {
    return true;
  }

  template<class T, class... Ts, class U, class... Us, class Tuple1, class Tuple2>
  constexpr bool eq_impl(pack<T,Ts...>, pack<U,Us...>, Tuple1 const & lhs, Tuple2 const & rhs) {
    return static_cast<T const&>(lhs).get() == static_cast<U const&>(rhs).get()
        && eq_impl(pack<Ts...>(), pack<Us...>(), lhs, rhs);
  }

  template<std::size_t... Ints, class... Ts, class... Us>
  constexpr bool operator==(
    tuple_impl<std::index_sequence<Ints...>, Ts...> const & lhs,
    tuple_impl<std::index_sequence<Ints...>, Us...> const & rhs
  ) {
    return eq_impl(
      pack<tuple_leaf<Ints, Ts>...>(),
      pack<tuple_leaf<Ints, Us>...>(),
      lhs, rhs
    );
  }

  template<class Tuple1, class Tuple2>
  constexpr bool less_impl(pack<>, pack<>, Tuple1 const &, Tuple2 const &) {
    return false;
  }

  template<class T, class... Ts, class U, class... Us, class Tuple1, class Tuple2>
  constexpr bool less_impl(pack<T,Ts...>, pack<U,Us...>, Tuple1 const & lhs, Tuple2 const & rhs) {
    return static_cast<T const&>(lhs).get() < static_cast<U const&>(rhs).get()
        || (!(static_cast<U const&>(rhs).get() < static_cast<T const&>(lhs).get())
            && less_impl(pack<Ts...>(), pack<Us...>(), lhs, rhs));
  }

  template<std::size_t... Ints, class... Ts, class... Us>
  constexpr bool operator<(
    tuple_impl<std::index_sequence<Ints...>, Ts...> const & lhs,
    tuple_impl<std::index_sequence<Ints...>, Us...> const & rhs
  ) {
    return less_impl(
      pack<tuple_leaf<Ints, Ts>...>(),
      pack<tuple_leaf<Ints, Us>...>(),
      lhs, rhs
    );
  }

  template<std::size_t... Ints, class... Ts, class... Us>
  constexpr bool
  operator!=(
    tuple_impl<std::index_sequence<Ints...>, Ts...> const & lhs,
    tuple_impl<std::index_sequence<Ints...>, Us...> const & rhs
  ) {
    return !(lhs == rhs);
  }

  template<std::size_t... Ints, class... Ts, class... Us>
  constexpr bool
  operator<=(
    tuple_impl<std::index_sequence<Ints...>, Ts...> const & lhs,
    tuple_impl<std::index_sequence<Ints...>, Us...> const & rhs
  ) {
    return !(rhs < lhs);
  }

  template<std::size_t... Ints, class... Ts, class... Us>
  constexpr bool
  operator>(
    tuple_impl<std::index_sequence<Ints...>, Ts...> const & lhs,
    tuple_impl<std::index_sequence<Ints...>, Us...> const & rhs
  ) {
    return rhs < lhs;
  }

  template<std::size_t... Ints, class... Ts, class... Us>
  constexpr bool
  operator>=(
    tuple_impl<std::index_sequence<Ints...>, Ts...> const & lhs,
    tuple_impl<std::index_sequence<Ints...>, Us...> const & rhs
  ) {
    return !(lhs < rhs);
  }
}

using detail_::get;
using detail_::each_from_tuple;
using detail_::apply_from_tuple;
using detail_::transform_from_tuple;

using detail_::tuple_index_of;
using std::tuple_size;
using std::tuple_element;
using std::tuple_element_t;

}

namespace std {

template<std::size_t... Ints, class... Ts>
struct tuple_size< ::rapidtuple::detail_::tuple_impl<std::index_sequence<Ints...>, Ts...>>
: std::integral_constant<std::size_t, sizeof...(Ts)>
{};


template<std::size_t I, std::size_t... Ints, class... Ts>
struct tuple_element<
  I, ::rapidtuple::detail_::tuple_impl<std::index_sequence<Ints...>, Ts...>
>{ using type = decltype(::rapidtuple::detail_::get_i<I>(std::declval< ::rapidtuple::detail_::tuple_impl<std::index_sequence<Ints...>, Ts...>&>())); };


template<class... Ts, class Alloc>
struct uses_allocator< ::rapidtuple::tuple<Ts...>, Alloc>
: std::true_type
{};


template<class... Ts>
struct tuple_size< ::falcon::tuple<Ts...>>
: std::integral_constant<std::size_t, sizeof...(Ts)>
{};


template<std::size_t I, class... Ts>
struct tuple_element<I, ::falcon::tuple<Ts...>>
{ using type = brigand::at_c<brigand::list<Ts...>, I>; };


template<class... Ts, class Alloc>
struct uses_allocator< ::falcon::tuple<Ts...>, Alloc>
: std::true_type
{};

using ::falcon::get;

}

#endif
