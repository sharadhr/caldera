find_package(Vulkan REQUIRED)
add_library(VulkanHppModule)
target_sources(VulkanHppModule PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${Vulkan_INCLUDE_DIR}
  FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
target_compile_features(VulkanHppModule PUBLIC cxx_std_23)
target_link_libraries(VulkanHppModule PUBLIC Vulkan::Headers)
target_compile_definitions(VulkanHppModule PUBLIC
  VULKAN_HPP_NO_CONSTRUCTORS
)