cmake -S. -Bbuild
cmake --build build
ctest --test-dir build -R Solver --output-on-failure -V
