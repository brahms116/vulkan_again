cmake_minimum_required(VERSION 3.5)

project(VulkanAgain)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(TINY_OBJ_LOADER_DIR "$ENV{HOME}/libs/tinyobjloader")

file(GLOB_RECURSE SRC_FILES ${SRC_DIR}/*.cpp ${SRC_DIR}/*.hpp)

add_executable(VulkanAgain ${SRC_FILES})

target_include_directories(VulkanAgain PRIVATE ${TINY_OBJ_LOADER_DIR})
target_link_libraries(VulkanAgain Vulkan::Vulkan glfw glm::glm)
