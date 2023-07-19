project(png-decoder)

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD             17)
set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)
set(GOOGLETEST_PATH "${CMAKE_SOURCE_DIR}/contrib/googletest" CACHE PATH "")

find_package(Catch REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/contrib/benchmark")

include(cmake/TestSolution.cmake)
include(cmake/BuildFlags.cmake)

find_package(PNG)

if (NOT DEFINED PNG_FOUND)
    message(FATAL_ERROR "Can't find libpng")
endif()

include(build.cmake)

add_catch(test_png_decoder test.cpp)
target_compile_definitions(test_png_decoder PUBLIC TASK_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(test_png_decoder PRIVATE ${PNG_INCLUDE_DIRS})
target_link_libraries(test_png_decoder ${PNG_STATIC} ${PNG_LIBRARY})
