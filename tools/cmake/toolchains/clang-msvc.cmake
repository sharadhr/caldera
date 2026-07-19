set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
	set(CMAKE_CROSSCOMPILING OFF CACHE BOOL "")
	set(EXECUTABLE_SUFFIX ".exe")
endif ()

set(COMPILER_ARGS
	"-g"
	"-ferror-limit=0"
)

set(CMAKE_C_COMPILER
	"C:/Program Files/LLVM/bin/clang${EXECUTABLE_SUFFIX}"
	${COMPILER_ARGS}
)
set(CMAKE_CXX_COMPILER
	"C:/Program Files/LLVM/bin/clang++${EXECUTABLE_SUFFIX}"
	${COMPILER_ARGS}
)
set(CMAKE_ASM_COMPILER
	"C:/Program Files/LLVM/bin/clang${EXECUTABLE_SUFFIX}"
	${COMPILER_ARGS}
)
set(CMAKE_RC_COMPILER
	"C:/Program Files/LLVM/bin/llvm-rc${EXECUTABLE_SUFFIX}"
)

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug,RTC>:Debug>DLL")
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
