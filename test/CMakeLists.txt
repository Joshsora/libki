set(CATCH_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

file(GLOB files "src/unit-*.cpp")
foreach (file ${files})
	get_filename_component(file_basename ${file} NAME_WE)
	string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

	add_executable(${testcase} ${file})
	target_link_libraries(${testcase} Catch ${PROJECT_NAME})
	add_test(${testcase} ${testcase})
endforeach()
