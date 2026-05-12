cmake_minimum_required(VERSION 3.16)

project(TerracraftBotSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(task
    src/Main.cpp
    src/InputParser.cpp
    src/Dungeon.cpp
    src/AliceBot.cpp
    src/OutputWriter.cpp
)

target_include_directories(task PRIVATE src)

target_compile_options(task PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
