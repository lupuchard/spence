cmake_minimum_required(VERSION 3.3)
project(spence)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fsanitize=address")

file(GLOB SOURCE_FILES core/*.cpp core/*.h)
#set(SOURCE_FILES main.cpp Grid.h Tile.h Map.h Map.cpp Vec.h Unit.h Renderer.h SFMLRenderer.cpp
#        SFMLRenderer.h SFMLEventManager.cpp SFMLEventManager.h Squirrel.cpp Squirrel.h)

# Squirrel
add_custom_command(
        OUTPUT "${CMAKE_SOURCE_DIR}/core/SQUIRREL3/lib/libsquirrel.a"
        COMMAND make
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/core/SQUIRREL3"
)
add_custom_target(nutz ALL DEPENDS "${CMAKE_SOURCE_DIR}/core/SQUIRREL3/lib/libsquirrel.a")
link_directories(core/SQUIRREL3/lib)

#add_definitions(-DSFML_STATIC)

# SFML
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
#find_package(SFML 2.4.2 REQUIRED system window graphics)
#include_directories("${CMAKE_SOURCE_DIR}/SFML-2.4.2/include")
#link_directories(SFML-2.4.2/lib SQUIRREL3/lib)

add_executable(spence ${SOURCE_FILES})

target_link_libraries(spence sfml-system sfml-window sfml-graphics squirrel sqstdlib)
add_dependencies(spence nutz)
