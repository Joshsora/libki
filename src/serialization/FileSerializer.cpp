#include "ki/serialization/FileSerializer.h"
#include <fstream>
#include "ki/serialization/JsonSerializer.h"
#include "ki/serialization/XmlSerializer.h"
#include "rapidxml_utils.hpp"

namespace ki
{
namespace serialization
{
	FileSerializer::FileSerializer(pclass::TypeSystem &type_system)
	{
		m_type_system = &type_system;
	}

	void FileSerializer::save_binary(pclass::PropertyClass *object,
		BinarySerializer::flags flags, const std::string &filepath) const
	{
		// Force the WRITE_SERIALIZER_FLAGS flag so that the correct flags
		// can be loaded later
		flags |= BinarySerializer::flags::WRITE_SERIALIZER_FLAGS;

		// Create the buffer, stream, and serializer, and save the object
		BitBuffer buffer;
		BitStream stream(buffer);
		BinarySerializer serializer(*m_type_system, true, flags);
		serializer.save(object, stream);

		// Write to file
		std::ofstream ofs(filepath, std::ios::binary);
		ofs << BINARY_HEADER;
		ofs.write(
			reinterpret_cast<char *>(buffer.data()),
			stream.tell().as_bytes()
		);
	}

	void FileSerializer::save_xml(
		pclass::PropertyClass *object, const std::string &filepath) const
	{
		// Serialize the object into an XML string
		XmlSerializer serializer(*m_type_system);
		const auto xml_string = serializer.save(object);

		// Write the XML to a file
		std::ofstream ofs(filepath, std::ios::binary);
		if (!ofs.is_open())
		{
			std::ostringstream oss;
			oss << "Failed to open filepath for writing: " << filepath;
			throw runtime_error(oss.str());
		}
		ofs << R"(<?xml version="1.0" encoding="UTF-8"?>)" << std::endl
			<< xml_string;
	}

	void FileSerializer::save_json(
		pclass::PropertyClass *object, const std::string &filepath) const
	{
		// Serialize the object into a JSON string
		JsonSerializer serializer(*m_type_system, true);
		const auto json_string = serializer.save(object);
		std::ofstream ofs(filepath, std::ios::binary);
		if (!ofs.is_open())
		{
			std::ostringstream oss;
			oss << "Failed to open filepath for writing: " << filepath;
			throw runtime_error(oss.str());
		}
		ofs << JSON_HEADER << json_string;
	}

	void FileSerializer::load(
		std::unique_ptr<pclass::PropertyClass> &dest, const std::string &filepath) const
	{
		// Open the specified file for reading
		std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
		if (!ifs.is_open())
		{
			std::ostringstream oss;
			oss << "Failed to open filepath for reading: " << filepath;
			throw runtime_error(oss.str());
		}

		// Make sure there is at least enough data to determine which
		// serializer was used
		const size_t file_size = ifs.tellg();
		ifs.seekg(std::ios::beg);
		if (file_size < 4)
		{
			std::ostringstream oss;
			oss << "Not enough data to determine serializer used in file: "
				<< filepath;
			throw runtime_error(oss.str());
		}

		// Load the contents of the file into a buffer
		std::vector<char> file_data(file_size);
		ifs.read(file_data.data(), file_size);
		ifs.close();

		// Use the first 4 bytes to distinguish which serializer was used
		if (strncmp(file_data.data(), BINARY_HEADER, 4) == 0)
		{
			// Create and populate a BitStream with the file's data
			BitBuffer buffer(file_size - 4);
			buffer.write_copy(
				reinterpret_cast<uint8_t *>(&file_data[4]),
				BitBuffer::buffer_pos(0, 0),
				(file_size - 4) * 8
			);
			BitStream stream(buffer);

			// Deserialize the contents of the stream
			BinarySerializer serializer(*m_type_system, true,
				BinarySerializer::flags::WRITE_SERIALIZER_FLAGS);
			serializer.load(dest, stream, file_size - 4);
		}
		else if (strncmp(file_data.data(), JSON_HEADER, 4) == 0)
		{
			auto json_string = std::string(&file_data[4], file_size - 4);
			JsonSerializer serializer(*m_type_system, true);
			serializer.load(dest, json_string);
		}
		else
		{
			rapidxml::file<char> file(filepath.c_str());
			XmlSerializer serializer(*m_type_system);
			serializer.load(dest, file.data());
		}
	}
}
}
