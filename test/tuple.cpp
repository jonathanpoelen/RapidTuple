#include <type_traits>

#include <tuple>
#include <array>
#include <vector>
#include <cassert>
#include <sstream>
#include <iostream>

#include <functional> // std::cref

#include "rapidtuple/tuple.hpp"

template<class>
struct Check {};

#define CHECK_EQUAL(a,b) do {\
  auto const & xxxx_a = a;\
  auto const & xxxx_b = b;\
  if (xxxx_a != xxxx_b) {\
    std::cerr << "--- line " << __LINE__ << " ---\n  " << #a << " != " #b << "\n  " << xxxx_a << " != " << xxxx_b << "\n"; \
    return 1; \
  }\
} while(0)

int main() {
  {
#define TYPE int,double&,char&&
    using T1 = std::tuple<TYPE>;
    using T2 = rapidtuple::tuple<TYPE>;
#undef TYPE

    static_assert(std::tuple_size<T1>::value == std::tuple_size<T2>::value, "different size");
    static_assert(std::tuple_size<T1>::value == rapidtuple::tuple_size<T2>::value, "different size");

    Check<std::tuple_element_t<1, T1>>{} = Check<std::tuple_element_t<1, T2>>{};
    Check<std::tuple_element_t<1, T1>>{} = Check<rapidtuple::tuple_element_t<1, T2>>{};

#define SAME(i)\
  Check<std::tuple_element<i,T1>::type>() =\
  Check<std::tuple_element<i,T2>::type>()

    SAME(0);
    SAME(1);
    SAME(2);

#undef SAME

#define NIL

#define SAME(i, Q)\
  Check<decltype(std::get<i>(std::declval<T1 Q>()))>() =\
  Check<decltype(rapidtuple::get<i>(std::declval<T2 Q>()))>()

#define TEST(Q)\
  SAME(0,Q);\
  SAME(1,Q);\
  SAME(2,Q);

  TEST(NIL);
  TEST(&);
  TEST(&&);
  TEST(const &);

#undef TEST

#define TEST(Q)\
  SAME(int,Q);\
  SAME(double&,Q);\
  SAME(char&&,Q);

  TEST(NIL);
  TEST(&);
  TEST(&&);
  TEST(const &);

#undef SAME
#undef TEST
  }
  {
    using T1 = rapidtuple::tuple<long, long>;
    using T2 = rapidtuple::tuple<int, int>;
    T1 t1;
    t1 = T1{1,2};
    t1 = T1{1,std::ignore};
    T2 t2;
    t2 = t1;
    t2 = std::move(t1);
    T1(std::move(t1));
    T1{t1};
    T1(std::move(t2));
    T1{t2};
  }
  {
    struct T{
      T() { std::cout << "T()\n"; }
      T(int& i) { std::cout << "T(" << i << "&)\n"; }
      T(int&& i) { std::cout << "T(" << i << "&&)\n"; }
      T(int const & i) { std::cout << "T(" << i << " const &)\n"; }
      T(T &) { std::cout << "T(T &)\n"; }
      T(T &&) { std::cout << "T(T &&)\n"; }
      T(T const &) { std::cout << "T(T const &)\n"; }
      T&operator=(T &) { std::cout << "T=(T &)\n"; return *this; }
      T&operator=(T &&) { std::cout << "T=(T &&)\n"; return *this; }
      T&operator=(T const &) { std::cout << "T=(T const &)\n"; return *this; }
    };
    std::stringbuf buf;
    auto old_buf = std::cout.rdbuf(&buf);

    {
      using T1 = std::tuple<T>;
      using T2 = rapidtuple::tuple<T>;
      {
        T1 t;
        t = t;
        t = T1{T(1)};
        std::get<0>(t) = 3;
      }
      std::string s = buf.str();
      buf.str("");
      {
        T2 t;
        t = t;
        t = T2{T(1)};
        rapidtuple::get<0>(t) = 3;
      }
      CHECK_EQUAL(s, buf.str());
      buf.str("");
    }
    {
      using T1 = std::tuple<T&>;
      using T2 = rapidtuple::tuple<T&>;
      {
        T r;
        T1 t{r};
        t = t;
        t = T1{r};
        std::get<0>(t) = r;
      }
      std::string s = buf.str();
      buf.str("");
      {
        T r;
        T2 t{r};
        t = t;
        t = T2{r};
        rapidtuple::get<0>(t) = r;
      }
      CHECK_EQUAL(s, buf.str());
      buf.str("");
    }
    {
      using T1 = std::tuple<T&&>;
      using T2 = rapidtuple::tuple<T&&>;
      {
        T r;
        T1 t{T{}};
        t = t;
        t = T1{T{}};
        std::get<0>(t) = r;
        std::get<0>(t) = T{};
      }
      std::string s = buf.str();
      buf.str("");
      {
        T r;
        T2 t{T{}};
        t = t;
        t = T2{T{}};
        rapidtuple::get<0>(t) = r;
        rapidtuple::get<0>(t) = T{};
      }
      CHECK_EQUAL(s, buf.str());
      buf.str("");
    }
    {
      using T1 = std::tuple<T,T>;
      using T2 = rapidtuple::tuple<T,T>;
      using P = std::pair<T,T>;
      {
        P const cr;
        P r;

        T1 t{P{}};
        T1{cr};
        T1{r};
        t = P{};
        t = cr;
        t = r;
      }
      std::string s = buf.str();
      buf.str("");
      {
        P const cr;
        P r;

        T2 t{P{}};
        T2{cr};
        T2{r};
        t = P{};
        t = cr;
        t = r;
      }
      CHECK_EQUAL(s, buf.str());
      buf.str("");
    }
    {
      {
        T{};
        T t{};
        T{std::move(t)};
      }
      std::string s = buf.str();
      buf.str("");
      {
        rapidtuple::tuple<T,T>{std::ignore,T{}};
      }
      CHECK_EQUAL(s, buf.str());
      buf.str("");
    }
    {
      using T1 = std::tuple<std::array<T,2>>;
      using T2 = rapidtuple::tuple<std::array<T,2>>;
      {
        T1({2,1});
      }
      std::string s = buf.str();
      buf.str("");
      {
        T2({2,1});
      }
      CHECK_EQUAL(s, buf.str());
      buf.str("");
    }

    std::cout.rdbuf(old_buf);
  }

  {
    rapidtuple::tuple<std::vector<int>>{
      std::allocator_arg_t{}, std::allocator<int>{}
    };
    std::allocator_arg_t arg{};
    std::allocator<int> a{};
    rapidtuple::tuple<std::vector<int>>{arg, a};
  }

  {
    rapidtuple::tuple<>{}; // empty tuple
    static_assert(sizeof(std::tuple<int>{}) == sizeof(rapidtuple::tuple<int>{}), "different size");
    struct S {};
    static_assert(sizeof(std::tuple<S>{}) == sizeof(rapidtuple::tuple<S>{}), "different size");
    static_assert(sizeof(std::tuple<S,S>{}) == sizeof(rapidtuple::tuple<S,S>{}), "different size");
    static_assert(sizeof(std::tuple<std::tuple<S,S>,S>{}) == sizeof(rapidtuple::tuple<rapidtuple::tuple<S,S>,S>{}), "different size");
  }

  {
    Check<decltype(rapidtuple::tuple_cat(
      rapidtuple::tuple<int>{},rapidtuple::tuple<float,double>{}
    ))>() = Check<rapidtuple::tuple<int,float,double>>();

    Check<decltype(rapidtuple::tuple_cat(
      std::declval<rapidtuple::tuple<int>&>(), std::declval<rapidtuple::tuple<float,double>&>()
    ))>() = Check<rapidtuple::tuple<int,float,double>>();
  }

  {
    int i;
    Check<decltype(rapidtuple::tie(i,i))>() = Check<rapidtuple::tuple<int&, int&>>();
    Check<decltype(rapidtuple::forward_as_tuple(i,1))>() = Check<rapidtuple::tuple<int&, int&&>>();
  }

  {
#define TEST(T, I)\
  std::integral_constant<std::size_t, I>() =\
  std::integral_constant<std::size_t, rapidtuple::tuple_index_of<T, rapidtuple::tuple<int, float>>::value>()
    TEST(int, 0);
    TEST(float, 1);
#undef TEST
  }
  {
    rapidtuple::tuple<int,int> t{2,5};
    int x = 0;
    CHECK_EQUAL(7, (each_from_tuple([&x](auto e) { x+=e; }, t), x));
    CHECK_EQUAL(12, (each_from_tuple([&x](auto e) { x+=e; }, t, std::index_sequence<1>{}), x));
    using rapidtuple::each_from_tuple;
    CHECK_EQUAL(10, (each_from_tuple([&x](auto e) { x+=e; }, std::make_tuple(-2,3), std::index_sequence<0>{}), x));
  }

  {
    rapidtuple::tuple<int,int> t{2,5};
    CHECK_EQUAL(7, apply_from_tuple([](int x, int y) { return x + y; }, t));
    CHECK_EQUAL(5, apply_from_tuple([](int x) { return x; }, t, std::index_sequence<1>{}));
    using rapidtuple::apply_from_tuple;
    CHECK_EQUAL(10, apply_from_tuple([](int x, int y, int z) { return x+y+z; }, std::make_tuple(-2,3,9)));
  }

  {
    rapidtuple::tuple<int,int> t{2,5};
    struct to_long { long operator()(int i) const { return i; } };
    Check<rapidtuple::tuple<long,long>>{} = Check<decltype(transform_from_tuple(to_long{}, t))>{};
    struct to_void { void operator()(int) const { } };
    Check<rapidtuple::tuple<rapidtuple::ignore_t>>{} = Check<decltype(transform_from_tuple(to_void{}, t, std::index_sequence<0>{}))>{};
  }

  {
    int x = 1;
    Check<rapidtuple::tuple<int const &>>{} = Check<decltype(rapidtuple::make_tuple(std::cref(x)))>{};
  }

  {
    rapidtuple::tuple<int,int> t1{2,4};
    if (!(t1 == t1)) {
      std::cerr << __LINE__ << "\n";
      return 1;
    }
    if (t1 < t1) {
      std::cerr << __LINE__ << "\n";
      return 1;
    }
    rapidtuple::tuple<int,int> t2{4,2};
    if (t2 == t1) {
      std::cerr << __LINE__ << "\n";
      return 1;
    }
    if (!(t1 < t2)) {
      std::cerr << __LINE__ << "\n";
      return 1;
    }
  }

  return 0;
}
