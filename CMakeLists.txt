cmake_minimum_required(VERSION 3.25)
project(semestralka C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(semestralka
        main.c
        klient.c
        server.c
        shared_memory.c
)

find_package(Threads REQUIRED)
target_link_libraries(semestralka PRIVATE Threads::Threads)

find_package(Curses REQUIRED)
target_include_directories(semestralka PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(semestralka PRIVATE ${CURSES_LIBRARIES})
