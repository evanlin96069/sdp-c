@echo off

set bin_dir=bin
set lib_dir=lib
set obj_dir=obj
set src_dir=src

set warnings=-Wall -Wextra -pedantic -Wno-unused-parameter -Wno-strict-aliasing
set cflags=-I"include" %warnings% -std=c99

set dbg=0

if %dbg% == 1 (
    set cflags=%cflags% -O0 -g3 -D_DEBUG
) else (
    set cflags=%cflags% -O3 -s
)

if not exist %bin_dir% mkdir %bin_dir%
if not exist %lib_dir% mkdir %lib_dir%
if not exist %obj_dir% mkdir %obj_dir%

for /r %src_dir% %%f in (*.c) do (
  if /I not "%%~dpf"=="%src_dir%\utils\" (
    gcc -c -o "%obj_dir%\%%~nf.o" %cflags% "%%f"
  )
)

for /r %src_dir%\utils\ %%f in (*.c) do (
  gcc -c -o "%obj_dir%\%%~nf.o" %cflags% "%%f"
)

ar rcs %lib_dir%\libdemo.a %obj_dir%\*.o
windres "app\sdp.rc" -O coff --use-temp-file -o "%obj_dir%\sdp.res"

gcc %cflags% -o"%bin_dir%\sdp.exe" "app\sdp.c" "%obj_dir%\sdp.res" "%lib_dir%\libdemo.a"
gcc %cflags% -o"%bin_dir%\bits.test.exe" "test\bits.test.c" "%lib_dir%\libdemo.a"
gcc %cflags% -o"%bin_dir%\demo.test.exe" "test\demo.test.c" "%lib_dir%\libdemo.a"

%bin_dir%\bits.test.exe
%bin_dir%\demo.test.exe

rd /s /q %obj_dir%
