add_library(caldera INTERFACE)

target_link_libraries(caldera INTERFACE
	efsw::efsw
	quill::quill_module
	SDL3::SDL3-static
	SDL3::Headers
	vk-bootstrap::vk-bootstrap
	VulkanHppModule
	VulkanMemoryAllocatorHppModule
)

add_executable(engine main.cpp)
# Imported C++ modules are only resolved from a target's direct link list, not through INTERFACE libraries.
target_link_libraries(engine PRIVATE caldera quill::quill_module)
