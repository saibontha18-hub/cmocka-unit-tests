cmake_minimum_required(VERSION 3.16)
project(ring_buffer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Werror)

find_library(CMOCKA_LIB cmocka REQUIRED)
find_path(CMOCKA_INCLUDE_DIR cmocka.h REQUIRED)

add_library(ringbuf src/ring_buffer.c)
target_include_directories(ringbuf PUBLIC src)

add_library(hashmap src/hash_map.c)
target_include_directories(hashmap PUBLIC src)

add_executable(test_ring_buffer tests/test_ring_buffer.c)
target_include_directories(test_ring_buffer PRIVATE ${CMOCKA_INCLUDE_DIR} src)
target_link_libraries(test_ring_buffer PRIVATE ringbuf ${CMOCKA_LIB})

add_executable(test_hash_map tests/test_hash_map.c)
target_include_directories(test_hash_map PRIVATE ${CMOCKA_INCLUDE_DIR} src)
target_link_libraries(test_hash_map PRIVATE hashmap ${CMOCKA_LIB})

enable_testing()
add_test(NAME ring_buffer COMMAND test_ring_buffer)
add_test(NAME hash_map COMMAND test_hash_map)
