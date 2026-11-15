@echo off
if not exist build\streamtree.exe call build.bat
if errorlevel 1 exit /b 1
if not exist data mkdir data
build\streamtree.exe --samples 3000 --drift-at 1500 --seed 42 --noise 0.05 --max-nodes 101 --grace 100 --output data\prequential_results.csv
