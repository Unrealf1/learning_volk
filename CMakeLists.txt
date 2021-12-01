cmake_minimum_required(VERSION 3.12)

project(vulkan_guide)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
# for some reason VMA don't have CMakeLists.txt in release archive
file(DOWNLOAD 
    "https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/CMakeLists.txt" 
    "${CMAKE_CURRENT_BINARY_DIR}/_deps/vulkanmemoryallocator-src/CMakeLists.txt"
)
file(DOWNLOAD
    "https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/src/CMakeLists.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/_deps/vulkanmemoryallocator-src/src/CMakeLists.txt"
)
file(DOWNLOAD
    "https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/src/VmaReplay/CMakeLists.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/_deps/vulkanmemoryallocator-src/src/VmaReplay/CMakeLists.txt"
)
file(DOWNLOAD
    "https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/src/Shaders/CMakeLists.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/_deps/vulkanmemoryallocator-src/src/Shaders/CMakeLists.txt"
)
CPMAddPackage("gh:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@2.3.0")
CPMAddPackage("gh:charles-lunarg/vk-bootstrap@0.4")
CPMAddPackage("gh:tinyobjloader/tinyobjloader@2.0.0rc9#v2.0.0rc9")
CPMAddPackage("gh:ocornut/imgui@1.85")



add_subdirectory(third_party/stb)

add_library(engine STATIC engine/window.cpp engine/logger.cpp engine/renderer.cpp)
target_link_libraries(engine SDL2 spdlog)

add_executable(main main.cpp)

target_link_libraries(main engine)
target_link_libraries(main spdlog glm SDL2 VulkanMemoryAllocator vk-bootstrap tinyobjloader) #imgui
