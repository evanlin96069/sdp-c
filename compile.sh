#!/bin/sh

bin_dir=bin
lib_dir=lib
obj_dir=obj
src_dir=src

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
    gcc -c -o "$obj_dir/$(basename "$file" .c).o" -I"include" "$file"
  fi
done

for file in $src_dir/utils/*.c; do
  gcc -c -o "$obj_dir/$(basename "$file" .c).o" -I"include" "$file"
done

ar rcs $lib_dir/libdemo.a $obj_dir/*.o
rm -rf obj

gcc -I"include" -L"$lib_dir" -o"$bin_dir/sdp" "app/sdp.c" "$lib_dir/libdemo.a"
gcc -I"include" -L"$lib_dir" -o"$bin_dir/bits.test" "test/bits.test.c" "$lib_dir/libdemo.a"
gcc -I"include" -L"$lib_dir" -o"$bin_dir/demo.test" "test/demo.test.c" "$lib_dir/libdemo.a"

$bin_dir/bits.test
$bin_dir/demo.test
