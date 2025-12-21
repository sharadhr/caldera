add_library(quill_static)
target_sources(quill_static
    PUBLIC 
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES "QuillStatic.cpp"
)

# Link quill dependency
target_link_libraries(quill_static PUBLIC quill::quill)