#include <type_traits>

// #include <tuple>
// #include <array>
// #include <vector>
// #include <cassert>
#include <sstream>
#include <iostream>

// #include <functional> // std::cref

#include "rapidtuple/tuple.hpp"

template<std::size_t i>
using i_ = std::integral_constant<std::size_t, i>;

template<class, bool = 1>
struct S
{};

template<class T>
struct S<T, 0>
{
  template<class U> void operator = (U const &) {}
};

template<bool b>
void Check()
{
  (void)typename std::enable_if<b, int>::type{};
}

class nothrow {};
struct throwable
{
  throwable() {}
  throwable(throwable &&) {}
  throwable(throwable const &) {}
  throwable(nothrow) noexcept {}
  template<class T> throwable(T const &) {}
  throwable & operator = (throwable &&) { return *this; }
  throwable & operator = (throwable const &) { return *this; }
  template<class T> throwable & operator = (T const &) { return *this; }
};

#define CHECK_OP(a, b, op) do {                                         \
  auto const & xxxx_a = a;                                              \
  auto const & xxxx_b = b;                                              \
  if (!(xxxx_a op xxxx_b)) {                                            \
    std::cerr                                                           \
      << __FILE__ ":" << __LINE__ << ": error: " #a " " #op " " #b " [" \
      << xxxx_a << " " #op " " << xxxx_b << "]\n";                      \
    std::abort();                                                       \
  }                                                                     \
} while(0)

#define CHECK_EQUAL(a,b) CHECK_OP(a, b, ==)
#define CHECK_NE(a,b) CHECK_OP(a, b, !=)

template<class Tuple>
using tuple_size_t = typename std::tuple_size<Tuple>::type;

struct O
{
  O() { std::cout << "O()\n"; }
  O(int& i) = delete; // { std::cout << "O(" << i << "&)\n"; }
  O(int&& i) { std::cout << "O(" << i << "&&)\n"; }
  O(int const & i) = delete; // { std::cout << "O(" << i << " const &)\n"; }
  O(O &) { std::cout << "O(O &)\n"; }
  O(O &&) { std::cout << "O(O &&)\n"; }
  O(O const &) { std::cout << "O(O const &)\n"; }
  O&operator=(O &) { std::cout << "O=(O &)\n"; return *this; }
  O&operator=(O &&) { std::cout << "O=(O &&)\n"; return *this; }
  O&operator=(O const &) { std::cout << "O=(O const &)\n"; return *this; }
};

struct allocator {};

struct Oa
{
  Oa() { std::cout << "Oa()\n"; }
  Oa(int& i) = delete; // { std::cout << "Oa(" << i << "&)\n"; }
  Oa(int&& i) { std::cout << "Oa(" << i << "&&)\n"; }
  Oa(int const & i) = delete; // { std::cout << "Oa(" << i << " const &)\n"; }
  Oa(Oa &) { std::cout << "Oa(Oa &)\n"; }
  Oa(Oa &&) { std::cout << "Oa(Oa &&)\n"; }
  Oa(Oa const &) { std::cout << "Oa(Oa const &)\n"; }
  Oa&operator=(Oa &) { std::cout << "Oa=(Oa &)\n"; return *this; }
  Oa&operator=(Oa &&) { std::cout << "Oa=(Oa &&)\n"; return *this; }
  Oa&operator=(Oa const &) { std::cout << "Oa=(Oa const &)\n"; return *this; }

  Oa(allocator) { std::cout << "Oa()\n"; }
  Oa(int& i, allocator) = delete; // { std::cout << "Oa(" << i << "&)\n"; }
  Oa(int&& i, allocator) { std::cout << "Oa(" << i << "&&)\n"; }
  Oa(int const & i, allocator) = delete; // { std::cout << "Oa(" << i << " const &)\n"; }
  Oa(Oa &, allocator) { std::cout << "Oa(Oa &)\n"; }
  Oa(Oa &&, allocator) { std::cout << "Oa(Oa &&)\n"; }
  Oa(Oa const &, allocator) { std::cout << "Oa(Oa const &)\n"; }
};

namespace std
{
  template<class Alloc>
  struct uses_allocator<::Oa, Alloc>
  : std::true_type
  {};
}

struct explicit_noexcept
{
  explicit explicit_noexcept(int) noexcept
  {}
};

