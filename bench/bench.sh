#!/bin/sh

[ "$#" = 0 ] && echo $0 compilers... && exit 1

d="$(dirname $0)"

compile() {
  echo -n "$@" '  ' ; /usr/bin/time --format="%Es  %Mk" "$2" -std=c++14 "$1" $3 -c -I"$d"/../include
}

for c in "$@" ; do
  compile "$d"/decl.cpp "$c" > /dev/null 2> /dev/null
  compile "$d"/decl.cpp "$c" '                      '
  echo
  for f in "$d"/*.cpp ; do
    compile "$f" "$c" -DNAMESPACE=std'       '
    compile "$f" "$c" -DNAMESPACE=rapidtuple
    echo
  done
  echo
done

rm "$d"/*.o
