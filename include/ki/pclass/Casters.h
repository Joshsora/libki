#pragma once
#include <type_traits>
#include <sstream>
#include <json.hpp>
#include "ki/pclass/Value.h"
#include "ki/util/BitTypes.h"
#include "ki/pclass/types/EnumType.h"

namespace ki
{
namespace pclass
{
namespace detail
{
	/**
	 * Determines whether a type can be assigned to a
	 * nlohmann::json object.
	 */
	template <typename SrcT, typename Enable = void>
	struct is_json_assignable : std::false_type {};

	/**
	 * All fundamental types can be assigned to a json object.
	 */
	template <typename SrcT>
	struct is_json_assignable<
		SrcT,
		typename std::enable_if<std::is_fundamental<SrcT>::value>::type
	> : std::true_type {};

	/**
	 * std::string can be assigned to a json object.
	 */
	template <>
	struct is_json_assignable<std::string> : std::true_type {};

	/**
	 * std::u16string can be assigned to a json object.
	 */
	template <>
	struct is_json_assignable<std::u16string> : std::true_type {};

	/**
	 * value_caster specialization for the generic case of casting
	 * any json-assignable value to a json object.
	 */
	template <typename SrcT>
	struct value_caster<
		SrcT, nlohmann::json,
		typename std::enable_if<is_json_assignable<SrcT>::value>::type
	>
		: value_caster_impl<SrcT, nlohmann::json>
	{
		nlohmann::json cast_value(const SrcT &value) const override
		{
			return value;
		}
	};

	/**
	 * value_caster specialization for casting bit integers (bi<N> and bui<N>)
	 * to a json object.
	 */
	template <uint8_t N, bool Unsigned>
	struct value_caster<
		BitInteger<N, Unsigned>, nlohmann::json
	>
		: value_caster_impl<BitInteger<N, Unsigned>, nlohmann::json>
	{
		using type = typename std::conditional<
			Unsigned,
			typename bits<N>::uint_type,
			typename bits<N>::int_type
		>::type;

		nlohmann::json cast_value(
			const BitInteger<N, Unsigned> &value) const override
		{
			return static_cast<type>(value);
		}
	};

	/**
	 * value_caster specialization for casting enum to bit integer types 
	 * (bi<N> and bui<N>).
	 */
	template <typename SrcT, uint8_t N, bool Unsigned>
	struct value_caster<
		SrcT, BitInteger<N, Unsigned>,
		typename std::enable_if<std::is_enum<SrcT>::value>::type
	>
		: value_caster_impl<SrcT, BitInteger<N, Unsigned>>
	{
		using underlying_type = typename std::underlying_type<SrcT>::type;
	
		BitInteger<N, Unsigned> cast_value(const SrcT &value) const override
		{
			return static_cast<underlying_type>(value);
		}
	};

	/**
	 * value_caster specialization for casting enums to a json object.
	 */
	template <typename SrcT>
	struct value_caster<
		SrcT, nlohmann::json,
		typename std::enable_if<std::is_enum<SrcT>::value>::type
	>
		: value_caster_impl<SrcT, nlohmann::json>
	{
		using underlying_type = typename std::underlying_type<SrcT>::type;

		nlohmann::json cast_value(const SrcT &value) const override
		{
			return static_cast<underlying_type>(value);
		}
	};

	template <typename T, typename Enable = void>
	struct string_cast_t
	{
		using type = T;
	};

	/**
	 * Writing a int8_t or uint8_t value to a stream will
	 * write a ASCII character, rather than an integer, so
	 * we cast to int16_t/uint16_t before writing.
	 */
	template <>
	struct string_cast_t<int8_t>
	{
		using type = int16_t;
	};
	template <>
	struct string_cast_t<uint8_t>
	{
		using type = uint16_t;
	};

	/**
	 * Enums should be written as 32-bit integers.
	 */
	template <typename T>
	struct string_cast_t<
		T,
		typename std::enable_if<std::is_enum<T>::value>::type
	>
	{
		using type = enum_value_t;
	};

	/**
	 * Caster implementation for casting any type to string
	 * via std::ostringstream.
	 */
	template <typename SrcT>
	struct value_caster<SrcT, std::string>
		: value_caster_impl<SrcT, std::string>
	{
		std::string cast_value(const SrcT &value) const override
		{
			std::ostringstream oss;
			auto casted_value = static_cast<
				typename string_cast_t<SrcT>::type>(value);
			oss << casted_value;
			return oss.str();
		}
	};

	/**
	 * Caster implementation for casting from json to any
	 * primitive type.
	 */
	template <typename DestT>
	struct value_caster<nlohmann::json, DestT>
		: value_caster_impl<nlohmann::json, DestT>
	{
		DestT cast_value(const nlohmann::json &value) const override
		{
			return value;
		}
	};

	/**
	 * Caster implementation for casting from json to a
	 * std::string.
	 */
	template <>
	struct value_caster<nlohmann::json, std::string>
		: value_caster_impl<nlohmann::json, std::string>
	{
		std::string cast_value(const nlohmann::json &value) const override
		{
			return value;
		}
	};

