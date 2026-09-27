add_library(caldera)

target_link_libraries(caldera PRIVATE
	efsw::efsw
	QuillStatic
	SDL3::SDL3-static
	SDL3::Headers
	vk-bootstrap::vk-bootstrap
	VulkanHppModule
	VulkanMemoryAllocatorHppModule
)

add_executable(engine main.cpp)
