#include "ki/serialization/BinarySerializer.h"
#include <zlib.h>
#include <cassert>
#include "ki/util/unique.h"
#include "ki/util/trace.h"

namespace ki
{
namespace serialization
{
	BinarySerializer::BinarySerializer(const pclass::TypeSystem &type_system,
		const bool is_file, const flags flags)
	{
		m_type_system = &type_system;
		m_is_file = is_file;
		m_flags = flags;
		m_root_object = nullptr;
	}

	void BinarySerializer::save(const pclass::PropertyClass *object, BitStream &stream)
	{
		// Write the serializer flags
		if (FLAG_IS_SET(m_flags, flags::WRITE_SERIALIZER_FLAGS))
			stream.write<uint32_t>(static_cast<uint32_t>(m_flags));

		// If the contents of the stream are going to be compressed,
		// reserve space to put the length and compression toggle.
		const auto compression_header_pos = stream.tell();
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
		{
			if (m_is_file)
				stream.write<bool>(false);
			stream.write<uint32_t>(0);
		}

		// Write the object to the stream
		const auto start_pos = stream.tell();
		m_root_object = object;
		save_object(object, stream);

		// Compress the contents of the stream
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
		{
			// Remember where the ending of the data is
			const auto end_pos = stream.tell();
			const auto size_bits = (end_pos - start_pos).as_bits();
			const auto size_bytes = (end_pos - start_pos).as_bytes();

			// Make a copy of the uncompressed data
			auto *uncompressed = new uint8_t[size_bytes] {0};
			stream.seek(start_pos);
			stream.read_copy(uncompressed, size_bits);

			// Setup compression
			static const std::size_t bufsize = 1024;
			uint8_t temp_buffer[bufsize] {0};
			std::vector<uint8_t> compressed;
			z_stream z;
			z.zalloc = nullptr;
			z.zfree = nullptr;
			z.next_in = uncompressed;
			z.avail_in = size_bytes;
			z.next_out = temp_buffer;
			z.avail_out = bufsize;

			// Compress the uncompressed data
			deflateInit(&z, Z_DEFAULT_COMPRESSION);
			while (z.avail_in != 0)
			{
				const auto result = deflate(&z, Z_NO_FLUSH);
				assert(result == Z_OK);

				// Have we filled up the temporary buffer?
				if (z.avail_out == 0)
				{
					// Copy all data from the temporary buffer into the
					// compressed vector
					compressed.insert(compressed.end(),
						temp_buffer, temp_buffer + bufsize);
					z.next_out = temp_buffer;
					z.avail_out = bufsize;
				}
			}
			auto deflate_result = Z_OK;
			while (deflate_result == Z_OK)
			{
				if (z.avail_out == 0)
				{
					compressed.insert(compressed.end(),
						temp_buffer, temp_buffer + bufsize);
					z.next_out = temp_buffer;
					z.avail_out = bufsize;
				}
				deflate_result = deflate(&z, Z_FINISH);
			}
			assert(deflate_result == Z_STREAM_END);
			compressed.insert(compressed.end(),
				temp_buffer, temp_buffer + bufsize - z.avail_out);
			deflateEnd(&z);

			// Cleanup temporary buffers
			delete[] uncompressed;

			// Write the compression header
			const auto use_compression = compressed.size() < size_bytes;
			stream.seek(compression_header_pos);
			if (m_is_file)
				stream.write<bool>(use_compression);
			stream.write<uint32_t>(size_bytes);
			
			// Write the compressed data
			if (use_compression)
				stream.write_copy(compressed.data(), compressed.size() * 8);
			else
				// Go back to the end of the original data
				stream.seek(end_pos);
		}
	}

	bool BinarySerializer::presave_object(const pclass::PropertyClass *object, BitStream &stream) const
	{
		// If we have an object, write the type hash, otherwise, write NULL (0).
		if (object)
			stream.write<uint32_t>(object->get_type().get_hash());
		else
			stream.write<uint32_t>(0);
		return object != nullptr;
	}

	void BinarySerializer::save_object(const pclass::PropertyClass *object, BitStream &stream) const
	{
		// Write any object headers
		if (!presave_object(object, stream))
			return;

		// Remember where we started writing the object data
		const auto start_pos = stream.tell();

		// If we're using the file format, reserve space to put the size of this object
		if (m_is_file)
			stream.write<uint32_t>(0);

		// Write each of the object's properties
		auto &properties = object->get_properties();
		for (auto it = properties.begin();
			it != properties.end(); ++it)
		{
			save_property(*it, stream);
		}
		
		// If we're using the file format, we need to write how big the object is
		if (m_is_file)
		{
			// Remember where the ending of the data is
			const auto end_pos = stream.tell();
			const auto size_bits = (end_pos - start_pos).as_bits();

			// Write the size, and then move back to the end
			stream.seek(start_pos);
			stream.write(size_bits);
			stream.seek(end_pos);
		}
	}

