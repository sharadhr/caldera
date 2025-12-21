add_library(caldera)
target_sources(caldera
	PUBLIC FILE_SET CXX_MODULES
		FILES 
			Caldera.cpp
			Util.cpp
	PRIVATE 
		Engine.cpp
		Images.cpp
		Init.cpp
		Types.cpp
)

target_link_libraries(caldera PRIVATE
	efsw::efsw
	quill_static
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
	vk-bootstrap::vk-bootstrap
	VulkanHppModule
	VulkanMemoryAllocatorHppModule
)

add_executable(engine main.cpp)
target_link_libraries(engine
	PRIVATE
		caldera
		quill_static
)
