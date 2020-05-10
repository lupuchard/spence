cmake_minimum_required(VERSION 3.10)
project(spence)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++17")

file(GLOB CORE_FILES core/*.cpp core/*.h)
file(GLOB MAP_FILES core/map/*.cpp core/map/*.h)
file(GLOB GAME_FILES core/game/*.cpp core/game/*.h)
file(GLOB UTIL_FILES core/util/*.cpp core/util/*.h)

include_directories(core core/map core/game core/util)
add_executable(spence ${CORE_FILES} ${MAP_FILES} ${GAME_FILES} ${UTIL_FILES})

target_link_libraries(spence sfml-system sfml-window sfml-graphics)
