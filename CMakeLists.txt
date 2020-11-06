cmake_minimum_required(VERSION 3.7)

project(gb)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

message(STATUS "Found SDL2: <<${SDL2_INCLUDE_DIRS}>>")
message(STATUS "Found OpenGL: <<${OPENGL_INCLUDE_DIR}>>")

file(GLOB_RECURSE EMU_SOURCES emu/*.c)
file(GLOB_RECURSE EMU_HEADERS emu/*.h)

file(GLOB_RECURSE DRIVER_SOURCES driver/*.c)
file(GLOB_RECURSE DRIVER_HEADERS driver/*.h)

file(GLOB_RECURSE COMMON_SOURCES common/*.c)
file(GLOB_RECURSE COMMON_HEADERS common/*.h)

include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} stb common driver emu)

add_executable(gb main.c ${DRIVER_SOURCES} ${DRIVER_HEADERS} ${EMU_SOURCES} ${EMU_HEADERS} ${COMMON_SOURCES} ${COMMON_HEADERS})
target_link_libraries(gb ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})