struct explicit_
{
  explicit explicit_(int)
  {}
};

struct empty {};
struct empty2 {};
struct empty_final final {};
struct empty_final2 final {};

template<template<class...> class Tuple, bool B>
void test_type()
{
  using plain = int;
  using ref = double &;
  using rref = char &&;
  using T = Tuple<plain, ref, rref>;

  S<i_<3>>{} = S<tuple_size_t<T>>{};
  S<i_<3>>{} = S<tuple_size_t<T const>>{};
  S<i_<3>>{} = S<tuple_size_t<T volatile>>{};
  S<i_<3>>{} = S<tuple_size_t<T const volatile>>{};

  S<falcon::tuple_element_t<0, T>>{} = S<plain>{};
  S<falcon::tuple_element_t<1, T>>{} = S<ref>{};
  S<falcon::tuple_element_t<2, T>>{} = S<rref>{};

  S<falcon::tuple_element_t<0, T const>>{} = S<plain const>{};
  S<falcon::tuple_element_t<1, T const>>{} = S<ref>{};
  S<falcon::tuple_element_t<2, T const>>{} = S<rref>{};

  S<falcon::tuple_element_t<0, T volatile>>{} = S<plain volatile>{};
  S<falcon::tuple_element_t<1, T volatile>>{} = S<ref>{};
  S<falcon::tuple_element_t<2, T volatile>>{} = S<rref>{};

  S<falcon::tuple_element_t<0, T const volatile>>{} = S<plain const volatile>{};
  S<falcon::tuple_element_t<1, T const volatile>>{} = S<ref>{};
  S<falcon::tuple_element_t<2, T const volatile>>{} = S<rref>{};

  i_<sizeof(Tuple<>)>{} = i_<1>{};
  i_<sizeof(Tuple<int, int>)>{} = i_<sizeof(int) * 2>{};
  i_<sizeof(Tuple<empty>)>{} = i_<1>{};
  i_<sizeof(Tuple<empty, empty>)>{} = i_<2>{};
  i_<sizeof(Tuple<empty, empty2>)>{} = i_<1>{};
  i_<sizeof(Tuple<empty_final, empty>)>{} = i_<1>{};
  i_<sizeof(Tuple<empty_final, empty_final2>)>{} = i_<2>{};

  falcon::tuple_index_of_t<int, Tuple<int>>{} = i_<0>{};
  falcon::tuple_index_of_t<int, Tuple<char, int>>{} = i_<1>{};
  falcon::tuple_index_of_t<int, Tuple<char, int, float>>{} = i_<1>{};

  falcon::tuple_indexes_of_t<int, Tuple<>>{} = std::index_sequence<>{};
  falcon::tuple_indexes_of_t<int, Tuple<int>>{} = std::index_sequence<0>{};
  falcon::tuple_indexes_of_t<int, Tuple<int, int>>{} = std::index_sequence<0, 1>{};
  falcon::tuple_indexes_of_t<int, Tuple<char, int, int>>{} = std::index_sequence<1, 2>{};

  using std::get;

  S<decltype(get<0>(std::declval<T>()))>{} = S<plain&&>{};
  S<decltype(get<1>(std::declval<T>()))>{} = S<ref>{};
  S<decltype(get<2>(std::declval<T>()))>{} = S<rref>{};

  S<decltype(get<0>(std::declval<T const>())), B>{} = S<plain const &&, B>{};
  S<decltype(get<1>(std::declval<T const>())), B>{} = S<ref, B>{};
  S<decltype(get<2>(std::declval<T const>())), B>{} = S<rref, B>{};

  S<decltype(get<0>(std::declval<T&>()))>{} = S<plain &>{};
  S<decltype(get<1>(std::declval<T&>()))>{} = S<ref>{};
  S<decltype(get<2>(std::declval<T&>()))>{} = S<rref &>{};

  S<decltype(get<0>(std::declval<T const &>()))>{} = S<plain const &>{};
  S<decltype(get<1>(std::declval<T const &>()))>{} = S<ref>{};
  S<decltype(get<2>(std::declval<T const &>()))>{} = S<rref &>{};

  S<decltype(get<plain>(std::declval<T>()))>{} = S<plain&&>{};
  S<decltype(get<ref  >(std::declval<T>()))>{} = S<ref>{};
  S<decltype(get<rref >(std::declval<T>()))>{} = S<rref>{};

  S<decltype(get<plain>(std::declval<T const>())), B>{} = S<plain const &&, B>{};
  S<decltype(get<ref  >(std::declval<T const>())), B>{} = S<ref, B>{};
  S<decltype(get<rref >(std::declval<T const>())), B>{} = S<rref, B>{};

  S<decltype(get<plain>(std::declval<T&>()))>{} = S<plain &>{};
  S<decltype(get<ref  >(std::declval<T&>()))>{} = S<ref>{};
  S<decltype(get<rref >(std::declval<T&>()))>{} = S<rref &>{};

  S<decltype(get<plain>(std::declval<T const &>()))>{} = S<plain const &>{};
  S<decltype(get<ref  >(std::declval<T const &>()))>{} = S<ref>{};
  S<decltype(get<rref >(std::declval<T const &>()))>{} = S<rref &>{};
}

