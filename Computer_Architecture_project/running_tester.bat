@echo off
cls

REM Define directories
set SRC_DIR=.
set OBJ_DIR=obj
set EXE_DIR=exe

REM Create the obj and exe directories if they don't exist
if not exist %OBJ_DIR% mkdir %OBJ_DIR%
if not exist %EXE_DIR% mkdir %EXE_DIR%

REM Compile utils.c into utils.o
echo Compiling utils.c...
gcc -c %SRC_DIR%/utils.c -o %OBJ_DIR%/utils.o
if errorlevel 1 exit /b

REM Compile cache.c into cache.o (depends on utils)
echo Compiling cache.c...
gcc -c %SRC_DIR%/cache.c -o %OBJ_DIR%/cache.o
if errorlevel 1 exit /b

REM Compile initialize.c into initialize.o (depends on cpu_structs.h)
echo Compiling initialize.c...
gcc -c %SRC_DIR%/initialize.c -o %OBJ_DIR%/initialize.o
if errorlevel 1 exit /b

REM Compile tester.c into tester.o (depends on initialize.h)
echo Compiling tester.c...
gcc -c %SRC_DIR%/tester.c -o %OBJ_DIR%/tester.o
if errorlevel 1 exit /b

REM Link all object files into an executable
echo Linking objects into tester.exe...
gcc %OBJ_DIR%/tester.o %OBJ_DIR%/initialize.o %OBJ_DIR%/cache.o %OBJ_DIR%/utils.o -o %EXE_DIR%/tester.exe


if errorlevel 1 exit /b
REM Run the executable (main_program.exe) from the exe folder
exe\tester.exe
if errorlevel 1 exit /b

REM Optional: Clean up object files after building the executable
REM del obj\*.o asm_programs/program.asm imem

echo Build and execution complete!
pause
