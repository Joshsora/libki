#include "ki/protocol/control/ClientHello.h"
#include "ki/dml/Record.h"

namespace ki
{
namespace protocol
{
namespace control
{
	ClientHello::ClientHello(uint16_t session_id,
		uint32_t timestamp, uint32_t milliseconds)
	{
		m_session_id = session_id;
		m_timestamp = timestamp;
		m_milliseconds = milliseconds;
	}

	uint16_t ClientHello::get_session_id() const
	{
		return m_session_id;
	}

	void ClientHello::set_session_id(uint16_t session_id)
	{
		m_session_id = session_id;
	}

	uint32_t ClientHello::get_timestamp() const
	{
		return m_timestamp;
	}

	void ClientHello::set_timestamp(uint32_t timestamp)
	{
		m_timestamp = timestamp;
	}

	uint32_t ClientHello::get_milliseconds() const
	{
		return m_milliseconds;
	}

	void ClientHello::set_milliseconds(uint32_t milliseconds)
	{
		m_milliseconds = milliseconds;
	}

	void ClientHello::write_to(std::ostream& ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("unknown");
		record.add_field<dml::UINT>("unknown2");
		record.add_field<dml::UINT>("m_timestamp")->set_value(m_timestamp);
		record.add_field<dml::UINT>("m_milliseconds")->set_value(m_milliseconds);
		record.add_field<dml::USHRT>("m_session_id")->set_value(m_session_id);
		record.write_to(ostream);
	}

	void ClientHello::read_from(std::istream& istream)
	{
		dml::Record record;
		record.add_field<dml::USHRT>("unknown");
		record.add_field<dml::UINT>("unknown2");
		auto *timestamp = record.add_field<dml::UINT>("m_timestamp");
		auto *milliseconds = record.add_field<dml::UINT>("m_milliseconds");
		auto *session_id = record.add_field<dml::USHRT>("m_session_id");
		record.read_from(istream);

		m_timestamp = timestamp->get_value();
		m_milliseconds = milliseconds->get_value();
		m_session_id = session_id->get_value();
	}

	size_t ClientHello::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::UINT) + 
			sizeof(dml::UINT) + sizeof(dml::UINT) +
			sizeof(dml::USHRT);
	}

	Packet *ClientHello::create_packet(uint16_t session_id,
		uint32_t timestamp, uint32_t milliseconds)
	{
		const ClientHello data(session_id, timestamp, milliseconds);
		auto *packet = new Packet(true, (uint8_t)Opcode::CLIENT_HELLO);
		packet->set_payload_data(data);
		return packet;
	}
}
}
}