#pragma once
#include <string>

namespace ki
{
namespace pclass
{
	template <
		typename _Elem,
		typename _Traits,
		typename _Alloc
	>
	struct PrimitiveTypeWriter<std::basic_string<_Elem, _Traits, _Alloc>>
	{
	private:
		using type = std::basic_string<_Elem, _Traits, _Alloc>;

	public:
		static void write_to(BitStreamBase &stream, const type &value)
		{
			// Write the length as an unsigned short
			stream.write<uint16_t>(value.length());

			// Write each character as _Elem
			for (auto it = value.begin(); it != value.end(); ++it)
				stream.write<_Elem>(*it);
		}
	};

	template <
		typename _Elem,
		typename _Traits,
		typename _Alloc
	>
	struct PrimitiveTypeReader<std::basic_string<_Elem, _Traits, _Alloc>>
	{
	private:
		using type = std::basic_string<_Elem, _Traits, _Alloc>;

	public:
		static void read_from(BitStreamBase &stream, type &value)
		{
			// Read the length and create a new string with the correct capacity
			auto length = stream.read<uint16_t>();
			value = type(length, ' ');

			// Read each character into the string
			for (auto it = value.begin(); it != value.end(); ++it)
				*it = stream.read<_Elem>();
		}
	};
}
}
