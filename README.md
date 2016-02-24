Implementation of tuple with low memory usage of compiler and a quick compilation (C++14).

[![Build Status](https://travis-ci.org/jonathanpoelen/RapidTuple.svg?branch=master)](https://travis-ci.org/jonathanpoelen/RapidTuple)


Feature
-------

- `ignore_t` = `std::remove_const<decltype(std::ignore_t)>`

- `std::ignore` parameter does not initialize value

- `default_element` parameter use the default constructor

- `tuple_set<T...>`: Fails to compile if the tuple has more than one element of type T.

- `tuple_index_of<T, Tuple>`: Index of type T in Tuple.

- `Fn each_from_tuple(Fn, Tuple)`:  Call function on each element of a tuple.
- `Fn each_from_tuple(Fn, Tuple, std::index_sequence<I...>)`:  Call function on each index I of a tuple.

- `decltype(auto) apply_from_tuple(Fn, Tuple)`:  Call function with arguments from a tuple.
- `decltype(auto) apply_from_tuple(Fn, Tuple, std::index_sequence<I...>)`:  Call function with index I from a tuple.

- `tuple<decltype(fn(get<0>(tuple), ...))> transform_from_tuple(Fn, Tuple)`:  Same as `apply_from_tuple`, but returns a tuple. The type `void` is automatically converted to `ignore_t`.
- `tuple<decltype(fn(get<I>(tuple)...))> transform_from_tuple(Fn, Tuple, std::index_sequence<I...>)`:  Same, but with specified indexes.

- `index_sequence_from_tuple<Tuple>` = `std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>`

Documentation
-------------

See [std::tuple](http://en.cppreference.com/w/cpp/utility/tuple)


Bench
-----

```txt
./decl.cpp g++-4.9                          0:00.21s  38016k

./decl.cpp g++-4.9 -DNAMESPACE=std          0:00.63s  96316k
./decl.cpp g++-4.9 -DNAMESPACE=RapidTuple   0:00.40s  59008k

./get_index.cpp g++-4.9 -DNAMESPACE=std          0:00.99s  132564k
./get_index.cpp g++-4.9 -DNAMESPACE=RapidTuple   0:00.62s  75632k

./get_type.cpp g++-4.9 -DNAMESPACE=std          0:00.86s  113732k
./get_type.cpp g++-4.9 -DNAMESPACE=RapidTuple   0:00.72s  76948k


./decl.cpp  g++-5.0                          0:00.33s  40420k

./decl.cpp  g++-5.0 -DNAMESPACE=std          0:01.01s  91812k
./decl.cpp  g++-5.0 -DNAMESPACE=RapidTuple   0:00.64s  57656k

./get_index.cpp  g++-5.0 -DNAMESPACE=std          0:01.58s  119532k
./get_index.cpp  g++-5.0 -DNAMESPACE=RapidTuple   0:00.99s  70480k

./get_type.cpp  g++-5.0 -DNAMESPACE=std          0:01.39s  103940k
./get_type.cpp  g++-5.0 -DNAMESPACE=RapidTuple   0:01.10s  69184k


./decl.cpp clang++-3.5                          0:00.22s  40976k

./decl.cpp clang++-3.5 -DNAMESPACE=std          0:00.42s  53848k
./decl.cpp clang++-3.5 -DNAMESPACE=RapidTuple   0:00.30s  46732k

./get_index.cpp clang++-3.5 -DNAMESPACE=std          0:00.74s  76792k
./get_index.cpp clang++-3.5 -DNAMESPACE=RapidTuple   0:00.41s  54456k

./get_type.cpp clang++-3.5 -DNAMESPACE=std          0:00.61s  63720k
./get_type.cpp clang++-3.5 -DNAMESPACE=RapidTuple   0:00.44s  54176k


./decl.cpp clang++-3.6                          0:00.22s  42828k

./decl.cpp clang++-3.6 -DNAMESPACE=std          0:00.43s  55444k
./decl.cpp clang++-3.6 -DNAMESPACE=RapidTuple   0:00.30s  48712k

./get_index.cpp clang++-3.6 -DNAMESPACE=std          0:00.81s  78480k
./get_index.cpp clang++-3.6 -DNAMESPACE=RapidTuple   0:00.43s  56540k

./get_type.cpp clang++-3.6 -DNAMESPACE=std          0:00.63s  65328k
./get_type.cpp clang++-3.6 -DNAMESPACE=RapidTuple   0:00.44s  55892k
```


Differences
-----------

```c++
struct X
{
  X() = delete;
};

int main()
{
  using T1 = std::tuple<int, X>;
  using T2 = rapidtuple::tuple<int, X>;

  static_assert(std::is_constructible<X>::value, ""); // error
  static_assert(std::is_constructible<T1>::value, ""); // ok
  static_assert(std::is_constructible<T2>::value, ""); // ok

  X{}; // ok
  T1{}; // error
  T2{}; // ok
}
```

