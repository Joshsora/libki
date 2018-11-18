#pragma once
#include <type_traits>
#include "ki/util/BitTypes.h"

namespace ki
{
namespace pclass
{
	template <typename ValueT>
	struct PrimitiveTypeWriter<
		ValueT,
		typename std::enable_if<is_integral<ValueT>::value>::type
	>
	{
		static void write_to(BitStreamBase &stream, const ValueT &value)
		{
			stream.write<ValueT>(value);
		}
	};

	template <typename ValueT>
	struct PrimitiveTypeReader<
		ValueT,
		typename std::enable_if<is_integral<ValueT>::value>::type
	>
	{
		static void read_from(BitStreamBase &stream, ValueT &value)
		{
			value = stream.read<ValueT>();
		}
	};
}
}
