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
		typename std::enable_if<std::is_floating_point<ValueT>::value>::type
	>
	{
		static void write_to(BitStreamBase &stream, const ValueT &value)
		{
			// Reinterpret the reference as a reference to an integer
			const uint_type &v = *(
				reinterpret_cast<const uint_type *>(&value)
			);
			stream.write<uint_type>(v, bitsizeof<ValueT>::value);
		}

	private:
		/**
          * An unsigned integer type with the same size as the floating point type
		  * ValueT.
		  */
		using uint_type = typename bits<bitsizeof<ValueT>::value>::uint_type;
	};

	template <typename ValueT>
	struct PrimitiveTypeReader<
		ValueT,
		typename std::enable_if<std::is_floating_point<ValueT>::value>::type
	>
	{
		static void read_from(BitStreamBase &stream, ValueT &value)
		{
			// Reinterpret the reference as a reference to an integer
			uint_type &v = *(
				reinterpret_cast<uint_type *>(&value)
			);
			v = stream.read<uint_type>(bitsizeof<ValueT>::value);
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