using falcon::as_const;

template<class F, class... Ts>
auto is_callable(int, F f, Ts && ... args)
-> decltype((void)(f(std::forward<Ts>(args)...)), std::true_type{})
{ return {}; }

template<class F, class... Ts>
std::false_type is_callable(char, F, Ts && ...)
{ return {}; }

#ifdef IN_IDE_PARSER
#define DOT3
#else
#define DOT3 ...
#endif

template<template<class...> class Tuple>
struct tuple_cat_select
{
  template<class... Ts>
  static
  auto _(Ts && ... args)
  FALCON_DECLTYPE_AUTO_RETURN(
    std::tuple_cat(std::forward<Ts>(args)...)
  )
};

template<>
struct tuple_cat_select<falcon::tuple>
{
  template<class... Ts>
  static
  auto _(Ts && ... args)
  FALCON_DECLTYPE_AUTO_RETURN(
    falcon::tuple_cat(std::forward<Ts>(args)...)
  )
};

template<template<class...> class Tuple, class O, class... TArgs>
void test_cons_impl(TArgs const & ... args)
{
#define add_line std::cout << "--- l." << __LINE__ << '\n'
  {
    add_line;
    using T = Tuple<>;
    T t;
    T{/*bug(std::tuple) args DOT3, */t};
    T{/*bug(std::tuple) args DOT3, */std::move(t)};
    t = t;
    t = std::move(t);
    t.swap(t);
  }
  {
    add_line;
    using T = Tuple<empty_final>;
    T t;
    T{args DOT3, t};
    T{args DOT3, std::move(t)};
    t = t;
    t = std::move(t);
    t.swap(t);
  }
  {
    add_line;
    using T = Tuple<O>;
    T t;
    T{args DOT3, t};
    T{args DOT3, std::move(t)};
    t = t;
    t = std::move(t);
    t.swap(t);
  }
  {
    add_line;
    using T = Tuple<O>;
    T t;
    t = t;
    t = T{args DOT3, O(1)};
    std::get<0>(t) = 3;
    T{args DOT3, t};
    T{args DOT3, as_const(t)};
    T{args DOT3, std::move(t)};
  }
  {
    add_line;
    using T = Tuple<O&>;
    O r;
    T t{args DOT3, r};
    t = t;
    t = T{args DOT3, r};
    std::get<0>(t) = r;
    T{args DOT3, t};
    T{args DOT3, as_const(t)};
    T{args DOT3, std::move(t)};
  }
  {
    add_line;
    using T = Tuple<O const&>;
    O r;
    O const cr;
    T t{args DOT3, r};
    T ct{args DOT3, cr};
    T{args DOT3, t};
    T{args DOT3, as_const(t)};
    T{args DOT3, std::move(t)};
    T{args DOT3, ct};
    T{args DOT3, as_const(ct)};
    T{args DOT3, std::move(ct)};
  }
  {
    add_line;
    using T = Tuple<O&&>;
    O r;
    T t{args DOT3, O{}};
    t = t;
    t = std::move(t);
    std::get<0>(t) = r;
    std::get<0>(t) = O{};
    is_callable(1, [](auto & x) -> decltype(x = x){}, t) = std::true_type{};
    is_callable(1, [](auto & x) -> decltype(T{/*bug(std::tuple) args DOT3, */x}){}, t) = std::false_type{};
    is_callable(1, [](auto & x) -> decltype(T{/*bug(std::tuple) args DOT3, */as_const(x)}){}, t) = std::false_type{};
    T{args DOT3, std::move(t)};
  }
  {
    add_line;
    using T = Tuple<O,O>;
    using P = std::pair<O,O>;
    P r;

    T t{args DOT3, r};
    T{args DOT3, as_const(r)};
    T{args DOT3, t};
    T{args DOT3, std::move(t)};
    t = std::move(r);
    t = as_const(r);
    t = r;
    t = t;
    t = std::move(t);
  }
  {
    add_line;
    using T = Tuple<O,O>;
    using P = std::pair<O const, O const>;
    P r;

    T t{args DOT3, r};
    T{args DOT3, as_const(r)};
    T{args DOT3, t};
    T{args DOT3, std::move(t)};
    t = std::move(r);
    t = as_const(r);
    t = r;
    t = t;
    t = std::move(t);
  }
  {
    using T = Tuple<O, std::string>;
    using P = Tuple<O, char const *>;
    P r{O{}, ""};

    T t{args DOT3, r};
    T{args DOT3, as_const(r)};
    T{args DOT3, t};
    T{args DOT3, std::move(t)};
    t = std::move(r);
    t = as_const(r);
    t = r;
    t = t;
    t = std::move(t);
  }
  {
    add_line;
    using T = Tuple<std::array<O,2>>;
    T(args DOT3, {2,1});
  }
  {
    using T1 = Tuple<long, int>;
    using T2 = Tuple<long, int>;
    using falcon::get;
    T1 t1;
    CHECK_EQUAL(get<0>(t1), 0);
    CHECK_EQUAL(get<1>(t1), 0);
    CHECK_EQUAL(get<long>(t1), 0);
    CHECK_EQUAL(get<int>(t1), 0);
    t1 = T1{args DOT3, 1,2};
    CHECK_EQUAL(get<0>(t1), 1);
    CHECK_EQUAL(get<1>(t1), 2);
    CHECK_EQUAL(get<long>(t1), 1);
    CHECK_EQUAL(get<int>(t1), 2);
    T2 t2;
    CHECK_EQUAL(get<0>(t2), 0);
    CHECK_EQUAL(get<1>(t2), 0);
    CHECK_EQUAL(get<long>(t2), 0);
    CHECK_EQUAL(get<int>(t2), 0);
    using std::swap;
    swap(t1, t2);
    CHECK_EQUAL(get<0>(t1), 0);
    CHECK_EQUAL(get<1>(t1), 0);
    CHECK_EQUAL(get<long>(t1), 0);
    CHECK_EQUAL(get<int>(t1), 0);
    CHECK_EQUAL(get<0>(t2), 1);
    CHECK_EQUAL(get<1>(t2), 2);
    CHECK_EQUAL(get<long>(t2), 1);
    CHECK_EQUAL(get<int>(t2), 2);
    t1 = t2;
    CHECK_EQUAL(get<0>(t1), 1);
    CHECK_EQUAL(get<1>(t1), 2);
    CHECK_EQUAL(get<long>(t1), 1);
    CHECK_EQUAL(get<int>(t1), 2);
    t2 = std::move(t1);
    T1(args DOT3, std::move(t1));
    T1{args DOT3, t1};
    T1(args DOT3, std::move(t2));
    T1{args DOT3, t2};
  }

  // TODO
//   {
//     using T1 = std::pair<O, O>;
//     using T2 = Tuple<O>;
//     using T3 = Tuple<T1>;
//     using T4 = Tuple<T1, T2>;
//     using Tr1 = Tuple<O, O, O>;
//     using Tr2 = T3;
//     using Tr3 = Tuple<T1, T1, T2>;
//     tuple_cat_select<Tuple> Cat;
//     T1 t1;
//     T2 t2;
//     Tr1{Cat(t1, t2)};
//     Tr1{Cat(T1{}, T2{})};
//     Tr2{Cat(T3{})};
//     Tr3{Cat(T3{}, T4{})};
//   }
#undef add_line
}

