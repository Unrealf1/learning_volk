CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

project(vma)

CPMAddPackage(
    NAME                VulkanMemoryAllocator
    GITHUB_REPOSITORY   GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG             master
    DONWLOAD_ONLY       YES    
)

#add_library(VulkanMemoryAllocator INTERFACE)
#target_include_directories(VulkanMemoryAllocator INTERFACE ${VulkanMemoryAllocator_SOURCE_DIR}/include/)

