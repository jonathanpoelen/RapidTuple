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

#ifndef rapidtuple_TUPLE_HPP
#define rapidtuple_TUPLE_HPP

#include <tuple> //std::ignore_t, std::tuple_size, std::tuple_element
#include <utility>
#include <type_traits>
#include <initializer_list>

namespace rapidtuple {

namespace detail_ {
  template<class Ints, class... Ts>
  struct tuple_impl;

  template<class T>
  struct is_unique {};

  template<class... Ts>
  struct tuple_set_impl : is_unique<Ts>... { // error: you have defined a tuple_set with a type present more than once
    using type = tuple_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;
  };
}


template<class... Ts>
using tuple = detail_::tuple_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

template<class... Ts>
using tuple_set = typename detail_::tuple_set_impl<Ts...>::type;

constexpr struct default_element_t {
  constexpr default_element_t() {}
} default_element;

namespace detail_ {
  template<class T>
  struct remove_reference_wrapper
  { using type = T; };

  template<class T>
  struct remove_reference_wrapper<std::reference_wrapper<T>>
  { using type = T&; };
}


template<class... Ts>
constexpr tuple<detail_::remove_reference_wrapper<std::decay_t<Ts>>...>
make_tuple(Ts &&... args) {
  return {std::forward<Ts>(args)...};
}


template<class... Ts>
constexpr tuple<Ts&...> tie(Ts&... args) noexcept {
  return {args...};
}


template<class... Ts>
constexpr tuple<Ts&&...> forward_as_tuple(Ts&&... args) noexcept {
  return {std::forward<Ts>(args)...};
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
        std::make_index_sequence<std::tuple_size<Tuples>::value>...
      >,
      detail_::pack<Tuples...>
  >::impl(std::forward<Tuples>(tuples)...);
}


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

    constexpr head(decltype(std::ignore)) {}

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
    constexpr head(std::allocator_arg_t, Alloc const & a, decltype(std::ignore))
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
    constexpr head(decltype(std::ignore)) {}
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
    constexpr head(std::allocator_arg_t, Alloc const & a, decltype(std::ignore))
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


  template<std::size_t I, class T, class... Ts>
  struct at;

  template<std::size_t I, class, class, class, class, class, class... Ts>
  struct strip5 : at<I-5, Ts...>
  {};

  template<std::size_t I, class T, class... Ts>
  struct at : strip5<I-1, Ts...>
  {};

  template<class T, class... Ts>
  struct at<0, T, Ts...>
  { using type = T; };

  template<class T, class... Ts>
  struct at1 { using type = T; };
  template<class, class T, class... Ts>
  struct at2 { using type = T; };
  template<class, class, class T, class... Ts>
  struct at3 { using type = T; };
  template<class, class, class, class T, class... Ts>
  struct at4 { using type = T; };
  template<class, class, class, class, class T, class... Ts>
  struct at5 { using type = T; };

  template<class T, class... Ts> struct at<1, T, Ts...> : at1<Ts...> {};
  template<class T, class... Ts> struct at<2, T, Ts...> : at2<Ts...> {};
  template<class T, class... Ts> struct at<3, T, Ts...> : at3<Ts...> {};
  template<class T, class... Ts> struct at<4, T, Ts...> : at4<Ts...> {};
  template<class T, class... Ts> struct at<5, T, Ts...> : at5<Ts...> {};
}


namespace detail_ {
  using std::swap;

  template<class...> struct types_list;

  template<class T, class U>
  struct enable_if_convertible : std::enable_if<
      std::is_convertible<T, U>::value
  > {};

  template<class U> struct enable_if_convertible<decltype(std::ignore), U> { using type = void; };
  template<class U> struct enable_if_convertible<decltype(std::ignore)&, U> { using type = void; };
  template<class U> struct enable_if_convertible<decltype(std::ignore)&&, U> { using type = void; };

