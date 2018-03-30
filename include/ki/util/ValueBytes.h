#pragma once

namespace ki
{
	template <typename ValueT>
	union ValueBytes {
		ValueT value = 0;
		char buff[sizeof(ValueT)];
	};
}
