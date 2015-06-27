#include <type_traits>

#include <tuple>
#include <array>
#include <vector>
#include <cassert>
#include <sstream>
#include <iostream>
#include "tuple.hpp"

template<class>
struct Check {};

#define CHECK_EQUAL(a,b) do {\
  auto const & xxxx_a = a;\
  auto const & xxxx_b = b;\
  if (xxxx_a != xxxx_b) {\
    std::cerr << xxxx_a << "---" << __LINE__ << "---\n" << xxxx_b << "\n"; \
    return 1; \
  }\
} while(0)

int main() {
  {
#define TYPE int,double&,char&&
    using T1 = std::tuple<TYPE>;
    using T2 = rapidtuple::tuple<TYPE>;
#undef TYPE

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

#undef SAME
#undef TEST

#define SAME(t, Q)\
  Check<decltype(std::get<t>(std::declval<T1 Q>()))>() =\
  Check<decltype(rapidtuple::get<t>(std::declval<T2 Q>()))>()

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
    using T1 = rapidtuple::tuple<int, int>;
    using T2 = rapidtuple::tuple<long, long>;
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
  }

  {
    rapidtuple::tuple<int,int> t{2,5};
    int x = 0;
    CHECK_EQUAL(7, (apply_from_tuple(t, [&x](auto & e) { x+=e; }), x));
  }

  return 0;
}
