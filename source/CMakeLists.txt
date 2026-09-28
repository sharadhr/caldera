add_library(caldera INTERFACE)

target_link_libraries(caldera INTERFACE
	efsw::efsw
	SDL3::SDL3-static
	SDL3::Headers
	vk-bootstrap::vk-bootstrap
	VulkanHppModule
	VulkanMemoryAllocatorHppModule
)

add_executable(engine main.cpp)
target_link_libraries(engine PRIVATE caldera)
