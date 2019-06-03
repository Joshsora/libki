#pragma once
#include "ki/pclass/Type.h"

namespace ki
{
namespace pclass
{
	namespace detail
	{
		/**
		 * Provides implementations to PrimitiveType<ValueT>::is_byte_based,
		 * PrimitiveType<ValueT>::write_to, and PrimitiveType<ValueT>::read_from.
		 */
		template <typename ValueT, typename Enable = void>
		struct primitive_type_helper
		{
			static bool is_byte_based()
			{
				// Provide a compiler error if this is not specialized
				static_assert(
					sizeof(ValueT) == 0,
					"Missing specialization of primitive_type_helper::is_byte_based"
				);
			}

			static void write_to(BitStream &stream, const bool is_file, const ValueT &value)
			{
				// Provide a compiler error if this is not specialized
				static_assert(
					sizeof(ValueT) == 0,
					"Missing specialization of primitive_type_helper::write_to"
				);
			}

			static Value read_from(BitStream &stream, const bool is_file)
			{
				// Provide a compiler error if this is not specialized
				static_assert(
					sizeof(ValueT) == 0,
					"Missing specialization of primitive_type_helper::read_from"
				);
			}
		};

		/**
		 * Specialization of primitive_type_helper for integer types.
		 */
		template <typename ValueT>
		struct primitive_type_helper<
			ValueT,
			typename std::enable_if<std::is_integral<ValueT>::value>::type
		>
		{
			static bool is_byte_based()
			{
				return true;
			}

			static void write_to(BitStream &stream, const bool is_file, const ValueT &value)
			{
				stream.write<ValueT>(value);
			}

			static Value read_from(BitStream &stream, const bool is_file)
			{
				return Value::make_value<ValueT>(
					stream.read<ValueT>()
				);
			}
		};

		/**
		* Specialization of primitive_type_helper for ki::BitInteger<> types.
		*/
		template <uint8_t N, bool Unsigned>
		struct primitive_type_helper<ki::BitInteger<N, Unsigned>>
		{
		private:
			using type = ki::BitInteger<N, Unsigned>;

		public:
			static bool is_byte_based()
			{
				return false;
			}

			static void write_to(BitStream &stream, const bool is_file, const type &value)
			{
				stream.write<type>(value);
			}

			static Value read_from(BitStream &stream, const bool is_file)
			{
				return Value::make_value<type>(
					stream.read<type>()
				);
			}
		};

		/**
		* Specialization of primitive_type_helper for boolean types.
		*/
		template <>
		struct primitive_type_helper<bool>
		{
		private:
			using underlying_type = ki::BitInteger<1, true>;

		public:
			static bool is_byte_based()
			{
				return false;
			}

			static void write_to(BitStream &stream, const bool is_file, const bool &value)
			{
				stream.write<underlying_type>(value);
			}

			static Value read_from(BitStream &stream, const bool is_file)
			{
				return Value::make_value<bool>(
					stream.read<underlying_type>() != 0
				);
			}
		};
		
		/**
		 * Specialization of primitive_type_helper for floating point types.
		 */
		template <typename ValueT>
		struct primitive_type_helper<
			ValueT,
			typename std::enable_if<std::is_floating_point<ValueT>::value>::type
		>
		{
		private:
			/**
			* An unsigned integer type with the same size as the floating point type
			* ValueT.
			*/
			using uint_type = typename bits<bitsizeof<ValueT>::value>::uint_type;

		public:
			static bool is_byte_based()
			{
				return true;
			}

			static void write_to(BitStream &stream, const bool is_file, const ValueT &value)
			{
				// Reinterpret the reference as a reference to an integer
				const uint_type &v = *(
					reinterpret_cast<const uint_type *>(&value)
				);
				stream.write<uint_type>(v, bitsizeof<ValueT>::value);
			}

			static Value read_from(BitStream &stream, const bool is_file)
			{
				uint_type uint_value = stream.read<uint_type>(bitsizeof<ValueT>::value);
				return Value::make_value<ValueT>(*reinterpret_cast<ValueT *>(&uint_value));
			}
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
			static bool is_byte_based()
			{
				return true;
			}

			static void write_to(BitStream &stream, const bool is_file, const type &value)
			{
				if (is_file)
				{
					// TODO: Determine how the size of the length prefix is chosen
					// while in file mode
					stream.write<uint8_t>(value.length() * 2);
				}
				else
				{
					// Write the length as an unsigned short
					stream.write<uint16_t>(value.length());
				}

				// Write each character as _Elem
				for (auto it = value.begin(); it != value.end(); ++it)
					stream.write<_Elem>(*it);
			}

			static Value read_from(BitStream &stream, const bool is_file)
			{
				std::size_t length;
				if (is_file)
				{
					// TODO: Determine how the size of the length prefix is chosen
					// while in file mode
					length = stream.read<uint8_t>() / 2;
				}
				else
				{
					// Read the length as an unsigned short
					length = stream.read<uint16_t>();
				}

				// Create a new string with the correct capacity
				auto value = type(length, ' ');

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

		bool is_byte_based() const override
		{
			return detail::primitive_type_helper<ValueT>::is_byte_based();
		}

		void write_to(BitStream &stream, const bool is_file, Value &value) const override
		{
			try
			{
				Value casted_value = value.as<ValueT>();
				detail::primitive_type_helper<ValueT>::write_to(
					stream, is_file,
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

		Value read_from(BitStream &stream, const bool is_file) const override
		{
			try
			{
				return detail::primitive_type_helper<ValueT>::read_from(stream, is_file);
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
