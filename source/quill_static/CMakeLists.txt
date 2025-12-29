add_library(QuillStatic)
target_sources(QuillStatic
	PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES "QuillStatic.cpp"
)

# Link quill dependency
target_link_libraries(QuillStatic PUBLIC quill::quill)
