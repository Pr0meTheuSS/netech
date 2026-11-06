cmake_minimum_required(VERSION 3.16)

project(hw1 C)

# C11: достаточно для stdint.h / stddef.h из hw1.h.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Гигиена CI: предупреждения трактуются как ошибки.
# Флаги действуют и на библиотеку, и на тесты.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror)
endif()

# --- Санитайзеры (CI-джоб sanitizers) ----------------------------------
# Локально: cmake -B build-asan -DENABLE_SANITIZERS=ON
# ASan ловит выходы за границы памяти, UBSan — неопределённое поведение,
# LeakSanitizer (в составе ASan на Linux) — утечки памяти.
option(ENABLE_SANITIZERS "Собрать с -fsanitize=address,undefined" OFF)
if(ENABLE_SANITIZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "ENABLE_SANITIZERS: поддерживаются только GCC и Clang")
    endif()
    add_compile_options(-fsanitize=address,undefined
                           -fno-sanitize-recover=undefined
                           -fno-omit-frame-pointer -g)
    # Рантайм санитайзеров должен попасть в итоговый исполняемый файл,
    # поэтому флаг нужен и на этапе линковки.
    add_link_options(-fsanitize=address,undefined)
endif()

# Библиотека с реализацией. Преподаватель может подключать свои тесты,
# слинковав их с целью hw1 (интерфейс — include/).
add_library(hw1 STATIC src/hw1.c)
target_include_directories(hw1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- Тесты (ctest) -----------------------------------------------------

enable_testing()

add_executable(test_hw1 tests/test_hw1.c)
target_link_libraries(test_hw1 PRIVATE hw1)

add_test(NAME hw1 COMMAND test_hw1)
