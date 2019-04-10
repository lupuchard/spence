cmake_minimum_required(VERSION 3.3)
project(spence)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fsanitize=address")

file(GLOB CORE_FILES core/*.cpp core/*.h)
file(GLOB MAP_FILES core/map/*.cpp core/map/*.h)

# Squirrel
add_custom_command(
        OUTPUT "${CMAKE_SOURCE_DIR}/core/SQUIRREL3/lib/libsquirrel.a"
        COMMAND make
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/core/SQUIRREL3"
)
add_custom_target(nutz ALL DEPENDS "${CMAKE_SOURCE_DIR}/core/SQUIRREL3/lib/libsquirrel.a")
link_directories(core/SQUIRREL3/lib)

add_executable(spence ${CORE_FILES} ${MAP_FILES} core/UI.cpp core/UI.h)

target_link_libraries(spence sfml-system sfml-window sfml-graphics squirrel sqstdlib)
add_dependencies(spence nutz)
