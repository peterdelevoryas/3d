cmake_minimum_required(VERSION 3.19)
project(3d)
find_package(Vulkan REQUIRED)

add_executable(3d WIN32 main.c)
target_link_libraries(3d Vulkan::Vulkan)
target_compile_options(3d PUBLIC -ffast-math)
