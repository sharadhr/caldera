set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
	set(CMAKE_CROSSCOMPILING OFF CACHE BOOL "")
endif ()

set(TARGET "x86_64-w64-mingw32")
set(CMAKE_C_COMPILER_TARGET ${TARGET})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET})

set(COMPILER_ARGS
	"--start-no-unused-arguments"
	"-gcodeview"
	"--end-no-unused-arguments"
	"-ferror-limit=0"
	"-fms-extensions"
	"-fms-hotpatch"
)

set(CMAKE_C_COMPILER
	"${TARGET}-clang${CMAKE_HOST_EXECUTABLE_SUFFIX}"
	${COMPILER_ARGS}
)
set(CMAKE_CXX_COMPILER
	"${TARGET}-clang++${CMAKE_HOST_EXECUTABLE_SUFFIX}"
	${COMPILER_ARGS}
)
set(CMAKE_ASM_COMPILER
	"${TARGET}-clang${CMAKE_HOST_EXECUTABLE_SUFFIX}"
	${COMPILER_ARGS}
)
set(CMAKE_RC_COMPILER
	"llvm-rc${CMAKE_HOST_EXECUTABLE_SUFFIX}"
)

set(CMAKE_LINKER_TYPE LLD)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Some useful flags
if (NOT DEFINED _VCPKG_ROOT_DIR)
	string(JOIN " " WARNING_FLAGS
		"-Wall"
		"-Walloca"
		"-Wcast-align"
		"-Wcast-qual"
		"-Wconversion"
		"-Wctor-dtor-privacy"
		"-Wdeprecated-copy-dtor"
		"-Wdouble-promotion"
		"-Wenum-conversion"
		"-Wextra"
		"-Wextra-semi"
		"-Wfloat-equal"
		"-Wformat-overflow"
		"-Wformat=2"
		"-Wframe-larger-than=1048576"
		"-Wimplicit-fallthrough"
		"-Wmismatched-tags"
		"-Wmissing-braces"
		"-Wmultichar"
		"-Wno-unused-parameter"
		"-Wnon-virtual-dtor"
		"-Wnull-dereference"
		"-Wold-style-cast"
		"-Woverloaded-virtual"
		"-Wpedantic"
		"-Wpointer-arith"
		"-Wrange-loop-construct"
		"-Wshadow"
		"-Wsign-conversion"
		"-Wundef"
		"-Wuninitialized"
		"-Wunused"
		"-Wvla"
		"-Wwrite-strings"
	)
endif ()
string(JOIN " " ASAN_FLAGS
	"-O3"
	"-fsanitize=address,undefined"
	"-fno-omit-frame-pointer"
)
string(JOIN " " TSAN_FLAGS
	"-O3"
	"-fsanitize=thread,undefined"
)

string(JOIN " " LTO_FLAGS
	"-march=native"
	"-flto=thin"
)

set(FLAG_TYPES "C" "CXX")
foreach (CONFIG "ASAN" "TSAN")
	foreach (FLAG_TYPE ${FLAG_TYPES})
		set(CMAKE_${FLAG_TYPE}_FLAGS_${CONFIG} "${${CONFIG}_FLAGS}" CACHE STRING "" FORCE)
	endforeach ()
	set(CMAKE_MAP_IMPORTED_CONFIG_${CONFIG} "Release" "RelWithDebInfo" "MinSizeRel" "")
endforeach ()

# Set the default flags for normal build types
set(CMAKE_C_FLAGS_INIT ${WARNING_FLAGS})
set(CMAKE_CXX_FLAGS_INIT ${WARNING_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS_INIT ${LINKER_FLAGS})
set(CMAKE_SHARED_LINKER_FLAGS_INIT ${LINKER_FLAGS})

# For release, which is LTOed
set(CMAKE_C_FLAGS_RELEASE_INIT ${LTO_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE_INIT ${LTO_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "-flto=thin")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT "-flto=thin")
