@echo off
cls
REM Create the obj and exe directories if they don't exist
if not exist obj mkdir obj
if not exist exe mkdir exe

REM Compile source files into object files inside the obj folder
gcc -c utils.c -o running_files/obj/utils.o
if errorlevel 1 exit /b

gcc -c pipeline.c -o running_files/obj/pipeline.o
if errorlevel 1 exit /b

gcc -c state_machine.c -o running_files/obj/state_machine.o
if errorlevel 1 exit /b

gcc -c cache/cache.c -o obj/cache.o
if errorlevel 1 exit /b

REM Link the object files to create the executable inside the exe folder
gcc running_files/obj/pipeline.o running_files/obj/state_machine.o running_files/obj/cache.o running_files/obj/utils.o -o running_files/exe/pipeline.exe
if errorlevel 1 exit /b

REM Run the executable (pipeline.exe) from the exe folder
running_files\exe\pipeline.exe
if errorlevel 1 exit /b

REM Optional: Clean up object files after building the executable
REM del obj\*.o

echo Build and execution complete!
pause
