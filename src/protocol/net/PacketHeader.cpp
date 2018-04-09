#include "ki/protocol/net/PacketHeader.h"
#include "ki/protocol/exception.h"
#include <sstream>

namespace ki
{
namespace protocol
{
namespace net
{
	PacketHeader::PacketHeader(const bool control, const uint8_t opcode)
	{
		m_control = control;
		m_opcode = opcode;
	}

	bool PacketHeader::is_control() const
	{
		return m_control;
	}

	void PacketHeader::set_control(const bool control)
	{
		m_control = control;
	}

	uint8_t PacketHeader::get_opcode() const
	{
		return m_opcode;
	}

	void PacketHeader::set_opcode(const uint8_t opcode)
	{
		m_opcode = opcode;
	}

	void PacketHeader::write_to(std::ostream& ostream) const
	{
		ostream.put(m_control);
		ostream.put(m_opcode);
		ostream.put(0);
		ostream.put(0);
	}

	void PacketHeader::read_from(std::istream& istream)
	{
		m_control = istream.get() >= 1;
		if (istream.fail())
			throw parse_error("Not enough data was available to read packet header. (m_control)");
		m_opcode = istream.get();
		if (istream.fail())
			throw parse_error("Not enough data was available to read packet header. (m_opcode)");
		istream.ignore(2);
		if (istream.fail())
			throw parse_error("Not enough data was available to read packet header. (ignored bytes)");
	}

	size_t PacketHeader::get_size() const
	{
		return 4;
	}
}
}
}
