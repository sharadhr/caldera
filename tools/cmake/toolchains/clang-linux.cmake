set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	set(CMAKE_CROSSCOMPILING OFF CACHE BOOL "")
endif ()

set(COMPILER_ARGS
	"-g"
	"-stdlib=libc++"
	"-ferror-limit=0"
)

set(CMAKE_C_COMPILER
	"clang"
	${COMPILER_ARGS}
)
set(CMAKE_CXX_COMPILER
	"clang++"
	${COMPILER_ARGS}
)
set(CMAKE_ASM_COMPILER
	"clang"
	${COMPILER_ARGS}
)
set(CMAKE_RC_COMPILER
	"llvm-rc"
)

set(CMAKE_LINKER_TYPE LLD)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

string(JOIN " " LINKER_FLAGS
	"-stdlib=libc++"
	"-static-libstdc++"
	"--start-no-unused-arguments"
	"--rtlib=compiler-rt"
	"--unwindlib=libunwind"
	"-static-libgcc"
	"--end-no-unused-arguments"
)
string(JOIN " " ASAN_FLAGS
	"-fsanitize=address,undefined"
	"-fno-omit-frame-pointer"
)
set(TSAN_FLAGS "-fsanitize=thread,undefined")

# different possible flag types
set(FLAG_TYPES "C" "CXX" "EXE_LINKER" "SHARED_LINKER" "MODULE_LINKER")

# Add a new build type: LTO, equal to Release but with LTO
if ($ENV{BUILD_WITH_LTO})
	foreach (FLAG_TYPE ${FLAG_TYPES})
		set(CMAKE_${FLAG_TYPE}_FLAGS_LTO "-O3" CACHE STRING "" FORCE)
	endforeach ()

	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
	set(CMAKE_MAP_IMPORTED_CONFIG_LTO "Release" "RelWithDebInfo" "MinSizeRel")
endif ()

# Add `ASan` and `TSan` build types
foreach (CONFIG "ASAN" "TSAN")
	foreach (FLAG_TYPE ${FLAG_TYPES})
		set(CMAKE_${FLAG_TYPE}_FLAGS_${CONFIG} "${${CONFIG}_FLAGS}" CACHE STRING "" FORCE)
	endforeach ()
	set(CMAKE_MAP_IMPORTED_CONFIG_${CONFIG} "Release" "RelWithDebInfo" "MinSizeRel" "")
endforeach ()

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST")
set(CMAKE_EXE_LINKER_FLAGS_INIT ${LINKER_FLAGS})
set(CMAKE_SHARED_LINKER_FLAGS_INIT ${LINKER_FLAGS})
