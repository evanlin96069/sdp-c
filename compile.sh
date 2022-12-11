#!/bin/sh -e

bin_dir=bin
lib_dir=lib
obj_dir=obj
src_dir=src

warnings="-Wall -Wextra -pedantic -Wno-unused-parameter -Wno-strict-aliasing"
cflags="-Iinclude $warnings -std=c99"

dbg=0
if [ $dbg = 1 ]; then
	cflags="$cflags -g3 -D_DEBUG"
else
	cflags="$cflags -O3 -s"
fi

if [ ! -d "$bin_dir" ]; then
  mkdir $bin_dir
fi

if [ ! -d "$lib_dir" ]; then
  mkdir $lib_dir
fi

if [ ! -d "$obj_dir" ]; then
  mkdir $obj_dir
fi

for file in $src_dir/*.c; do
  if [ "$file" != "$src_dir/utils/" ]; then
    gcc -c -o "$obj_dir/$(basename "$file" .c).o" $cflags "$file"
  fi
done

for file in $src_dir/utils/*.c; do
  gcc -c -o "$obj_dir/$(basename "$file" .c).o" $cflags "$file"
done

ar rcs $lib_dir/libdemo.a $obj_dir/*.o
rm -rf obj

gcc  $cflags -L"$lib_dir" -o"$bin_dir/sdp" "app/sdp.c" "$lib_dir/libdemo.a"
gcc  $cflags -L"$lib_dir" -o"$bin_dir/bits.test" "test/bits.test.c" "$lib_dir/libdemo.a"
gcc  $cflags -L"$lib_dir" -o"$bin_dir/demo.test" "test/demo.test.c" "$lib_dir/libdemo.a"

$bin_dir/bits.test
$bin_dir/demo.test