template<template<class...> class Tuple>
void test_cons()
{
  test_cons_impl<Tuple, O>();
  test_cons_impl<Tuple, O>(std::allocator_arg_t{}, allocator{});
  test_cons_impl<Tuple, Oa>();
  test_cons_impl<Tuple, Oa>(std::allocator_arg_t{}, allocator{});
}

inline void test_special_cons()
{
  {
    using T = falcon::tuple<>;
    T t;
    T{std::allocator_arg_t{}, allocator{}, t};
    T{std::allocator_arg_t{}, allocator{}, std::move(t)};
  }
  {
    using U = falcon::tuple<int>;
    using T = falcon::tuple<U>;
    U t;
    T{t};
    T{std::move(t)};
  }
  {
    using U = falcon::tuple<int, int>;
    using T = falcon::tuple<U>;
    U t;
    T{t};
  }
  {
    auto sbuf = std::cout.rdbuf(nullptr);
    using T = falcon::tuple<O&&>;
    std::allocator_arg_t arg;
    allocator a;
    std::false_type no;
    std::true_type yes;
    T t{O{}};
    is_callable(1, [](auto & x) -> decltype(T{arg, a, x}){}, t) = no;
    is_callable(1, [](auto & x) -> decltype(T{arg, a, as_const(x)}){}, t) = no;
    is_callable(1, [](auto & x) -> decltype(T{arg, a, std::move(x)}){}, t) = yes;
    std::cout.rdbuf(sbuf);
  }
  {
    using T = falcon::tuple<>;
    std::array<int, 0> a;
    T t;
    t = t;
    t = std::move(t);
    t = a;
    t = std::move(a);
    T{t};
    T{std::move(t)};
    T{a};
    T{std::move(a)};
  }
    //t1 = T1{1,std::ignore}; TODO
//       rapidtuple::tuple<O,O>{std::ignore,O{}};

}

