cmake_minimum_required(VERSION 3.7)

project(gb)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})

message(STATUS "Found SDL2: <<${SDL2_INCLUDE_DIRS}>>")
message(STATUS "Found OpenGL: <<${OPENGL_INCLUDE_DIR}>>")

file(GLOB_RECURSE SRCS emu/*.c)
file(GLOB_RECURSE HDRS emu/*.h)

message(STATUS "Found sources: <<${SRCS}>>")
message(STATUS "Found headers: <<${HDRS}>>")

include_directories(gb ${HDRS})
add_executable(gb main.c ${SRCS})
target_link_libraries(gb ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})