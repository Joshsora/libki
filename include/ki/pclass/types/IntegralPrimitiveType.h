#pragma once
#include <type_traits>
#include "ki/util/BitTypes.h"

namespace ki
{
namespace pclass
{
namespace detail
{
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
}
}
}
