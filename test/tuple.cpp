#include <type_traits>

// #include <tuple>
// #include <array>
// #include <vector>
// #include <cassert>
#include <sstream>
#include <iostream>

// #include <functional> // std::cref

#include "rapidtuple/tuple.hpp"

template<class, bool = 1>
struct S
{};

template<class T>
struct S<T, 0>
{
  template<class U> void operator = (U const &) {}
};

#define CHECK_EQUAL(a,b) do {\
  auto const & xxxx_a = a;\
  auto const & xxxx_b = b;\
  if (xxxx_a != xxxx_b) {\
    std::cerr << "--- line " << __LINE__ << " ---\n  " << #a << " != " #b << "\n  " << xxxx_a << " != " << xxxx_b << "\n"; \
    std::abort(); \
  }\
} while(0)

template<std::size_t i>
using i_ = std::integral_constant<std::size_t, i>;

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
  S<decltype(get<ref>(std::declval<T>()))>{} = S<ref>{};
  S<decltype(get<rref>(std::declval<T>()))>{} = S<rref>{};

  S<decltype(get<plain>(std::declval<T const>())), B>{} = S<plain const &&, B>{};
  S<decltype(get<ref>(std::declval<T const>())), B>{} = S<ref, B>{};
  S<decltype(get<rref>(std::declval<T const>())), B>{} = S<rref, B>{};

  S<decltype(get<plain>(std::declval<T&>()))>{} = S<plain &>{};
  S<decltype(get<ref>(std::declval<T&>()))>{} = S<ref>{};
  S<decltype(get<rref>(std::declval<T&>()))>{} = S<rref &>{};

  S<decltype(get<plain>(std::declval<T const &>()))>{} = S<plain const &>{};
  S<decltype(get<ref>(std::declval<T const &>()))>{} = S<ref>{};
  S<decltype(get<rref>(std::declval<T const &>()))>{} = S<rref &>{};
}

template<template<class...> class Tuple>
void test_cons(std::streambuf & sbuf)
{
  auto old_sbuf = std::cout.rdbuf(&sbuf);

  {
    using T = Tuple<O>;
    T t;
//     t = t;
//     t = T{O(1)};
    std::get<0>(t) = 3;
  }
  {
    using T = Tuple<O&>;
    O r;
    T t{r};
//     t = t;
//     t = T{r};
    std::get<0>(t) = r;
  }
  {
    using T = Tuple<O&&>;
    O r;
    T t{O{}};
//     t = t;
//     t = T{O{}};
    std::get<0>(t) = r;
    std::get<0>(t) = O{};
  }
//   {
//     using T = Tuple<O,O>;
//     using P = std::pair<O,O>;
//     P const cr;
//     P r;
//
//     T t{P{}};
//     T{cr};
//     T{r};
//     t = P{};
//     t = cr;
//     t = r;
//   }
//   {
//     using T = Tuple<std::array<O,2>>;
//     T({2,1});
//   }

  std::cout.rdbuf(old_sbuf);
}

int main()
{
#if __cplusplus > FALCON_CXX_STD_14
# define cpp17_or_later 1
#else
# define cpp17_or_later 0
#endif

  test_type<std::tuple, cpp17_or_later>();
  test_type<falcon::tuple, true>();

  std::stringbuf sbuf1;
  std::stringbuf sbuf2;

  test_cons<std::tuple>(sbuf1);
  test_cons<falcon::tuple>(sbuf2);

  CHECK_EQUAL(sbuf1.str(), sbuf2.str());

//   {
//     using T1 = rapidtuple::tuple<long, long>;
//     using T2 = rapidtuple::tuple<int, int>;
//     T1 t1;
//     t1 = T1{1,2};
//     t1 = T1{1,std::ignore};
//     T2 t2;
//     t2 = t1;
//     t2 = std::move(t1);
//     T1(std::move(t1));
//     T1{t1};
//     T1(std::move(t2));
//     T1{t2};
//   }
//   {
//     {
//       O{};
//       O t{};
//       O{std::move(t)};
//     }
//     std::string s = buf.str();
//     buf.str("");
//     {
//       rapidtuple::tuple<O,O>{std::ignore,O{}};
//     }
//     CHECK_EQUAL(s, buf.str());
//     buf.str("");
//   }

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
//     rapidtuple::tuple<>{}; // empty tuple
//     static_assert(sizeof(std::tuple<int>{}) == sizeof(rapidtuple::tuple<int>{}), "different size");
//     struct S {};
//     static_assert(sizeof(std::tuple<S>{}) == sizeof(rapidtuple::tuple<S>{}), "different size");
//     static_assert(sizeof(std::tuple<S,S>{}) == sizeof(rapidtuple::tuple<S,S>{}), "different size");
//     static_assert(sizeof(std::tuple<std::tuple<S,S>,S>{}) == sizeof(rapidtuple::tuple<rapidtuple::tuple<S,S>,S>{}), "different size");
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


  using T = falcon::tuple<>;
  using T_int = falcon::tuple<int>;
  using T_int_int = falcon::tuple<int, int>;

  T t{};
  T_int ti{};
  T_int_int tii{};

  T{t};

  T_int{1};
  T_int{ti};

  T_int_int{1, 1};
  T_int_int{tii};

  return 0;
}
