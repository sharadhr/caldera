set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
	set(CMAKE_CROSSCOMPILING OFF CACHE BOOL "")
endif ()

set(TARGET "x86_64-w64-mingw32")
set(CMAKE_C_COMPILER_TARGET ${TARGET})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET})

include("${CMAKE_CURRENT_LIST_DIR}/clang-mingw-discovery.cmake")
cxxinit_resolve_mingw_toolchain(
	"${TARGET}"
	CXXINIT_MINGW_CLANG
	CXXINIT_MINGW_CLANGXX
	CXXINIT_MINGW_LLVM_RC
)

set(COMPILER_ARGS
	"--start-no-unused-arguments"
	"-gcodeview"
	"--end-no-unused-arguments"
	"-ferror-limit=0"
	"-fms-extensions"
	"-fms-hotpatch"
)

set(CMAKE_C_COMPILER
	"${CXXINIT_MINGW_CLANG}"
	${COMPILER_ARGS}
)
set(CMAKE_CXX_COMPILER
	"${CXXINIT_MINGW_CLANGXX}"
	${COMPILER_ARGS}
)
set(CMAKE_ASM_COMPILER
	"${CXXINIT_MINGW_CLANG}"
	${COMPILER_ARGS}
)
set(CMAKE_RC_COMPILER "${CXXINIT_MINGW_LLVM_RC}")

set(CMAKE_LINKER_TYPE LLD)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include("${CMAKE_CURRENT_LIST_DIR}/clang-windows-common.cmake")
