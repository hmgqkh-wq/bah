cmake_minimum_required(VERSION 3.10)
project(xclipse_xeno_wrapper C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Release)

# Ensure .so ends up in build/
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(vulkan.xeno_wrapper SHARED
    src/intercept.c
    src/logger.c
    src/feature_dump.c
)

target_include_directories(vulkan.xeno_wrapper PRIVATE include)
target_link_libraries(vulkan.xeno_wrapper PRIVATE dl pthread)

# CMake will prepend "lib" -> final filename becomes libvulkan.xeno_wrapper.so
set_target_properties(vulkan.xeno_wrapper PROPERTIES
    OUTPUT_NAME "vulkan.xeno_wrapper"
)
