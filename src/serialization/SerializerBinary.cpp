#include "ki/serialization/SerializerBinary.h"
#include <zlib.h>
#include <cassert>

namespace ki
{
namespace serialization
{
	SerializerBinary::SerializerBinary(const pclass::TypeSystem &type_system,
		const bool is_file, const flags flags)
	{
		m_type_system = &type_system;
		m_is_file = is_file;
		m_flags = flags;
		m_root_object = nullptr;
	}

	void SerializerBinary::save(const pclass::PropertyClass *object, BitStream &stream)
	{
		// Write the serializer flags
		if (FLAG_IS_SET(m_flags, flags::WRITE_SERIALIZER_FLAGS))
			stream.write<uint32_t>(static_cast<uint32_t>(m_flags));

		// If the contents of the stream are going to be compressed,
		// reserve space to put the length and compression toggle.
		const auto compression_header_pos = stream.tell();
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
		{
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

	void SerializerBinary::presave_object(const pclass::PropertyClass *object, BitStream &stream) const
	{
		// If we have an object, write the type hash, otherwise, write NULL (0).
		if (object)
			stream.write<uint32_t>(object->get_type().get_hash());
		else
			stream.write<uint32_t>(NULL);
	}

	void SerializerBinary::save_object(const pclass::PropertyClass *object, BitStream &stream) const
	{
		// Write any object headers
		presave_object(object, stream);

		// Make sure we have an object to write
		if (!object)
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
		
		// Re-align the stream so that our position lies on a byte
		// TODO: Look into how/when the serialization re-aligns as this may not be the right place
		stream.seek(BitStream::stream_pos(stream.tell().as_bytes(), 0));
	}

	void SerializerBinary::save_property(const pclass::IProperty &prop, BitStream &stream) const
	{
		// Remember where we started writing the property data
		const auto start_pos = stream.tell();

		// Do we need to write the property header?
		if (m_is_file)
		{
			// Reserve space to put the size of the property
			stream.write<uint32_t>(0);
			stream.write<uint32_t>(prop.get_full_hash());
		}

		// Is the property dynamic? (holding more than one value)
		auto &property_type = prop.get_type();
		if (prop.is_dynamic())
		{
			// Cast the property to a IDynamicProperty
			const auto &dynamic_property =
				dynamic_cast<const pclass::IDynamicProperty &>(prop);

			// Write the number of elements
			stream.write<uint32_t>(dynamic_property.get_element_count());

			// Iterate through the elements
			for (auto i = 0; i < dynamic_property.get_element_count(); i++)
			{
				// Is this a collection of pointers?
				if (prop.is_pointer())
				{
					// Is the property a collection of pointers to other objects?
					if (property_type.get_kind() == pclass::Type::kind::CLASS)
						// Write the value as a nested object
						save_object(dynamic_property.get_object(i), stream);
					else
						// Write the value as normal (let the property deal with dereferencing)
						dynamic_property.write_value_to(stream, i);
				}
				else
					// If the value isn't a pointer, and it's not dynamic, just write it as a value
					dynamic_property.write_value_to(stream, i);
			}
		}
		else if (prop.is_pointer())
		{
			// Does this property hold a pointer to another object?
			if (property_type.get_kind() == pclass::Type::kind::CLASS)
				// Write the value as a nested object
				save_object(prop.get_object(), stream);
			else
				// Write the value as normal (let the property deal with dereferencing)
				prop.write_value_to(stream);
		}
		else
			// If the value isn't a pointer, and it's not dynamic, just write it as a value
			prop.write_value_to(stream);

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

	void SerializerBinary::load(pclass::PropertyClass *&dest,
		BitStream &stream, const std::size_t size)
	{
		// Create a new stream that reads a segment of the stream given to us
		IBitBuffer *buffer = stream.buffer().segment(stream.tell(), size * 8);
		auto segment_stream = BitStream(*buffer);
		stream.seek(stream.tell() + size * 8, false);

		// Read the serializer flags
		if (FLAG_IS_SET(m_flags, flags::WRITE_SERIALIZER_FLAGS))
			m_flags = static_cast<flags>(segment_stream.read<uint32_t>());

		// Decompress the contents of the stream
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
		{
			// Read the compression header
			const auto use_compression = segment_stream.read<bool>();
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
				auto *uncompressed = new BitBuffer(uncompressed_size);
				uLong dest_len = uncompressed_size;
				uncompress(uncompressed->data(), &dest_len,
					compressed.data(), data_available_bytes);

				// Delete the old buffer and use the new uncompressed buffer
				delete buffer;
				buffer = uncompressed;
			}
			else
			{
				// Use a segment of the current buffer as the new buffer
				auto *segment_buffer = buffer->segment(segment_stream.tell(), data_available_bytes);
				delete buffer;
				buffer = segment_buffer;
			}

			// Create a new stream to read from the new buffer
			segment_stream = BitStream(*buffer);
		}

		// Load the root object
		try
		{
			load_object(dest, segment_stream);
		}
		catch (runtime_error &e)
		{
			delete buffer;
			throw e;
		}

		// Free resources
		delete buffer;
	}

	void SerializerBinary::preload_object(pclass::PropertyClass *&dest, BitStream &stream) const
	{
		const auto type_hash = stream.read<pclass::hash_t>();
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

	void SerializerBinary::load_object(pclass::PropertyClass *&dest, BitStream &stream) const
	{
		// Read the object header
		preload_object(dest, stream);

		// Did we get an object or null?
		if (!dest)
			return;
		auto &properties = dest->get_properties();
		
		if (m_is_file)
		{
			// Read the object's size, and create a new BitBufferSegment to
			// ensure that data is only read from inside this region.
			const auto object_size =
				stream.read<uint32_t>() - bitsizeof<uint32_t>::value;
			auto *object_buffer = stream.buffer().segment(
				stream.tell(), object_size
			);
			stream.seek(stream.tell() + object_size, false);
			auto object_stream = BitStream(*object_buffer);

			// Instead of loading properties sequentially, the file format specifies
			// the hash of a property before writing its value, so we just need to
			// iterate for how ever many properties the object has declared.
			for (std::size_t i = 0;
				i < properties.get_property_count(); i++)
			{
				// Read the property's size, and create a new BitBufferSegment to
				// ensure that data is only read from inside this region.
				const auto property_size =
					object_stream.read<uint32_t>() - bitsizeof<uint32_t>::value;
				auto *property_buffer = object_buffer->segment(
					object_stream.tell(), property_size
				);
				auto property_stream = BitStream(*property_buffer);
				object_stream.seek(object_stream.tell() + property_size, false);

				// Get the property to load based on it's hash, and then load
				// it's value.
				const auto property_hash = property_stream.read<uint32_t>();
				auto &prop = properties.get_property(property_hash);
				load_property(prop, property_stream);
				delete property_buffer;
			}
			delete object_buffer;
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

		// Re-align the stream so that our position lies on a byte
		stream.seek(BitStream::stream_pos(stream.tell().as_bytes(), 0), false);
	}

	void SerializerBinary::load_property(pclass::IProperty &prop, BitStream &stream) const
	{
		auto &property_type = prop.get_type();
		if (prop.is_dynamic())
		{
			auto &dynamic_property =
				dynamic_cast<pclass::IDynamicProperty &>(prop);

			// How many elements are there in this dynamic property?
			const auto element_count = stream.read<uint32_t>();
			dynamic_property.set_element_count(element_count);

			// Load each of the elements
			for (uint16_t i = 0; i < element_count; i++)
			{
				// Is this a collection of pointers?
				if (dynamic_property.is_pointer())
				{
					// Is the property a collection of pointers to other objects?
					if (property_type.get_kind() == pclass::Type::kind::CLASS)
					{
						// Read the object as a nested object
						pclass::PropertyClass *object = nullptr;
						load_object(object, stream);
						dynamic_property.set_object(object, i);
					}
					else
						// Read the value as normal (let the property deal with dereferencing)
						dynamic_property.read_value_from(stream, i);
				}
				else
					// If the value isn't a pointer, and it's not dynamic, just read it as a value
					dynamic_property.read_value_from(stream, i);
			}
		}
		else if (prop.is_pointer())
		{
			// Does this property hold a pointer to another object?
			if (property_type.get_kind() == pclass::Type::kind::CLASS)
			{
				// Read the object as a nested object
				pclass::PropertyClass *object = nullptr;
				load_object(object, stream);
				prop.set_object(object);
			}
			else
				// Read the value as normal (let the property deal with dereferencing)
				prop.read_value_from(stream);
		}
		else
			// If the value isn't a pointer, and it's not dynamic, just read it as a value
			prop.read_value_from(stream);
	}
}
}
