cmake_minimum_required(VERSION 3.31)
project(progettoAPI C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DEVAL -std=gnu11 -Wall -Werror -O2 -pipe -static -s")

add_executable(progettoAPI main.c)

target_link_libraries(progettoAPI m)