  template<std::size_t... Ints, class... Ts>
  struct tuple_impl<std::index_sequence<Ints...>, Ts...>
  : head<Ints, Ts>...
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
      (void)std::initializer_list<char>{(void((
        static_cast<head<Ints, Ts>&>(*this).get()
        = static_cast<head<Ints, Us> const &>(other).get()
      )), char())...};
      return *this;
    }

    template<class... Us>
    tuple_impl& operator=(tuple_impl<index_sequence_, Us...> && other) {
      (void)std::initializer_list<char>{(void((
        static_cast<head<Ints, Ts>&>(*this).get()
        = std::move(static_cast<head<Ints, Us>&>(other).get())
      )), char())...};
      return *this;
    }


    template<class U1, class U2>
    tuple_impl& operator=(std::pair<U1, U2> const & p) {
      (void)std::initializer_list<char>{(void((
        static_cast<head<Ints, Ts>&>(*this).get() = std::get<Ints>(p)
      )), char())...};
      return *this;
    }

    template<class U1, class U2>
    tuple_impl& operator=(std::pair<U1, U2> && p) noexcept {
      (void)std::initializer_list<char>{(void((
        static_cast<head<Ints, Ts>&>(*this).get() = std::get<Ints>(std::move(p))
      )), char())...};
      return *this;
    }


    void swap(tuple_impl & other)
    noexcept(noexcept(std::initializer_list<char>{(swap(
      static_cast<head<Ints, Ts>&>(other).get(),
      static_cast<head<Ints, Ts>&>(other).get()
    ), char())...})) {
      (void)(std::initializer_list<char>{(swap(
        static_cast<head<Ints, Ts>&>(other).get(),
        static_cast<head<Ints, Ts>&>(other).get()
      ), char())...});
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

  // inexplicable (tuple<int>({1}) -> tuple_impl<std::index_sequence<0>>) ???
  template<>
  struct tuple_impl<std::index_sequence<0>>
  {};

  template<std::size_t... TInts, class... Ts, std::size_t... UInts, class... Us>
  void swap(
    tuple_impl<std::index_sequence<TInts...>, Ts...> & t1,
    tuple_impl<std::index_sequence<UInts...>, Us...> & t2
  ) noexcept(noexcept(t1.swap(t2))) {
    t1.swap(t2);
  }


  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> &
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> & t) {
    return static_cast<head<
      I, typename at<I, Ts...>::type
    >&>(t).get();
  }

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> &&
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> && t) {
    using T = typename at<I, Ts...>::type;
    return static_cast<T&&>(
      static_cast<head<I, T>&&>(t).get()
    );
  }

  template<std::size_t I, class Ints, class... Ts>
  //tuple_element_t<I, tuple<Ts...>> const &
  constexpr decltype(auto) get(tuple_impl<Ints, Ts...> const & t) {
    return static_cast<head<
      I, typename at<I, Ts...>::type
    > const &>(t).get();
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


  template<class Fn, std::size_t... Ints, class... Ts>
  Fn apply_from_tuple(tuple_impl<std::index_sequence<Ints...>, Ts...> const & t, Fn fn)
  {
    void(std::initializer_list<int>{(void(
      fn(static_cast<head<Ints, Ts> const &>(t).get())
    ), 1)...});
    return fn;
  }

  template<class Fn, std::size_t... Ints, class... Ts>
  Fn apply_from_tuple(tuple_impl<std::index_sequence<Ints...>, Ts...> && t, Fn fn)
  {
    void(std::initializer_list<int>{(void(
      fn(static_cast<Ts&&>(static_cast<head<Ints, Ts> &>(t).get()))
    ), 1)...});
    return fn;
  }

  template<class Fn, std::size_t... Ints, class... Ts>
  Fn apply_from_tuple(tuple_impl<std::index_sequence<Ints...>, Ts...> & t, Fn fn)
  {
    void(std::initializer_list<int>{(void(
      fn(static_cast<head<Ints, Ts> &>(t).get())
    ), 1)...});
    return fn;
  }


  template<class Tuple1, class Tuple2>
  constexpr bool eq_impl(pack<>, pack<>, Tuple1 const &, Tuple2 const &) {
    return true;
  }

  template<class T, class... Ts, class U, class... Us, class Tuple1, class Tuple2>
  constexpr bool eq_impl(pack<T,Ts...>, pack<U,Us...>, Tuple1 const & t1, Tuple2 const & t2) {
    return static_cast<T const&>(t1).get() == static_cast<U const&>(t2).get()
        && eq_impl(pack<Ts...>(), pack<Us...>(), t1, t2);
  }

  template<std::size_t... TInts, class... Ts, std::size_t... UInts, class... Us>
  constexpr bool operator==(
    tuple_impl<std::index_sequence<TInts...>, Ts...> const & t1,
    tuple_impl<std::index_sequence<UInts...>, Us...> const & t2
  ) {
    return eq_impl(
      pack<head<TInts, Ts>...>(),
      pack<head<UInts, Us>...>(),
      t1, t2
    );
  }

  template<class Tuple1, class Tuple2>
  constexpr bool less_impl(pack<>, pack<>, Tuple1 const &, Tuple2 const &) {
    return false;
  }

  template<class T, class... Ts, class U, class... Us, class Tuple1, class Tuple2>
  constexpr bool less_impl(pack<T,Ts...>, pack<U,Us...>, Tuple1 const & t1, Tuple2 const & t2) {
    return static_cast<T const&>(t1).get() < static_cast<U const&>(t2).get()
        || (!(static_cast<T const&>(t1).get() < static_cast<U const&>(t2).get())
            && less_impl(pack<Ts...>(), pack<Us...>(), t1, t2));
  }

  template<std::size_t... TInts, class... Ts, std::size_t... UInts, class... Us>
  constexpr bool operator<(
    tuple_impl<std::index_sequence<TInts...>, Ts...> const & t1,
    tuple_impl<std::index_sequence<UInts...>, Us...> const & t2
  ) {
    return less_impl(
      pack<head<TInts, Ts>...>(),
      pack<head<UInts, Us>...>(),
      t1, t2
    );
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
using detail_::apply_from_tuple;


//template<class>
//struct tuple_size;

//template<class... Ts>
//struct tuple_size<tuple<Ts...>>
//: std::integral_constant<std::size_t, sizeof...(Ts)>
//{};

//template<class T>
//struct tuple_size<const T>
//: std::integral_constant<std::size_t, tuple_size<T>::value>
//{};

//template<class T>
//struct tuple_size<volatile T>
//: std::integral_constant<std::size_t, tuple_size<T>::value>
//{};

//template<class T>
//struct tuple_size<const volatile T>
//: std::integral_constant<std::size_t, tuple_size<T>::value>
//{};


//template<std::size_t I, class Tuple>
//struct tuple_element;

//template<std::size_t I, class... Ts>
//struct tuple_element<I, tuple<Ts...>>
//{ using type = typename detail_::at<I, Ts...>::type; };

//template<std::size_t I, class T>
//struct tuple_element<I, const T>
//{ using type = const typename std::tuple_element<I, T>::type; };

//template<std::size_t I, class T>
//struct tuple_element<I, volatile T>
//{ using type = volatile typename std::tuple_element<I, T>::type; };

//template< std::size_t I, class T>
//struct tuple_element<I, const volatile T>
//{ using type = const volatile typename std::tuple_element<I, T>::type; };

//template<std::size_t I, class Tuple>
//using tuple_element_t = typename tuple_element<I, Tuple>::type;


template<class... Ts, class... Us>
constexpr bool
operator!=(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
  return !(lhs == rhs);
}

template<class... Ts, class... Us>
constexpr bool
operator<=(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
  return !(rhs < lhs);
}

template<class... Ts, class... Us>
constexpr bool
operator>(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
  return rhs < lhs;
}

template<class... Ts, class... Us>
constexpr bool
operator>=(const tuple<Ts...>& lhs, const tuple<Us...>& rhs) {
  return !(lhs < rhs);
}


template<class... Ts>
void swap(tuple<Ts...> & t1, tuple<Ts...> & t2)
noexcept(noexcept(t1.swap(t2))) {
  t1.swap(t2);
}

}

namespace std {

template<std::size_t... Ints, class... Ts>
struct tuple_size< ::rapidtuple::detail_::tuple_impl<std::index_sequence<Ints...>, Ts...>>
: std::integral_constant<std::size_t, sizeof...(Ts)>
{};


template<std::size_t I, std::size_t... Ints, class... Ts>
struct tuple_element<
  I, ::rapidtuple::detail_::tuple_impl<std::index_sequence<Ints...>, Ts...>
>{ using type = typename ::rapidtuple::detail_::at<I, Ts...>::type; };


template<class... Ts, class Alloc>
struct uses_allocator< ::rapidtuple::tuple<Ts...>, Alloc>
: std::true_type
{};

}

#endif
