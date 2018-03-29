file(GLOB files "src/example-*.cpp")
foreach (file ${files})
	get_filename_component(file_basename ${file} NAME_WE)
	add_executable(${file_basename} ${file})
	set_target_properties(${file_basename}
		PROPERTIES
			CXX_STANDARD 11
	)
	target_link_libraries(${file_basename} ${PROJECT_NAME})
endforeach()
