project(ki)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(RAPIDXML_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party)
add_library(RapidXML INTERFACE)
target_include_directories(RapidXML INTERFACE ${RAPIDXML_INCLUDE_DIR})
set(JSON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party)
add_library(JSON INTERFACE)
target_include_directories(JSON INTERFACE ${JSON_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

add_library(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		LINKER_LANGUAGE CXX
		CXX_STANDARD 11
)
target_include_directories(${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} RapidXML JSON ${ZLIB_LIBRARIES})

add_subdirectory("src/dml")
add_subdirectory("src/pclass")
add_subdirectory("src/protocol")
add_subdirectory("src/serialization")
add_subdirectory("src/util")

find_package(Doxygen)
option(KI_BUILD_DOCUMENTATION "Determines whether to build the HTML documentation. (via Doxygen)" ${DOXYGEN_FOUND})
if (KI_BUILD_DOCUMENTATION)
	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen is needed to build the documentation.")
	endif()

    set(DOXYFILE_IN ${PROJECT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

	add_custom_target(
		doc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
	)
endif()

option(COVERALLS "Generate coveralls data" OFF)
if (COVERALLS)
    include(Coveralls)
	coveralls_turn_on_coverage()

	get_target_property(COVERAGE_SRCS ${PROJECT_NAME} SOURCES)
    coveralls_setup("${COVERAGE_SRCS}" ON)
endif()

option(KI_BUILD_EXAMPLES "Determines whether to build examples." ON)
if (KI_BUILD_EXAMPLES)
	add_subdirectory("examples")
endif()

option(KI_BUILD_TESTS "Determines whether to build tests." ON)
if (KI_BUILD_TESTS)
	enable_testing()
	add_subdirectory("test")
endif()
