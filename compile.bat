@echo off

set bin_dir=bin
set lib_dir=lib
set obj_dir=obj
set src_dir=src

if not exist %bin_dir% mkdir %bin_dir%
if not exist %lib_dir% mkdir %lib_dir%
if not exist %obj_dir% mkdir %obj_dir%

for /r %src_dir% %%f in (*.c) do (
  if /I not "%%~dpf"=="%src_dir%\utils\" (
    gcc -c -o "%obj_dir%\%%~nf.o" -I"include" "%%f"
  )
)

for /r %src_dir%\utils\ %%f in (*.c) do (
  gcc -c -o "%obj_dir%\%%~nf.o" -I"include" "%%f"
)

ar rcs %lib_dir%\libdemo.a %obj_dir%\*.o
rd /s /q %obj_dir%

gcc -I"include" -L"%lib_dir%" -o"%bin_dir%\sdp.exe" "app\sdp.c" "%lib_dir%\libdemo.a"
gcc -I"include" -L"%lib_dir%" -o"%bin_dir%\bits.test.exe" "test\bits.test.c" "%lib_dir%\libdemo.a"
gcc -I"include" -L"%lib_dir%" -o"%bin_dir%\demo.test.exe" "test\demo.test.c" "%lib_dir%\libdemo.a"

%bin_dir%\bits.test.exe
%bin_dir%\demo.test.exe
