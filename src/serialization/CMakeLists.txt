target_sources(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/serialization/BinarySerializer.cpp
		${PROJECT_SOURCE_DIR}/src/serialization/FileSerializer.cpp
		${PROJECT_SOURCE_DIR}/src/serialization/JsonSerializer.cpp
		${PROJECT_SOURCE_DIR}/src/serialization/XmlSerializer.cpp
)