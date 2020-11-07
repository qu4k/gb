cmake_minimum_required(VERSION 3.7)

project(gb)

# VARs

set(CIMGUI vendor/cimgui)
set(IMGUI_IMPL ${CIMGUI}/imgui/examples)
set(GL3W vendor/gl3w)

# LIBS

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

message(STATUS "Found SDL2: <<${SDL2_INCLUDE_DIRS}>>")
message(STATUS "Found OpenGL: <<${OPENGL_INCLUDE_DIR}>>")

include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
include_directories(
	${CIMGUI} 
	${CIMGUI}/generator/output 
	${CIMGUI}/imgui ${IMGUI_IMPL}
  ${GL3W}/include)

# CIMGUI

file(GLOB 
	CIMGUI_SOURCES 
	"${CIMGUI}/*.cpp" 
	"${CIMGUI}/imgui/*.cpp"
	"${IMGUI_IMPL}/imgui_impl_sdl.cpp"
	"${IMGUI_IMPL}/imgui_impl_opengl3.cpp")
add_library(cimgui ${CIMGUI_SOURCES})
target_compile_definitions(cimgui PRIVATE 
	IMGUI_IMPL_API=extern\ \"C\"
	IMGUI_IMPL_OPENGL_LOADER_GL3W)

# GB

set(GL3W_SOURCES ${GL3W}/src/gl3w.c)

file(GLOB_RECURSE EMU_SOURCES emu/*.c emu/*.h)
include_directories(emu)

file(GLOB_RECURSE DRIVER_SOURCES driver/*.c driver/*.h)
include_directories(driver)

file(GLOB_RECURSE COMMON_SOURCES common/*.c common/*.h)
include_directories(common)

add_executable(gb main.c ${DRIVER_SOURCES} ${EMU_SOURCES} ${COMMON_SOURCES} ${GL3W_SOURCES})
target_compile_definitions(gb PRIVATE 
	IMGUI_IMPL_API=\ )
target_link_libraries(gb ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} cimgui)

set_target_properties(gb
    PROPERTIES
    C_STANDARD 11
    )