	void BinarySerializer::save_property(const pclass::IProperty &prop, BitStream &stream) const
	{
		// Ignore non-public properties if the WRITE_PUBLIC_ONLY flag is set
		if (FLAG_IS_SET(m_flags, flags::WRITE_PUBLIC_ONLY) &&
			!FLAG_IS_SET(prop.get_flags(), pclass::IProperty::flags::PUBLIC))
			return;

		// Realign the stream if we're going to write a prefix for this property
		if (prop.is_dynamic() || m_is_file)
			stream.realign();

		// Remember where we started writing the property data
		const auto start_pos = stream.tell();

		// Do we need to write the property header?
		if (m_is_file)
		{
			// Reserve space to put the size of the property
			stream.write<uint32_t>(0);
			stream.write<uint32_t>(prop.get_full_hash());
		}

		// If the property is dynamic, write the element count
		if (prop.is_dynamic())
		{
			if (m_is_file)
			{
				// TODO: Determine how the size of the element count is chosen
				// while in file mode
				stream.write<uint8_t>(prop.get_element_count() * 2);
			}
			else
			{
				// Write the element count as an unsigned int
				stream.write<uint32_t>(prop.get_element_count());
			}
		}

		for (auto i = 0; i < prop.get_element_count(); ++i)
		{
			// Realign the stream if necessary
			if (prop.get_type().is_byte_aligned())
				stream.realign();

			if (prop.is_pointer()
				&& prop.get_type().get_kind() == pclass::Type::Kind::CLASS)
			{
				// Write the value as a nested object
				save_object(prop.get_object(i), stream);
			}
			else
				prop.write_value_to(stream, m_is_file, i);
		}

		// Finish writing the property header by writing the length
		if (m_is_file)
		{
			// Remember where the ending of the data is
			const auto end_pos = stream.tell();
			const auto size_bits = (end_pos - start_pos).as_bits();

			// Write the size, and then move back to the end
			stream.seek(start_pos);
			stream.write(size_bits);
			stream.seek(end_pos);
		}
	}

	void BinarySerializer::load(
		std::unique_ptr<pclass::PropertyClass> &dest,
		BitStream &stream, const std::size_t size)
	{
		// Create a new stream that reads a segment of the stream given to us
		auto segment_buffer = stream.buffer().segment(stream.tell(), size * 8);
		auto buffer = std::unique_ptr<IBitBuffer>(
			dynamic_cast<IBitBuffer *>(segment_buffer.release())
		);
		auto segment_stream = BitStream(*buffer);
		stream.seek(stream.tell() + size * 8, false);

		// Read the serializer flags
		if (FLAG_IS_SET(m_flags, flags::WRITE_SERIALIZER_FLAGS))
			m_flags = static_cast<flags>(segment_stream.read<uint32_t>());

		// Decompress the contents of the stream
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
		{
			// Read the compression header
			bool use_compression = true;
			if (m_is_file)
				use_compression = segment_stream.read<bool>();
			const auto uncompressed_size = segment_stream.read<uint32_t>();

			// Work out how much data is available after the compression header
			const auto start_pos = segment_stream.tell();
			const auto end_pos = BitStream::stream_pos(buffer->size(), 0);
			const auto data_available_bytes = (end_pos - start_pos).as_bytes();

			// Has compression been used on this object?
			if (use_compression)
			{
				// Create a buffer for the compressed data and read it in
				BitBuffer compressed(data_available_bytes);
				segment_stream.read_copy(compressed.data(), data_available_bytes * 8);

				// Uncompress the compressed buffer
				auto uncompressed = ki::make_unique<BitBuffer>(uncompressed_size);
				uLong dest_len = uncompressed_size;
				uncompress(uncompressed->data(), &dest_len,
					compressed.data(), data_available_bytes);

				// Delete the old buffer and use the new uncompressed buffer
				buffer = std::unique_ptr<IBitBuffer>(
					dynamic_cast<IBitBuffer *>(uncompressed.release())
				);
			}
			else
			{
				// Use a segment of the current buffer as the new buffer
				segment_buffer = buffer->segment(segment_stream.tell(), data_available_bytes);
				buffer = std::unique_ptr<IBitBuffer>(
					dynamic_cast<IBitBuffer *>(segment_buffer.release())
				);
			}

			// Create a new stream to read from the new buffer
			segment_stream = BitStream(*buffer);
		}

		// Load the root object
		load_object(dest, segment_stream);
	}

	void BinarySerializer::preload_object(
		std::unique_ptr<pclass::PropertyClass> &dest, BitStream &stream) const
	{
		const auto type_hash = stream.read<pclass::hash_t>();
		TRACE << "BinarySerializer::preload_object: type_hash = "
			  << type_hash << std::endl;

		if (type_hash != 0)
		{
			// Instantiate the type
			const auto &type = m_type_system->get_type(type_hash);
			dest = type.instantiate();
		}
		else
			// Instantiate to a nullptr
			dest = nullptr;
	}

