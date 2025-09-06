cmake_minimum_required(VERSION 4.0)

project(caldera LANGUAGES CXX)
include(libraries)

add_library(caldera)
target_sources(caldera
	PUBLIC FILE_SET CXX_MODULES
		BASE_DIRS source
		FILES
			source/Caldera.cppm
			source/Engine.cppm
			source/Images.cppm
			source/Initialisers.cppm
			source/Types.cppm
)
target_link_libraries(caldera PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
	spdlog::spdlog
	vk-bootstrap::vk-bootstrap
	VulkanHppModule
	VulkanMemoryAllocatorHppModule
)

add_executable(engine source/main.cpp)
target_link_libraries(engine PRIVATE caldera)