// print stack trace with asan
[[noreturn]] inline void boom()
{
  enum bad_test {};
  bad_test * ptr = nullptr;
  *ptr = bad_test{}; // Boom ! :D
  std::abort();
}

struct checkbuf
: std::streambuf
{
  explicit
  checkbuf(std::string s)
  : s_(std::move(s))
  {}

  ~checkbuf()
  { terminate(); }

  void terminate()
  {
    auto const sz = static_cast<std::streamsize>(s_.size());
    if (i_ < sz) {
      std::cerr.write(s_.data() + i_, sz - i_) << "\n";
      std::cerr.flush();
      boom();
    }
  }

protected:
  std::streamsize xsputn(const char_type* s, std::streamsize n) override
  {
    auto const sz = static_cast<std::streamsize>(s_.size());
    if (n + i_ > sz || memcmp(s_.data() + i_, s, std::size_t(n))) {
      int i2 = 0;
      int inl = 0;
      while (n + i_ < sz && s_[std::size_t(n)] == s[i2]) {
        if (s_[std::size_t(n)] == '\n') {
          inl = i2;
        }
        ++i_;
        ++i2;
      }
      std::cerr << "\n[\n";
      std::cerr.write(s + inl, n - inl);
      std::cerr << "\n -- differ to -- \n";
      std::streamsize i = std::streamsize(i_) - (i2 - inl);
      std::cerr.write(s_.data() + i, std::min(sz - i, std::streamsize(50)));
      std::cerr << "\n]\n";
      std::cerr.flush();

      boom();
    }
    i_ += n;
    return n;
  }

private:
  std::string s_;
  std::streamsize i_ = 0;
};

