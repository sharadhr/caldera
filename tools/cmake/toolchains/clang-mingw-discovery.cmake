include_guard(GLOBAL)

# MinGW compiler discovery helpers used by clang-mingw.cmake.
# Lookup order:
# 1) PATH (implicit via find_program)
# 2) LLVM_MINGW_ROOT/bin
# 3) MSYS2_ROOT/clang64/bin and MSYS2_ROOT/mingw64/bin
# 4) WinGet llvm-mingw package bins under LOCALAPPDATA (newest first)

# Collects additional hint directories for MinGW compiler lookup.
# Args:
#   OUT_HINTS: output variable name to receive the hints list.
function (cxxinit_collect_mingw_hints OUT_HINTS)
	set(_CXXINIT_MINGW_HINTS "")
	if (DEFINED ENV{LLVM_MINGW_ROOT})
		list(APPEND _CXXINIT_MINGW_HINTS "$ENV{LLVM_MINGW_ROOT}/bin")
	endif ()
	if (DEFINED ENV{MSYS2_ROOT})
		list(APPEND _CXXINIT_MINGW_HINTS
			"$ENV{MSYS2_ROOT}/clang64/bin"
			"$ENV{MSYS2_ROOT}/mingw64/bin"
		)
	endif ()
	if (WIN32 AND DEFINED ENV{LOCALAPPDATA} AND NOT "$ENV{LOCALAPPDATA}" STREQUAL "")
		file(GLOB _CXXINIT_WINGET_MINGW_HINTS LIST_DIRECTORIES true
			"$ENV{LOCALAPPDATA}/Microsoft/WinGet/Packages/MartinStorsjo.LLVM-MinGW.UCRT_*/llvm-mingw-*-ucrt-*/bin"
		)
		if (_CXXINIT_WINGET_MINGW_HINTS)
			list(SORT _CXXINIT_WINGET_MINGW_HINTS ORDER DESCENDING)
			list(APPEND _CXXINIT_MINGW_HINTS ${_CXXINIT_WINGET_MINGW_HINTS})
		endif ()
	endif ()
	list(REMOVE_DUPLICATES _CXXINIT_MINGW_HINTS)
	list(FILTER _CXXINIT_MINGW_HINTS EXCLUDE REGEX "^$")
	set(${OUT_HINTS} "${_CXXINIT_MINGW_HINTS}" PARENT_SCOPE)
endfunction ()

# Resolves the prefixed MinGW toolchain (`target`-clang).
# Args:
#   TARGET: compiler target prefix (for example: x86_64-w64-mingw32).
#   HINTS_VAR: variable name containing additional hint directories.
#   OUT_CLANG: output variable name for the resolved clang path.
function (cxxinit_require_mingw_clang TARGET HINTS_VAR OUT_CLANG)
	set(_CXXINIT_HINTS ${${HINTS_VAR}})
	find_program(_CXXINIT_MINGW_CLANG
		NAMES "${TARGET}-clang${CMAKE_HOST_EXECUTABLE_SUFFIX}"
		HINTS ${_CXXINIT_HINTS}
	)
	if (NOT _CXXINIT_MINGW_CLANG)
		list(APPEND _CXXINIT_HINTS_TEXT ${_CXXINIT_HINTS})
		message(FATAL_ERROR
			"Unable to locate MinGW Clang toolchain. Ensure one of these is set up before configuring:\n"
			"  - PATH contains ${TARGET}-clang${CMAKE_HOST_EXECUTABLE_SUFFIX}\n"
			"  - LLVM_MINGW_ROOT points to an llvm-mingw installation\n"
			"  - MSYS2_ROOT points to an MSYS2 installation\n"
			"Checked additional hint directories: ${_CXXINIT_HINTS_TEXT}"
		)
	endif ()
	set(${OUT_CLANG} "${_CXXINIT_MINGW_CLANG}" PARENT_SCOPE)
endfunction ()

# Resolves llvm-rc near the selected compiler, with a name fallback.
# Args:
#   COMPILER_DIR: directory where clang/clang++ was found.
#   HINTS_VAR: variable name containing additional hint directories.
#   OUT_RC: output variable name for the resource compiler.
function (cxxinit_resolve_mingw_rc COMPILER_DIR HINTS_VAR OUT_RC)
	set(_CXXINIT_HINTS ${${HINTS_VAR}})
	find_program(_CXXINIT_MINGW_LLVM_RC
		NAMES "llvm-rc${CMAKE_HOST_EXECUTABLE_SUFFIX}"
		HINTS "${COMPILER_DIR}" ${_CXXINIT_HINTS}
	)
	if (NOT _CXXINIT_MINGW_LLVM_RC)
		set(${OUT_RC} "llvm-rc${CMAKE_HOST_EXECUTABLE_SUFFIX}" PARENT_SCOPE)
		return ()
	endif ()
	set(${OUT_RC} "${_CXXINIT_MINGW_LLVM_RC}" PARENT_SCOPE)
endfunction ()

# Main entrypoint used by clang-mingw.cmake.
# Args:
#   TARGET: compiler target prefix.
#   OUT_CLANG: target-clang path.
#   OUT_CLANGXX: target-clang++ path.
#   OUT_RC: llvm-rc path.
function (cxxinit_resolve_mingw_toolchain TARGET OUT_CLANG OUT_CLANGXX OUT_RC)
	cxxinit_collect_mingw_hints(_CXXINIT_MINGW_HINTS)
	cxxinit_require_mingw_clang("${TARGET}" _CXXINIT_MINGW_HINTS _CXXINIT_MINGW_CLANG)

	get_filename_component(_CXXINIT_MINGW_COMPILER_DIR "${_CXXINIT_MINGW_CLANG}" DIRECTORY)
	set(_CXXINIT_MINGW_CLANGXX "${_CXXINIT_MINGW_COMPILER_DIR}/${TARGET}-clang++${CMAKE_HOST_EXECUTABLE_SUFFIX}")
	if (NOT EXISTS "${_CXXINIT_MINGW_CLANGXX}")
		message(FATAL_ERROR
			"Detected '${TARGET}-clang' at '${_CXXINIT_MINGW_CLANG}', but matching '${TARGET}-clang++' is missing at '${_CXXINIT_MINGW_CLANGXX}'."
		)
	endif ()

	cxxinit_resolve_mingw_rc("${_CXXINIT_MINGW_COMPILER_DIR}" _CXXINIT_MINGW_HINTS _CXXINIT_MINGW_LLVM_RC)

	set(${OUT_CLANG} "${_CXXINIT_MINGW_CLANG}" PARENT_SCOPE)
	set(${OUT_CLANGXX} "${_CXXINIT_MINGW_CLANGXX}" PARENT_SCOPE)
	set(${OUT_RC} "${_CXXINIT_MINGW_LLVM_RC}" PARENT_SCOPE)
endfunction ()
