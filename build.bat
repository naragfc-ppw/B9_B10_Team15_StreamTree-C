@echo off
if not exist build mkdir build
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude src\main.c src\stream_gen.c src\stats.c src\hoeffding_tree.c src\prequential.c src\drift.c -o build\streamtree.exe -lm
if errorlevel 1 (
  echo Build failed. Check that GCC is installed and available in PATH.
  exit /b 1
)
echo Build successful: build\streamtree.exe
