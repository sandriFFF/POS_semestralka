cmake_minimum_required(VERSION 4.1)
project(semestralka C)

set(CMAKE_C_STANDARD 11)
add_executable(semestralka main.c
        had.c
        had.h
        klient.c
        klient.h
        server.c
        server.h
        shared_memory.c
        shared_memory.h
        hra.c
        hra.h)
