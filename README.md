# cmocka-unit-tests

A tiny example of unit-testing embedded-style C with the [CMocka](https://cmocka.org) framework. The module under test is a fixed-capacity byte ring (circular) buffer written in strict C99 with no dynamic allocation — the kind of building block that shows up in UART drivers and data loggers.

The test suite covers: initial state, FIFO put/get ordering, overflow rejection (buffer contents preserved), underflow rejection (output untouched), index wrap-around, and repeated fill/drain cycles.

## Prerequisites

- GCC, CMake 3.16+
- CMocka development files:
  - Debian/Ubuntu: `sudo apt install libcmocka-dev`
  - Fedora: `sudo dnf install libcmocka-devel`
  - macOS: `brew install cmocka`

## Build and run

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected output ends with `100% tests passed, 0 tests failed`.

For CI, the same three commands work on any runner with the prerequisites installed.

## Files

- `src/ring_buffer.h`, `src/ring_buffer.c` — the module under test
- `tests/test_ring_buffer.c` — CMocka test suite
- `CMakeLists.txt` — builds the library, the test binary, and registers it with CTest

## License

MIT
