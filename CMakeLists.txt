project(ki)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(RAPIDXML_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party)
add_library(RapidXML INTERFACE)
target_include_directories(RapidXML INTERFACE ${RAPIDXML_INCLUDE_DIR})

add_library(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		LINKER_LANGUAGE CXX
		CXX_STANDARD 11
)
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
	INTERFACE
		${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} RapidXML)

add_subdirectory("src/dml")
add_subdirectory("src/protocol")

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
