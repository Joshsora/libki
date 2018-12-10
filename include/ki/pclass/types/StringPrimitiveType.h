#pragma once
#include <string>

namespace ki
{
namespace pclass
{
namespace detail
{
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
}
}
