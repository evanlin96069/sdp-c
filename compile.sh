#!/bin/sh -e

CC=gcc

mkdir -p bin
mkdir -p obj

dbg=0

if [ $dbg = 1 ]; then
	cflags="-g3 -Wall -Wextra -pedantic -std=c99"
else
	cflags="-O1 -Wall -Wextra -pedantic -std=c99"
fi

# clean up obj
rm -f obj/*.o

#compile obj files
src="\
	alloc.c
	bits.c
	demo.c
	net_message.c"
for i in $src; do
	$CC src/$i -Iinclude $cflags -c -o obj/${i%.c}.o
done

# libdemo.a
ar -rcs bin/libdemo.a obj/*.o

# sdp.c
$CC src/sdp.c bin/libdemo.a -Iinclude $cflags -o bin/sdp

# tests
test="\
	demo.test.c
	bits.test.c"

for i in $test; do
	$CC test/$i bin/libdemo.a $cflags -o bin/${i%.c}
done

# run tests
for i in $test; do
	bin/${i%.c}
done
