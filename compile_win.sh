#!/bin/sh -e

CC="zig cc"

mkdir -p bin-win

warnings="-Wall -Wextra -pedantic -std=c99 -Wno-unused-parameter"

cflags="-O1 -target x86_64-windows-gnu"

$CC src/*.c src/utils/*.c -Iinclude $warnings $cflags -o bin-win/sdp.exe
