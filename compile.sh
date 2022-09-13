#!/bin/sh -e

CC=gcc

mkdir -p bin
mkdir -p obj

warnings="-Wall -Wextra -pedantic -std=c99 -Wno-unused-parameter"

dbg=0
if [ $dbg = 1 ]; then
	cflags="-O0 -g3 -D_DEBUG"
else
	cflags="-O1"
fi

# clean up obj
rm -f obj/*.o

#compile obj files
src="\
	utils/alloc.c
	utils/vector.c
	utils/indent_writer.c
	bits.c
	demo.c
	demo_message.c
	net_message.c"
for i in $src; do
	filename="${i##*/}"
	$CC src/$i -Iinclude $warnings $cflags -c -o obj/${filename%.c}.o
done

# libdemo.a
ar -rcs bin/libdemo.a obj/*.o

# sdp.c
$CC src/sdp.c bin/libdemo.a -Iinclude $warnings $cflags -o bin/sdp

# tests
test="\
	demo.test.c
	bits.test.c"

for i in $test; do
	$CC test/$i bin/libdemo.a $warnings $cflags -o bin/${i%.c}
done

# run tests
for i in $test; do
	bin/${i%.c}
done
