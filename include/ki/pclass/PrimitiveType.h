#pragma once
#include "ki/pclass/Type.h"

namespace ki
{
namespace pclass
{
	namespace detail
	{
		/**
		 * Provides implementations to PrimitiveType<ValueT>::write_to,
		 * and PrimitiveType<ValueT>::read_from.
		 */
		template <typename ValueT, typename Enable = void>
		struct primitive_type_helper
		{
			static void write_to(BitStream &stream, const ValueT &value)
			{
				// Provide a compiler error if this is not specialized
				static_assert(
					sizeof(ValueT) == 0,
					"Missing specialization of primitive_type_writer::write_to"
				);
			}

			static Value read_from(BitStream &stream)
			{
				// Provide a compiler error if this is not specialized
				static_assert(
					sizeof(ValueT) == 0,
					"Missing specialization of PrimitiveTypeReader::read_from"
				);

				// This should be impossible to reach.
				throw runtime_error("Missing specialization of PrimitiveTypeReader::read_from");
			}
		};

		/**
		 * Specialization of primitive_type_helper for integer types.
		 * This includes instantiations of ki::BitInteger<>.
		 */
		template <typename ValueT>
		struct primitive_type_helper<
			ValueT,
			typename std::enable_if<is_integral<ValueT>::value>::type
		>
		{
			static void write_to(BitStream &stream, const ValueT &value)
			{
				stream.write<ValueT>(value);
			}

			static Value read_from(BitStream &stream)
			{
				return Value::make_value<ValueT>(
					stream.read<ValueT>()
				);
			}
		};
		
		/**
		 * Specialization of primitive_type_helper for floating point
		 * types.
		 */
		template <typename ValueT>
		struct primitive_type_helper<
			ValueT,
			typename std::enable_if<std::is_floating_point<ValueT>::value>::type
		>
		{
			static void write_to(BitStream &stream, const ValueT &value)
			{
				// Reinterpret the reference as a reference to an integer
				const uint_type &v = *(
					reinterpret_cast<const uint_type *>(&value)
					);
				stream.write<uint_type>(v, bitsizeof<ValueT>::value);
			}

			static Value read_from(BitStream &stream)
			{
				uint_type uint_value = stream.read<uint_type>(bitsizeof<ValueT>::value);
				return Value::make_value<ValueT>(*reinterpret_cast<ValueT *>(&uint_value));
			}

		private:
			/**
			 * An unsigned integer type with the same size as the floating point type
			 * ValueT.
			 */
			using uint_type = typename bits<bitsizeof<ValueT>::value>::uint_type;
		};

		/**
		 * Specialization of primitive_type_helper for string types.
		 */
		template <
			typename _Elem,
			typename _Traits,
			typename _Alloc
		>
			struct primitive_type_helper<std::basic_string<_Elem, _Traits, _Alloc>>
		{
		private:
			using type = std::basic_string<_Elem, _Traits, _Alloc>;

		public:
			static void write_to(BitStream &stream, const type &value)
			{
				// Write the length as an unsigned short
				stream.write<uint16_t>(value.length());

				// Write each character as _Elem
				for (auto it = value.begin(); it != value.end(); ++it)
					stream.write<_Elem>(*it);
			}

			static Value read_from(BitStream &stream)
			{
				// Read the length and create a new string with the correct capacity
				auto length = stream.read<uint16_t>();
				auto value = type(length, ' ');;

				// Read each character into the string
				for (auto it = value.begin(); it != value.end(); ++it)
					*it = stream.read<_Elem>();

				// Copy string value into the return value
				return Value::make_value<type>(value);
			}
		};
	}
	
	/**
	 * A basic data type that can be used to build more complex structures.
	 * @tparam ValueT The compile-time primitive type that the class represents.
	 */
	template <typename ValueT>
	class PrimitiveType : public Type
	{
	public:
		// Do not allow copy construction or movement of types
		PrimitiveType(const PrimitiveType<ValueT> &that) = delete;
		PrimitiveType &operator=(const PrimitiveType<ValueT> &that) = delete;
		PrimitiveType(PrimitiveType<ValueT> &&that) noexcept = delete;
		PrimitiveType &operator=(PrimitiveType<ValueT> &&that) noexcept = delete;

		PrimitiveType(const std::string name, const TypeSystem &type_system)
			: Type(name, type_system)
		{
			m_kind = Kind::PRIMITIVE;
		}
		~PrimitiveType() = default;

		void write_to(BitStream &stream, Value &value) const override
		{
			try
			{
				Value casted_value = value.as<ValueT>();
				detail::primitive_type_helper<ValueT>::write_to(
					stream,
					casted_value.get<ValueT>()
				);
			}
			catch (runtime_error &e)
			{
				std::ostringstream oss;
				oss << "Invalid call to Type::write_to -- " << e.what();
				throw runtime_error(oss.str());
			}
		}

		Value read_from(BitStream &stream) const override
		{
			try
			{
				return detail::primitive_type_helper<ValueT>::read_from(stream);
			}
			catch (runtime_error &e)
			{
				std::ostringstream oss;
				oss << "Invalid call to Type::read_from -- " << e.what();
				throw runtime_error(oss.str());
			}
		}
	};
}
}
