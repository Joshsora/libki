#include "ki/serialization/SerializerBinary.h"
#include <zlib.h>
#include <cassert>

namespace ki
{
namespace serialization
{
	SerializerBinary::SerializerBinary(const pclass::TypeSystem* type_system,
		const bool is_file, const flags flags)
	{
		m_type_system = type_system;
		m_is_file = is_file;
		m_flags = flags;
		m_root_object = nullptr;
	}

	void SerializerBinary::save(const pclass::PropertyClass* object, BitStream& stream)
	{
		// Write the serializer flags
		if (FLAG_IS_SET(m_flags, flags::WRITE_SERIALIZER_FLAGS))
			stream.write<uint32_t>(static_cast<uint32_t>(m_flags));

		// Remember where we started writing data, so we can compress later
		// if necessary.
		const auto start_pos = stream.tell();

		// If the contents of the stream are going to be compressed,
		// reserve space to put the length
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
			stream.write<uint32_t>(0);

		// Write the object to the stream
		m_root_object = object;
		save_object(object, stream);

		// Compress the contents of the stream
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
		{
			// Remember where the ending of the data is
			const auto end_pos = stream.tell();
			const auto size_bits = (end_pos - (start_pos + bitsizeof<uint32_t>::value)).as_bits();
			const auto size_bytes = (size_bits / 8) + (size_bits % 8 > 0 ? 1 : 0);

			// Make a copy of the uncompressed data
			auto *uncompressed = new uint8_t[size_bytes];
			stream.read_copy(uncompressed, size_bits);

			// Setup compression
			static const std::size_t bufsize = 1024;
			auto *temp_buffer = new uint8_t[bufsize];
			std::vector<uint8_t> compressed(size_bytes);
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

			// Move the contents of the compressed buffer if the compressed
			// buffer is smaller
			stream.seek(start_pos);
			stream.write<uint32_t>(size_bytes);
			if (compressed.size() < size_bytes)
				stream.write_copy(compressed.data(), compressed.size() * 8);
			else
				// Go back to the end of the data
				stream.seek(end_pos);

			// Cleanup temporary buffers
			delete[] uncompressed;
			delete[] temp_buffer;
		}
	}

	void SerializerBinary::presave_object(const pclass::PropertyClass *object, BitStream& stream) const
	{
		// If we have an object, write the type hash, otherwise, write NULL (0).
		if (object)
			stream.write<uint32_t>(object->get_type().get_hash());
		else
			stream.write<uint32_t>(NULL);
	}

	void SerializerBinary::save_object(const pclass::PropertyClass *object, BitStream& stream) const
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
	}

	void SerializerBinary::save_property(const pclass::PropertyBase *prop, BitStream& stream) const
	{
		// Remember where we started writing the property data
		const auto start_pos = stream.tell();

		// Do we need to write the property header?
		if (m_is_file)
		{
			// Reserve space to put the size of the property
			stream.write<uint32_t>(0);
			stream.write<uint32_t>(prop->get_full_hash());
		}

		// Is the property dynamic? (holding more than one value)
		auto &property_type = prop->get_type();
		if (prop->is_dynamic())
		{
			// Cast the property to a DynamicPropertyBase
			const auto *dynamic_property =
				dynamic_cast<const pclass::DynamicPropertyBase *>(prop);

			// Write the number of elements
			stream.write<uint16_t>(dynamic_property->get_element_count());

			// Iterate through the elements
			for (auto i = 0; i < dynamic_property->get_element_count(); i++)
			{
				// Is this a collection of pointers?
				if (prop->is_pointer())
				{
					// Is the property a collection of pointers to other objects?
					if (property_type.get_kind() == pclass::Type::kind::CLASS)
						// Write the value as a nested object
						save_object(dynamic_property->get_object(i), stream);
					else
						// Write the value as normal (let the property deal with dereferencing)
						dynamic_property->write_value_to(stream, i);
				}
				else
					// If the value isn't a pointer, and it's not dynamic, just write it as a value
					dynamic_property->write_value_to(stream, i);
			}
		}
		else if (prop->is_pointer())
		{
			// Does this property hold a pointer to another object?
			if (property_type.get_kind() == pclass::Type::kind::CLASS)
				// Write the value as a nested object
				save_object(prop->get_object(), stream);
			else
				// Write the value as normal (let the property deal with dereferencing)
				prop->write_value_to(stream);
		}
		else
			// If the value isn't a pointer, and it's not dynamic, just write it as a value
			prop->write_value_to(stream);

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

	void SerializerBinary::load(pclass::PropertyClass*& dest, BitStream& stream)
	{
		// Read the serializer flags
		if (FLAG_IS_SET(m_flags, flags::WRITE_SERIALIZER_FLAGS))
			m_flags = static_cast<flags>(stream.read<uint32_t>());

		// Decompress the contents of the stream
		if (FLAG_IS_SET(m_flags, flags::COMPRESSED))
		{
			
		}
	}

	void SerializerBinary::preload_object(pclass::PropertyClass*& dest, BitStream& stream) const
	{
		
	}

	void SerializerBinary::load_object(pclass::PropertyClass*& dest, BitStream& stream) const
	{
		
	}

	void SerializerBinary::load_property(pclass::PropertyBase* prop, BitStream& stream) const
	{
		
	}
}
}
