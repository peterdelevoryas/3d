cmake_minimum_required(VERSION 3.19)
project(3d)
find_package(Vulkan REQUIRED)

add_executable(3d main.c vulkan.c)

if (UNIX AND NOT APPLE)
    target_sources(3d PRIVATE xcb_window.c)
    target_link_libraries(3d xcb)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_link_libraries(3d Vulkan::Vulkan)
target_compile_options(3d PUBLIC -ffast-math -Wall -g)
