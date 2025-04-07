
cmake_minimum_required(VERSION 3.1)
project(mco0 LANGUAGES C ASM)

add_executable(led.elf main.c start_up.s)
target_include_directories(led.elf PRIVATE .)
target_link_options(led.elf PRIVATE -T ${CMAKE_SOURCE_DIR}/linker.ld)
