cmake_minimum_required(VERSION 3.19)
project(3d)
find_package(Vulkan REQUIRED)


if (WIN32)
    add_executable(3d WIN32 main.c)
    target_sources(3d PRIVATE win32_window.c)
elseif (UNIX AND NOT APPLE)
    add_executable(3d main.c)
    target_link_libraries(3d xcb)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_link_libraries(3d Vulkan::Vulkan)
target_compile_options(3d PUBLIC -ffast-math)
