cmake_minimum_required(VERSION 4.3)
project(moviesearch C)

set(CMAKE_C_STANDARD 23)

add_executable(moviesearch main.c
        parser.h
        parser.c
        common.h)
