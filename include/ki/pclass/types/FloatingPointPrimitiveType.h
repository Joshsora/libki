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
}
}
}
