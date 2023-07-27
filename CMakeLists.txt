cmake_minimum_required(VERSION 3.26)
project(checkpoint C)

set(CMAKE_C_STANDARD 17)

add_executable(checkpoint main.c
        macros.h)
