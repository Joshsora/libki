#pragma once
#include <cstdint>
#include "ki/pclass/TypeSystem.h"
#include "ki/pclass/Property.h"
#include "ki/pclass/PropertyClass.h"
#include "ki/util/BitStream.h"
#include "ki/util/FlagsEnum.h"

namespace ki
{
namespace serialization
{
	/**
	 * TODO: Documentation
	 */
	class BinarySerializer
	{
	public:
		/**
		 * These flags control how the serializer reads/writes data.
		 */
		enum class flags : uint32_t
		{
			NONE = 0,

			/**
			 * When enabled, the flags the serializer was constructed with are written
			 * into an unsigned 32-bit integer before writing any data.
			 */
			WRITE_SERIALIZER_FLAGS = 0x01,

			/**
			 * When enabled, the serialized data (after the flags, if present) is compressed.
			 */
			COMPRESSED = 0x08
		};

		/**
		 * Construct a new binary serializer.
		 * @param type_system The TypeSystem instance to acquire Type information from.
		 * @param is_file Determines whether or not to write type sizes, and property headers.
		 * @param flags Determines how serialized data is formatted.
		 */
		explicit BinarySerializer(const pclass::TypeSystem &type_system,
			bool is_file, flags flags);
		virtual ~BinarySerializer() {}

		/**
		 * @param object 
		 * @param stream 
		 */
		void save(const pclass::PropertyClass *object, BitStream &stream);

		/**
		 * @param dest 
		 * @param stream
		 * @param size
		 */
		void load(std::unique_ptr<pclass::PropertyClass> &dest,
			BitStream &stream, std::size_t size);

	protected:
		/**
		 * @param object 
		 * @param stream 
		 */
		virtual void presave_object(const pclass::PropertyClass *object, BitStream &stream) const;

		/**
		 * @param object
 		 * @param stream
		 */
		void save_object(const pclass::PropertyClass *object, BitStream &stream) const;

		/**
		 * @param prop
		 * @param stream
		 */
		void save_property(const pclass::IProperty &prop, BitStream &stream) const;

		/**
		 * Read an object header, and instantiate the necessary PropertyClass.
		 * @param dest 
		 * @param stream 
		 */
		virtual void preload_object(
			std::unique_ptr<pclass::PropertyClass> &dest, BitStream &stream) const;
		
		/**
		 * @param dest
		 * @param stream
		 */
		void load_object(
			std::unique_ptr<pclass::PropertyClass> &dest, BitStream &stream) const;

		/**
		 * @param prop
		 * @param stream
		 */
		void load_property(pclass::IProperty &prop, BitStream &stream) const;

	private:
		const pclass::TypeSystem *m_type_system;
		bool m_is_file;
		flags m_flags;

		const pclass::PropertyClass *m_root_object;
	};
}
}

// Make sure the flags enum can be used like a bitflag
MAKE_FLAGS_ENUM(ki::serialization::BinarySerializer::flags);
