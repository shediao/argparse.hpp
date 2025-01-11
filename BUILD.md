
## Build

```sh
cmake -DARGPARSE_BUILD_TESTS=ON -B build -S .
cmake --build build
ctest --test-dir build
```
