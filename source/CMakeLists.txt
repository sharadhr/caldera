add_subdirectory(quill_static)
add_library(caldera)
target_sources(caldera
	PUBLIC FILE_SET public_cxx_modules TYPE CXX_MODULES
	FILES
		Caldera.cpp
		Util.cpp
		Types.cpp
	# PRIVATE FILE_SET private_cxx_modules TYPE CXX_MODULES
	PRIVATE 
		Engine.cpp
		Images.cpp
		Init.cpp
)

target_link_libraries(caldera PRIVATE
	efsw::efsw
	QuillStatic
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
		QuillStatic
)
