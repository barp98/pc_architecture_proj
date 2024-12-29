gcc -c pipeline.c -o pipeline.o
gcc -c pipeline_cpu.c -o pipeline_cpu.o
gcc -c cache.c -o cache.o
gcc pipeline.o pipeline_cpu.o cache.o -o pipeline.exe
