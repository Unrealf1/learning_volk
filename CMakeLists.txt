cmake_minimum_required(VERSION 3.12)

project(vulkan_guide)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#FindVulkan()
find_package(Vulkan REQUIRED)

include(cmake/CPM.cmake)

CPMAddPackage("gh:g-truc/glm@0.9.9.8#0.9.9.8")
CPMAddPackage(
  NAME      "SDL2"         
  VERSION   "2.0.18"
  OPTIONS
    "SDL_X11 ON"
  URL "https://www.libsdl.org/release/SDL2-2.0.18.zip"
)
CPMAddPackage("gh:gabime/spdlog@1.9.2")
CPMAddPackage("gh:charles-lunarg/vk-bootstrap@0.4")
CPMAddPackage("gh:tinyobjloader/tinyobjloader@2.0.0rc9#v2.0.0rc9")
CPMAddPackage("gh:ocornut/imgui@1.85")


add_subdirectory(third_party/stb)
add_subdirectory(third_party/vma)

add_library(engine STATIC engine/window.cpp engine/logger.cpp engine/renderer.cpp)
target_link_libraries(engine Vulkan::Vulkan SDL2 spdlog vk-bootstrap)

add_executable(main main.cpp)

target_link_libraries(main engine)
target_link_libraries(main spdlog glm SDL2 VulkanMemoryAllocator vk-bootstrap tinyobjloader) #imgui