	void BinarySerializer::load_object(
		std::unique_ptr<pclass::PropertyClass> &dest, BitStream &stream) const
	{
		// Read the object header
		preload_object(dest, stream);

		// Did we get an object or null?
		if (!dest)
			return;

		TRACE << "BinarySerializer::load_object: dest->get_type().get_name() = "
			  << dest->get_type().get_name() << std::endl;

		auto &properties = dest->get_properties();

		if (m_is_file)
		{
			// Read the object's size, and create a new BitBufferSegment to
			// ensure that data is only read from inside this region.
			const auto object_size =
				stream.read<uint32_t>() - bitsizeof<uint32_t>::value;
			TRACE << "BinarySerializer::load_object: object_size = "
				  << object_size << std::endl;
			auto object_buffer = stream.buffer().segment(
				stream.tell(), object_size
			);
			auto object_stream = BitStream(*object_buffer);

			// Instead of loading properties sequentially, the file format specifies
			// the hash of a property before writing its value, so we just need to
			// iterate over how ever many bits were specified as the object size.
			while (object_stream.tell().as_bits() < object_size)
			{
				// Re-align the object stream so that our position lies on a byte
				object_stream.seek(BitStream::stream_pos(object_stream.tell().as_bytes(), 0));

				// Read the property's size, and create a new BitBufferSegment to
				// ensure that data is only read from inside this region.
				const auto property_size =
					object_stream.read<uint32_t>() - bitsizeof<uint32_t>::value;
				TRACE << "BinarySerializer::load_object: property_size = "
					  << property_size << std::endl;
				const auto property_buffer = object_buffer->segment(
					object_stream.tell(), property_size
				);
				auto property_stream = BitStream(*property_buffer);

				// Get the property to load based on it's hash, and then load
				// it's value.
				const auto property_hash = property_stream.read<uint32_t>();
				TRACE << "BinarySerializer::load_object: property_hash = "
					  << property_hash << std::endl;
				auto &prop = properties.get_property(property_hash);
				load_property(prop, property_stream);

				// Seek out the end of the property in the object stream
				auto byte_pos = object_stream.tell().as_bytes() + property_stream.tell().as_bytes();
				object_stream.seek(BitStream::stream_pos(byte_pos, 0), false);
			}

			// Seek out the end of the object in the stream
			auto byte_pos = stream.tell().as_bytes() + object_stream.tell().as_bytes();
			stream.seek(BitStream::stream_pos(byte_pos, 0), false);
		}
		else
		{
			// Load properties in the order that the object's class declared them
			for (auto it = properties.begin();
					it != properties.end(); ++it)
				load_property(*it, stream);
		}

		// All properties on this object have been set now, so call the
		// created handler on the object
		dest->on_created();
	}

	void BinarySerializer::load_property(pclass::IProperty &prop, BitStream &stream) const
	{
		// Ignore non-public properties if the WRITE_PUBLIC_ONLY flag is set
		if (FLAG_IS_SET(m_flags, flags::WRITE_PUBLIC_ONLY) &&
			!FLAG_IS_SET(prop.get_flags(), pclass::IProperty::flags::PUBLIC))
			return;

		TRACE << "BinarySerializer::load_property: prop.get_name() = "
			  << prop.get_name() << std::endl;

		// Re-align the stream if we're going to read a prefix for this property
		if (prop.is_dynamic())
			stream.realign();

		// If the property is dynamic, we need to load the element count
		if (prop.is_dynamic())
		{
			std::size_t element_count;
			if (m_is_file)
			{
				// TODO: Determine how the size of the element count is chosen
				// while in file mode
				element_count = stream.read<uint8_t>() / 2;
			}
			else
			{
				// Load the element count as an unsigned int
				element_count = stream.read<uint32_t>();
			}
			prop.set_element_count(element_count);
			TRACE << "BinarySerializer::load_property: element_count = "
				  << element_count << std::endl;
		}

		for (auto i = 0; i < prop.get_element_count(); ++i)
		{
			TRACE << "BinarySerializer::load_property: i = " << i << std::endl;

			// Realign the stream if necessary
			if (prop.get_type().is_byte_aligned())
				stream.realign();

			if (prop.is_pointer() &&
				prop.get_type().get_kind() == pclass::Type::Kind::CLASS)
			{
				// Read the object as a nested object
				std::unique_ptr<pclass::PropertyClass> object = nullptr;
				load_object(object, stream);
				prop.set_object(object, i);
			}
			else
			{
				prop.read_value_from(stream, m_is_file, i);
				TRACE << "BinarySerializer::load_property: prop.get_value(" << i << ") = "
					  << prop.get_value(i).as<std::string>().get<std::string>() << std::endl;
			}
		}
	}
}
}
