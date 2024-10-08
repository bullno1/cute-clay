cmake_minimum_required(VERSION 3.15)
project(recute)

#include(CheckIPOSupported)
#check_ipo_supported(RESULT IPO_SUPPORTED)
#if (IPO_SUPPORTED)
	#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
	#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
#endif ()

# Fix output dir
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_DEBUG_POSTFIX _d)

# Add deps first

add_subdirectory(deps)

# Then everything else to avoid leaking into deps

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

function (setup_library LIBRARY_NAME IS_STATIC SOURCES)
	if (IS_STATIC)
		add_library(${LIBRARY_NAME} STATIC ${SOURCES})
	else ()
		string(REPLACE "-" "_" LIBRARY_NAME_NO_SLUG ${LIBRARY_NAME})
		string(TOUPPER ${LIBRARY_NAME_NO_SLUG} LIBRARY_NAME_UPPER)
		add_library(${LIBRARY_NAME} SHARED ${SOURCES})
		target_compile_definitions(${LIBRARY_NAME} PUBLIC ${LIBRARY_NAME_UPPER}_SHARED)
		target_compile_definitions(${LIBRARY_NAME} PRIVATE ${LIBRARY_NAME_UPPER}_BUILD)
		set_target_properties(${LIBRARY_NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
	endif ()

	if (NOT MSVC)
		target_link_options(${LIBRARY_NAME} PRIVATE
			-Wl,--exclude-libs,ALL
			-Wl,--no-whole-archive
		)
	endif ()
endfunction ()

if (MSVC)
	# 4200: Flexible array member is a standard feature since C99
	add_compile_options(/W4 /WX /wd4200)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	# MSVC does not define max_align_t
else()
	add_compile_options(
		-Wall -Wextra -pedantic -Werror
		-Wno-unused-parameter
		-Wno-error=c23-extensions
	)
endif()

add_subdirectory(src)
