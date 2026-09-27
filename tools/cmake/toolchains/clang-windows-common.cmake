if (NOT DEFINED CXXINIT_CLANG_WINDOWS_COMMON_INCLUDED)
set(CXXINIT_CLANG_WINDOWS_COMMON_INCLUDED ON)

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

unset(_SANITIZER_FLAGS)

endif ()