int main()
{
#if __cplusplus > FALCON_CXX_STD_14
# define cpp17_or_later 1
#else
# define cpp17_or_later 0
#endif

  test_type<std::tuple, cpp17_or_later>();
  test_type<falcon::tuple, true>();

  // TODO O and Oa with a member variable
  std::stringbuf sbuf1;
  auto old_sbuf = std::cout.rdbuf(&sbuf1);
  test_cons<std::tuple>();
  std::cout.rdbuf(old_sbuf);
  auto str = sbuf1.str();
  CHECK_NE(str.size(), 0u);
  //std::cerr << str << '\n';
  checkbuf sbuf2(std::move(str));
  std::cout.rdbuf(&sbuf2);
  test_cons<falcon::tuple>();
  std::cout.rdbuf(old_sbuf);
  sbuf2.terminate();

  // TODO tuple_cat internal copy/move

  test_special_cons();

  {
    int i;

    S<decltype(falcon::make_tuple(1, 3))>{} = S<falcon::tuple<int, int>>{};
    S<decltype(falcon::make_tuple(1, std::ref(i)))>{} = S<falcon::tuple<int, int&>>{};

    S<decltype(falcon::tie(i, i))>{} = S<falcon::tuple<int&, int&>>{};

    S<decltype(falcon::forward_as_tuple(1, i))>{} = S<falcon::tuple<int&&, int&>>{};

    S<decltype(falcon::tuple_cat(
      falcon::tuple<int, long>{},
      falcon::tuple<char, double>{},
      falcon::tuple<unsigned, float>{}
    ))>{} = S<falcon::tuple<int, long, char, double, unsigned, float>>{};

    S<decltype(falcon::tuple_cat(
      falcon::tuple<int, long>{},
      std::array<char, 3>{},
      falcon::tuple<unsigned, float>{}
    ))>{} = S<falcon::tuple<int, long, char, char, char, unsigned, float>>{};

    falcon::tuple_cat(
      std::pair<int, long>{},
      std::array<char, 3>{},
      std::tuple<unsigned, float>{}
    ) = falcon::tuple<int, long, char, char, char, unsigned, float>{};

    falcon::tuple_cat(std::pair<int, long>{}) = falcon::tuple<int, long>{};

    falcon::tuple_cat() = falcon::tuple<>{};
  }

  using tuple1 = falcon::tuple<int>;
  using tuple2 = falcon::tuple<unsigned>;
  Check<noexcept(tuple1())>();
  Check<noexcept(tuple1(1))>();
  Check<noexcept(tuple1(1u))>();
  Check<noexcept(tuple1(std::declval<tuple1>()))>();
  Check<noexcept(tuple1(std::declval<tuple1 &>()))>();
  Check<noexcept(tuple1(std::declval<tuple1 const &>()))>();
  Check<noexcept(tuple1(tuple2()))>();
  Check<noexcept(tuple1() = tuple2())>();
  Check<noexcept(tuple1() = std::array<int, 1>{})>();
  Check<falcon::is_nothrow_swappable<tuple1>::value>();
  Check<falcon::is_nothrow_swappable<tuple2>::value>();

  using tuple3 = falcon::tuple<int, int>;
  using tuple4 = falcon::tuple<unsigned, unsigned>;
  Check<noexcept(tuple3())>();
  Check<noexcept(tuple3(1, 1))>();
  Check<noexcept(tuple3(1u, 1))>();
  Check<noexcept(tuple3(std::declval<tuple3>()))>();
  Check<noexcept(tuple3(std::declval<tuple3 &>()))>();
  Check<noexcept(tuple3(std::declval<tuple3 const &>()))>();
  Check<noexcept(tuple3(tuple4()))>();
  Check<noexcept(tuple3() = tuple4())>();
  Check<noexcept(tuple3() = std::array<int, 2>{})>();
  Check<falcon::is_nothrow_swappable<tuple3>::value>();
  Check<falcon::is_nothrow_swappable<tuple4>::value>();

  using tuple5 = falcon::tuple<throwable>;
  Check< noexcept(tuple5(nothrow{}))>();
  Check<!noexcept(tuple5())>();
  Check<!noexcept(tuple5(1))>();
  Check<!noexcept(tuple5(throwable{}))>();
  Check<!noexcept(tuple5(std::declval<tuple5>()))>();
  Check<!noexcept(tuple5(std::declval<tuple5 &>()))>();
  Check<!noexcept(tuple5(std::declval<tuple5 const &>()))>();
  using tuple6 = falcon::tuple<throwable, throwable>;
  Check< noexcept(tuple6(nothrow{}, nothrow{}))>();
  Check<!noexcept(tuple6())>();
  Check<!noexcept(tuple6(1, 1))>();
  Check<!noexcept(tuple6(1, throwable{}))>();
  Check<!noexcept(tuple6(throwable{}, 1))>();
  Check<!noexcept(tuple6(throwable{}, throwable{}))>();
  Check<!noexcept(tuple6(std::declval<tuple6>()))>();
  Check<!noexcept(tuple6(std::declval<tuple6 &>()))>();
  Check<!noexcept(tuple6(std::declval<tuple6 const &>()))>();
  Check<!falcon::is_nothrow_swappable<tuple5>::value>();
  Check<!falcon::is_nothrow_swappable<tuple6>::value>();

  using tuple_string = falcon::tuple<std::string>;
  Check<!noexcept(tuple_string() = as_const(tuple_string()))>();
  Check< noexcept(tuple_string() = tuple_string())>();
  Check< noexcept(tuple_string() = {})>();



//   {
//     rapidtuple::tuple<std::vector<int>>{
//       std::allocator_arg_t{}, std::allocator<int>{}
//     };
//     std::allocator_arg_t arg{};
//     std::allocator<int> a{};
//     rapidtuple::tuple<std::vector<int>>{arg, a};
//   }
//
//   {
//     S<decltype(rapidtuple::tuple_cat(
//       rapidtuple::tuple<int>{},rapidtuple::tuple<float,double>{}
//     ))>() = S<rapidtuple::tuple<int,float,double>>();
//
//     S<decltype(rapidtuple::tuple_cat(
//       std::declval<rapidtuple::tuple<int>&>(), std::declval<rapidtuple::tuple<float,double>&>()
//     ))>() = S<rapidtuple::tuple<int,float,double>>();
//   }
//
//   {
//     int i;
//     S<decltype(rapidtuple::tie(i,i))>() = S<rapidtuple::tuple<int&, int&>>();
//     S<decltype(rapidtuple::forward_as_tuple(i,1))>() = S<rapidtuple::tuple<int&, int&&>>();
//   }
//
//   {
// #define TEST(T, I)\
//   std::integral_constant<std::size_t, I>() =\
//   std::integral_constant<std::size_t, rapidtuple::tuple_index_of<T, rapidtuple::tuple<int, float>>::value>()
//     TEST(int, 0);
//     TEST(float, 1);
// #undef TEST
//   }
//   {
//     rapidtuple::tuple<int,int> t{2,5};
//     int x = 0;
//     CHECK_EQUAL(7, ((void)(each_from_tuple([&x](auto e) { x+=e; }, t)), x));
//     CHECK_EQUAL(12, ((void)(each_from_tuple([&x](auto e) { x+=e; }, t, std::index_sequence<1>{})), x));
//     using rapidtuple::each_from_tuple;
//     CHECK_EQUAL(10, ((void)(each_from_tuple([&x](auto e) { x+=e; }, std::make_tuple(-2,3), std::index_sequence<0>{})), x));
//   }
//
//   {
//     rapidtuple::tuple<int,int> t{2,5};
//     CHECK_EQUAL(7, apply_from_tuple([](int x, int y) { return x + y; }, t));
//     CHECK_EQUAL(5, apply_from_tuple([](int x) { return x; }, t, std::index_sequence<1>{}));
//     using rapidtuple::apply_from_tuple;
//     CHECK_EQUAL(10, apply_from_tuple([](int x, int y, int z) { return x+y+z; }, std::make_tuple(-2,3,9)));
//   }
//
//   {
//     rapidtuple::tuple<int,int> t{2,5};
//     struct to_long { long operator()(int i) const { return i; } };
//     S<rapidtuple::tuple<long,long>>{} = S<decltype(transform_from_tuple(to_long{}, t))>{};
//     struct to_void { void operator()(int) const { } };
//     S<rapidtuple::tuple<rapidtuple::ignore_t>>{} = S<decltype(transform_from_tuple(to_void{}, t, std::index_sequence<0>{}))>{};
//   }
//
//   {
//     int x = 1;
//     S<rapidtuple::tuple<int const &>>{} = S<decltype(rapidtuple::make_tuple(std::cref(x)))>{};
//   }
//
//   {
//     rapidtuple::tuple<int,int> t1{2,4};
//     if (!(t1 == t1)) {
//       std::cerr << __LINE__ << "\n";
//       return 1;
//     }
//     if (t1 < t1) {
//       std::cerr << __LINE__ << "\n";
//       return 1;
//     }
//     rapidtuple::tuple<int,int> t2{4,2};
//     if (t2 == t1) {
//       std::cerr << __LINE__ << "\n";
//       return 1;
//     }
//     if (!(t1 < t2)) {
//       std::cerr << __LINE__ << "\n";
//       return 1;
//     }
//   }

  return 0;
}
