if (NOT DEFINED CXXINIT_CLANG_WINDOWS_COMMON_INCLUDED)
set(CXXINIT_CLANG_WINDOWS_COMMON_INCLUDED ON)

set(CXXINIT_CLANG_WARNING_FLAGS "")
if (NOT DEFINED _VCPKG_ROOT_DIR)
	string(JOIN " " CXXINIT_CLANG_WARNING_FLAGS
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

string(JOIN " " CXXINIT_CLANG_ASAN_FLAGS
	"-O3"
	"-fsanitize=address,undefined"
	"-fno-omit-frame-pointer"
)
string(JOIN " " CXXINIT_CLANG_TSAN_FLAGS
	"-O3"
	"-fsanitize=thread,undefined"
)

set(FLAG_TYPES "C" "CXX")
foreach (CONFIG "ASAN" "TSAN")
	set(_SANITIZER_FLAGS "${CXXINIT_CLANG_${CONFIG}_FLAGS}")
	foreach (FLAG_TYPE ${FLAG_TYPES})
		set(CMAKE_${FLAG_TYPE}_FLAGS_${CONFIG} "${_SANITIZER_FLAGS}" CACHE STRING "" FORCE)
	endforeach ()
	set(CMAKE_MAP_IMPORTED_CONFIG_${CONFIG} "Release" "RelWithDebInfo" "MinSizeRel" "")
endforeach ()

set(CMAKE_C_FLAGS_INIT "${CXXINIT_CLANG_WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${CXXINIT_CLANG_WARNING_FLAGS}")

unset(_SANITIZER_FLAGS)

endif ()