	/**
	 * A utility to call ValueCaster::declare<SrcT, DestT> with
	 * bi<N> and bui<N> as the destination type.
	 * 
	 * N is automatically decremented until it reaches 0.
	 */
	template <typename T, int N = 7>
	struct bit_integer_declarer
	{
		static void declare()
		{
			bit_integer_declarer<T, N - 1>::declare();
			ValueCaster::declare<T, bi<N>>();
			ValueCaster::declare<T, bui<N>>();
		}
	};

	/**
	 * Specialization for bit_integer_declarer.
	 * Stop decrementing N when it reaches 0.
	 */
	template <typename T>
	struct bit_integer_declarer<T, 0>
	{
		static void declare() {}
	};

	/**
	 * Utility class used by TypeSystem to declare casters for each
	 * primitive type.
	 */
	template <typename T, typename Enable = void>
	struct caster_declarer
	{
		static void declare() {}
	};

	/**
	 * caster_declarer specialization for all integral types.
	 * Integers can cast to other integer types, as well as floating point
	 * integers, std::string and nlohmann::json.
	 */
	template <typename T>
	struct caster_declarer<
		T,
		typename std::enable_if<is_integral<T>::value>::type
	>
	{
		static void declare()
		{
			ValueCaster::declare<T, bool>();
			bit_integer_declarer<T>::declare();
			ValueCaster::declare<T, int8_t>();
			ValueCaster::declare<T, int16_t>();
			ValueCaster::declare<T, bi<24>>();
			ValueCaster::declare<T, int32_t>();
			ValueCaster::declare<T, int64_t>();
			ValueCaster::declare<T, uint8_t>();
			ValueCaster::declare<T, uint16_t>();
			ValueCaster::declare<T, bui<24>>();
			ValueCaster::declare<T, uint32_t>();
			ValueCaster::declare<T, uint64_t>();
			ValueCaster::declare<T, float>();
			ValueCaster::declare<T, double>();
			ValueCaster::declare<T, std::string>();
			ValueCaster::declare<T, nlohmann::json>();
		}
	};

	/**
	 * caster_declarer specialization for all floating point types.
	 * Floating point integers can cast to all integer types, as well as
	 * std::string and nlohmann::json.
	 */
	template <typename T>
	struct caster_declarer<
		T,
		typename std::enable_if<std::is_floating_point<T>::value>::type
	>
	{
		static void declare()
		{
			ValueCaster::declare<T, bool>();
			bit_integer_declarer<T, 7>::declare();
			ValueCaster::declare<T, int8_t>();
			ValueCaster::declare<T, int16_t>();
			ValueCaster::declare<T, bi<24>>();
			ValueCaster::declare<T, int32_t>();
			ValueCaster::declare<T, int64_t>();
			ValueCaster::declare<T, uint8_t>();
			ValueCaster::declare<T, uint16_t>();
			ValueCaster::declare<T, bui<24>>();
			ValueCaster::declare<T, uint32_t>();
			ValueCaster::declare<T, uint64_t>();
			ValueCaster::declare<T, float>();
			ValueCaster::declare<T, double>();
			ValueCaster::declare<T, std::string>();
			ValueCaster::declare<T, nlohmann::json>();
		}
	};

	/**
	 * caster_declarer specialization for enum types.
	 * Enums can be cast to any integer type, as well as std::string,
	 * and nlohmann::json.
	 */
	template <typename T>
	struct caster_declarer<
		T,
		typename std::enable_if<std::is_enum<T>::value>::type
	>
	{
		static void declare()
		{
			ValueCaster::declare<T, bool>();
			bit_integer_declarer<T, 7>::declare();
			ValueCaster::declare<T, int8_t>();
			ValueCaster::declare<T, int16_t>();
			ValueCaster::declare<T, bi<24>>();
			ValueCaster::declare<T, int32_t>();
			ValueCaster::declare<T, int64_t>();
			ValueCaster::declare<T, uint8_t>();
			ValueCaster::declare<T, uint16_t>();
			ValueCaster::declare<T, bui<24>>();
			ValueCaster::declare<T, uint32_t>();
			ValueCaster::declare<T, uint64_t>();
			ValueCaster::declare<T, std::string>();
			ValueCaster::declare<T, nlohmann::json>();
		}
	};

	/**
	 * caster_declarer specialization for string.
	 * Strings can be cast to UTF-16 strings, and json objects.
	 */
	template <>
	struct caster_declarer<std::string>
	{
		static void declare()
		{
			// TODO: Casting string to u16string
			ValueCaster::declare<std::string, nlohmann::json>();
		}
	};

	/**
	 * caster_declarer specialization for UTF-16 strings.
	 * UTF-16 strings can be cast to strings, and json objects.
	 */
	template <>
	struct caster_declarer<std::u16string>
	{
		static void declare()
		{
			// TODO: Casting u16string to string
			ValueCaster::declare<std::u16string, nlohmann::json>();
		}
	};
}
}
}
