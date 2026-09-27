add_library(caldera)

target_sources(caldera
	PUBLIC FILE_SET CXX_MODULES
	FILES
		caldera/caldera.cppm
		caldera/Engine.cppm
)

target_link_libraries(caldera
	PRIVATE
		efsw::efsw
		vk-bootstrap::vk-bootstrap
		VulkanHppModule
		VulkanMemoryAllocatorHppModule
)

add_executable(engine)
target_sources(engine
	PRIVATE
		main.cpp
)
target_link_libraries(engine PRIVATE caldera SDL3::SDL3-static SDL3::Headers)
