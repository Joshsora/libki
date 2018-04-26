#pragma once
#include <cstdint>

namespace ki
{
namespace protocol
{
namespace control
{
	enum class Opcode : uint8_t
	{
		NONE = 0,
		SESSION_OFFER = 0,
		UDP_HELLO = 1,
		KEEP_ALIVE = 3,
		KEEP_ALIVE_RSP = 4,
		SESSION_ACCEPT = 5
	};
}
}
}