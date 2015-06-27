#include <tuple>
#include "rapidtuple/tuple.hpp"

struct A{};
struct B{};
struct C{};
struct D{};
struct E{};
struct F{};
struct G{};
struct H{};
struct I{};
struct J{};
struct K{};
struct L{};
struct M{};
struct N{};
struct O{};
struct P{};
struct Q{};
struct R{};
struct S{};
struct T{};
struct U{};
struct V{};
struct W{};
struct X{};
struct Y{};
struct Z{};

int main() {
#ifdef NAMESPACE
  NAMESPACE::tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z>{};
  NAMESPACE::tuple<Z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y>{};
  NAMESPACE::tuple<Y,Z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X>{};
  NAMESPACE::tuple<X,Y,Z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W>{};
  NAMESPACE::tuple<W,X,Y,Z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V>{};
#endif
}
