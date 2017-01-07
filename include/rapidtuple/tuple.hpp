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


template<class... Ts>
constexpr auto make_tuple(Ts &&... args) {
  return tuple<detail_::remove_reference_wrapper_t<std::decay_t<Ts>>...>{std::forward<Ts>(args)...};
}


template<class... Ts>
constexpr tuple<Ts&...> tie(Ts&... args) noexcept {
  return tuple<Ts&...>{args...};
}


template<class... Ts>
constexpr tuple<Ts&&...> forward_as_tuple(Ts&&... args) noexcept {
  return tuple<Ts&&...>{std::forward<Ts>(args)...};
}


using std::ignore;


namespace detail_ {
  template<class PackRet, class IntsPack, class TuplesPack>
  struct tuple_concater;

  template<class...>
  class pack {};
}

template<class... Tuples>
constexpr auto tuple_cat(Tuples&&... tuples) {
  return detail_::tuple_concater<
      detail_::pack<>,
      detail_::pack<
        std::make_index_sequence<std::tuple_size<std::decay_t<Tuples>>::value>...
      >,
      detail_::pack<Tuples...>
  >::impl(std::forward<Tuples>(tuples)...);
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
  struct head : private T
  {
    constexpr head() : T{} {}
    constexpr head(head &&) = default;
    constexpr head(head const &) = default;

    constexpr head(ignore_t) {}

    constexpr head(default_element_t) : T{} {}

    constexpr head(T const & arg)
    : T(arg)
    {}

    template<class U>
    constexpr head(U && arg)
    : T{std::forward<U>(arg)}
    {}

    template<class Alloc, class... Ts, class = typename std::enable_if<!std::uses_allocator<T, Alloc>::value>::type>
    constexpr head(std::allocator_arg_t, Alloc const &, Ts && ... args)
    : head(std::forward<Ts>(args)...)
    {}

    template<class Alloc>
    using disable_is_not_uses_allocator = typename std::enable_if<std::uses_allocator<T, Alloc>::value>::type;

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr head(std::allocator_arg_t, Alloc const & a)
    : T{a}
    {}

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr head(std::allocator_arg_t, Alloc const & a, ignore_t)
    : T{a}
    {}

    template<class Alloc, class U, class = disable_is_not_uses_allocator<Alloc>>
    constexpr head(std::allocator_arg_t, Alloc const & a, U && arg)
    : T{a, std::forward<U>(arg)}
    {}

    head & operator=(head && other)
    noexcept(std::is_nothrow_move_assignable<T>::value) {
      get() = std::forward<T>(other.get());
      return *this;
    }

    head & operator=(head const & other) {
      get() = other.get();
      return *this;
    }

    T & get() noexcept { return static_cast<T&>(*this); }
    T const & get() const { return static_cast<T const&>(*this); }

    operator ref<T> () { return {get()}; }
    operator cref<T> () const { return {get()}; }
  };

  template<std::size_t I, class T>
  struct head<I, T, false>
  {
    constexpr head() : x{} {}
    constexpr head(ignore_t) {}
    constexpr head(head &&) = default;
    constexpr head(head const &) = default;

    template<class U>
    constexpr head(U && arg)
    : x{std::forward<U>(arg)}
    {}

    template<class Alloc, class... Ts, class = typename std::enable_if<!std::uses_allocator<T, Alloc>::value>::type>
    constexpr head(std::allocator_arg_t, Alloc const &, Ts && ... args)
    : head(std::forward<Ts>(args)...)
    {}

    template<class Alloc>
    using disable_is_not_uses_allocator = typename std::enable_if<std::uses_allocator<T, Alloc>::value>::type;

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr head(std::allocator_arg_t, Alloc const & a)
    : x{a}
    {}

    template<class Alloc, class = disable_is_not_uses_allocator<Alloc>>
    constexpr head(std::allocator_arg_t, Alloc const & a, ignore_t)
    : x{a}
    {}

    template<class Alloc, class U, class = disable_is_not_uses_allocator<Alloc>>
    constexpr head(std::allocator_arg_t, Alloc const & a, U && arg)
    : x{a, std::forward<U>(arg)}
    {}

    head & operator=(head && other)
    noexcept(std::is_nothrow_move_assignable<T>::value) {
      x = std::forward<T>(other.get());
      return *this;
    }

    head & operator=(head const & other) {
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
  struct head<I, ignore_t, true>
  : ignore_t
  {
    template<class U>
    constexpr head(U const &)
    {}

    template<class Alloc, class... Ts>
    constexpr head(std::allocator_arg_t, Alloc const &, Ts const & ...)
    {}

    head & operator=(head &&) noexcept {}
    head & operator=(head const &) {}

    ignore_t & get() noexcept { return static_cast<ignore_t&>(*this); }
    ignore_t const & get() const { return static_cast<ignore_t const&>(*this); }

    operator ref<ignore_t> () { return {get()}; }
    operator cref<ignore_t> () const { return {get()}; }
  };

  template<std::size_t I>
  struct head<I, ignore_t const, false>
  : ignore_t
  {
    template<class U>
    constexpr head(U const &)
    {}

    template<class Alloc, class... Ts>
    constexpr head(std::allocator_arg_t, Alloc const &, Ts const & ...)
    {}

    head & operator=(head &&) noexcept {}
    head & operator=(head const &) {}
    head & operator=(ignore_t) = delete;

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
  : public head<Ints, Ts>...
  {
    using index_sequence_ = std::index_sequence<Ints...>;

  public:
    explicit tuple_impl() = default;
    tuple_impl(tuple_impl && other) = default;
    tuple_impl(tuple_impl const & other) = default;

    explicit constexpr tuple_impl(Ts const &... args)
    : head<Ints, Ts>(args)...
    {}

    template<class... Us, class = types_list<typename enable_if_convertible<Us, Ts>::type...>>
    explicit constexpr tuple_impl(Us &&... args)
    : head<Ints, Ts>(std::forward<Us>(args))...
    {}


    template<class... Us>
    constexpr tuple_impl(tuple_impl<index_sequence_, Us...> const & other)
    : head<Ints, Ts>(
        static_cast<head<Ints, Us> const &>(other).get()
    )...
    {}

    template<class... Us>
    constexpr tuple_impl(tuple_impl<index_sequence_, Us...> & other)
    : head<Ints, Ts>(
        static_cast<head<Ints, Us> const &>(other).get()
    )...
    {}

    template<class... Us>
    constexpr tuple_impl(tuple_impl<index_sequence_, Us...> && other)
    : head<Ints, Ts>(
        static_cast<Us&&>(static_cast<head<Ints, Us> &>(other).get())
    )...
    {}


    template<class U1, class U2>
    constexpr tuple_impl(std::pair<U1, U2> const & p)
    : head<Ints, Ts>(std::get<Ints>(p))...
    {}

    template<class U1, class U2>
    constexpr tuple_impl(std::pair<U1, U2> && p)
    : head<Ints, Ts>(std::get<Ints>(std::move(p)))...
    {}

    template<class U1, class U2>
    constexpr tuple_impl(std::pair<U1, U2> & p)
    : head<Ints, Ts>(std::get<Ints>(static_cast<std::pair<U1, U2> const &>(p)))...
    {}


    template<class Alloc>
    tuple_impl(std::allocator_arg_t, Alloc const & a)
    : head<Ints, Ts>(std::allocator_arg_t{}, a)...
    {}

    template<class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, Us &&... args)
    : head<Ints, Ts>(std::allocator_arg_t{}, a, std::forward<Us>(args))...
    {}

    template <class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, tuple_impl<index_sequence_, Us...> const & other)
    : head<Ints, Ts>(std::allocator_arg_t{}, a,
      static_cast<head<Ints, Us> const &>(other).get()
    )...
    {}

    template<class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, tuple_impl<index_sequence_, Us...> && other)
    : head<Ints, Ts>(std::allocator_arg_t{}, a, std::move(
      static_cast<head<Ints, Us>&&>(other).get()
    ))...
    {}

    template<class Alloc, class... Us>
    tuple_impl(std::allocator_arg_t, Alloc const & a, tuple_impl<index_sequence_, Us...> & other)
    : head<Ints, Ts>(std::allocator_arg_t{}, a, std::move(
      static_cast<head<Ints, Us>&>(other).get()
    ))...
    {}


    template <class Alloc, class U1, class U2>
    tuple_impl(std::allocator_arg_t, Alloc const & a, std::pair<U1, U2> const & p)
    : head<Ints, Ts>(std::allocator_arg_t{}, a, std::get<Ints>(p))...
    {}

    template <class Alloc, class U1, class U2>
    tuple_impl(std::allocator_arg_t, Alloc const & a, std::pair<U1, U2> && p)
    : head<Ints, Ts>(std::allocator_arg_t{}, a, std::get<Ints>(std::move(p)))...
    {}

    template <class Alloc, class U1, class U2>
    tuple_impl(std::allocator_arg_t, Alloc const & a, std::pair<U1, U2> & p)
    : head<Ints, Ts>(std::allocator_arg_t{}, a, std::get<Ints>(static_cast<std::pair<U1, U2> const &>(p)))...
    {}


    tuple_impl& operator=(tuple_impl const & other) = default;
    tuple_impl& operator=(tuple_impl && other) = default;

    template<class... Us>
    tuple_impl& operator=(tuple_impl<index_sequence_, Us...> const & other) {
      FALCON_UNPACK(
        static_cast<head<Ints, Ts>&>(*this).get()
        = static_cast<head<Ints, Us> const &>(other).get()
      );
      return *this;
    }

    template<class... Us>
    tuple_impl& operator=(tuple_impl<index_sequence_, Us...> && other) {
      FALCON_UNPACK(
        static_cast<head<Ints, Ts>&>(*this).get()
        = std::move(static_cast<head<Ints, Us>&>(other).get())
      );
      return *this;
    }


    template<class U1, class U2>
    tuple_impl& operator=(std::pair<U1, U2> const & p) {
      FALCON_UNPACK(
        static_cast<head<Ints, Ts>&>(*this).get()
        = std::get<Ints>(p)
      );
      return *this;
    }

    template<class U1, class U2>
    tuple_impl& operator=(std::pair<U1, U2> && p) noexcept {
      FALCON_UNPACK(
        static_cast<head<Ints, Ts>&>(*this).get()
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
        static_cast<head<Ints, Ts>&>(*this).get(),
        static_cast<head<Ints, Ts>&>(other).get()
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
  T get_i(head<I, T> const &);

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> &
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> & t) {
    return static_cast<head<I, decltype(get_i<I>(t))>&>(t).get();
  }

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> &&
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> && t) {
    using T = decltype(get_i<I>(t));
    return static_cast<T&&>(
      static_cast<head<I, T>&&>(t).get()
    );
  }

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> const &
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> const & t) {
    return static_cast<head<I, decltype(get_i<I>(t))> const &>(t).get();
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
  constexpr std::integral_constant<std::size_t, I> index_of_impl(head<I, T> const &);

  template<class T, class Tuple>
  struct tuple_index_of
  : decltype(index_of_impl<T>(std::declval<Tuple&>()))
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
      pack<head<Ints, Ts>...>(),
      pack<head<Ints, Us>...>(),
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
      pack<head<Ints, Ts>...>(),
      pack<head<Ints, Us>...>(),
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

  template<class PackRet, class IntsPack, class TuplesPack>
  struct tuple_concater;

  template<class... Es>
  struct tuple_concater<pack<Es...>, pack<>, pack<>>
  {
    template<class... Ts>
    constexpr static tuple<Es...> impl(Ts &&... args) {
      return tuple<Es...>(std::forward<Ts>(args)...);
    }
  };

  template<class... Es, std::size_t... Ints, class... TInts, class Tuple, class... Tuples>
  struct tuple_concater<
      pack<Es...>,
      pack<std::index_sequence<Ints...>, TInts...>,
      pack<Tuple, Tuples...>
  > {
    template<class... Ts>
    static constexpr auto
    impl(Tuple && t, Tuples &&... other, Ts &&... args) {
      return tuple_concater<
        pack<Es..., std::tuple_element_t<Ints, std::remove_reference_t<Tuple>>...>,
        pack<TInts...>,
        pack<Tuples...>
      >::impl(
        std::forward<Tuples>(other)...,
        std::forward<Ts>(args)...,
        get<Ints>(std::forward<Tuple>(t))...
      );
    }
  };
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

}

#endif
