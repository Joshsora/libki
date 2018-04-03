#include "ki/protocol/Packet.h"

namespace ki
{
namespace protocol
{
	Packet::Packet(bool control, uint8_t opcode)
	{
		m_control = control;
		m_opcode = opcode;
		m_payload = std::vector<char>();
	}

	bool Packet::is_control() const
	{
		return m_control;
	}

	void Packet::set_control(bool control)
	{
		m_control = control;
	}

	uint8_t Packet::get_opcode() const
	{
		return m_opcode;
	}

	void Packet::set_opcode(uint8_t opcode)
	{
		m_opcode = opcode;
	}

	void Packet::write_to(std::ostream& ostream) const
	{
		ostream.put(m_control);
		ostream.put(m_opcode);
		ostream.put(0);
		ostream.put(0);
		ostream.write(m_payload.data(), m_payload.size());
	}

	void Packet::read_from(std::istream& istream)
	{
		m_control = istream.get() >= 1;
		m_opcode = istream.get();
		istream.ignore(2);

		const std::ios::pos_type pos = istream.tellg();
		istream.seekg(0, std::ios::end);
		const size_t size = istream.tellg() - pos;
		istream.seekg(pos, std::ios::beg);

		m_payload.resize(size);
		istream.read(m_payload.data(), size);
	}

	size_t Packet::get_size() const
	{
		return 4 + m_payload.size();
	}
}
}
