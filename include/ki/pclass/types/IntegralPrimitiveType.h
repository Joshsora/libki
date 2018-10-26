#pragma once
#include <type_traits>

namespace ki
{
namespace pclass
{
	template <typename ValueT>
	struct PrimitiveTypeWriter<
		ValueT,
		typename std::enable_if<std::is_integral<ValueT>::value>::type
	>
	{
		static void write_to(BitStream &stream, const ValueT &value)
		{
			stream.write<ValueT>(value, sizeof(ValueT) * 8);
		}
	};

	template <typename ValueT>
	struct PrimitiveTypeReader<
		ValueT,
		typename std::enable_if<std::is_integral<ValueT>::value>::type
	>
	{
		static void read_from(BitStream &stream, ValueT &value)
		{
			value = stream.read<ValueT>(sizeof(ValueT) * 8);
		}
	};
}
}