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
		SERVER_HELLO = 0,
		UDP_HELLO = 1,
		PING = 3,
		PING_RSP = 4,
		CLIENT_HELLO = 5
	};
}
}
}