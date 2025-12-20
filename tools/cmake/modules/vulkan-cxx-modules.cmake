find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanMemoryAllocator-Hpp CONFIG REQUIRED)

# Temporary, until Vulkan-Headers correctly installs the module
add_library(VulkanHppModule)
target_sources(VulkanHppModule PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${Vulkan_INCLUDE_DIR}
  FILES 
	${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
	${Vulkan_INCLUDE_DIR}/vulkan/vulkan_video.cppm
)
target_compile_features(VulkanHppModule PUBLIC cxx_std_23)
target_link_libraries(VulkanHppModule PUBLIC Vulkan::Headers)
target_compile_definitions(VulkanHppModule PUBLIC
	VULKAN_HPP_NO_EXCEPTIONS
	VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
)

add_library(VulkanMemoryAllocatorHppModule)
target_sources(VulkanMemoryAllocatorHppModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${Vulkan_INCLUDE_DIR}/
	FILES ${Vulkan_INCLUDE_DIR}/vk_mem_alloc.cppm
)
target_compile_features(VulkanMemoryAllocatorHppModule PUBLIC cxx_std_23)
target_link_libraries(VulkanMemoryAllocatorHppModule PUBLIC
	Vulkan::Vulkan
	VulkanHppModule
	GPUOpen::VulkanMemoryAllocator
	VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
)
