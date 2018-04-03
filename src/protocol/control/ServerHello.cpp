#include "ki/protocol/control/ServerHello.h"
#include "ki/dml/Record.h"

namespace ki
{
namespace protocol
{
namespace control
{
	ServerHello::ServerHello(uint16_t session_id,
		uint32_t timestamp, uint32_t milliseconds)
	{
		m_session_id = session_id;
		m_timestamp = timestamp;
		m_milliseconds = milliseconds;
	}

	uint16_t ServerHello::get_session_id() const
	{
		return m_session_id;
	}

	void ServerHello::set_session_id(uint16_t session_id)
	{
		m_session_id = session_id;
	}

	uint32_t ServerHello::get_timestamp() const
	{
		return m_timestamp;
	}

	void ServerHello::set_timestamp(uint32_t timestamp)
	{
		m_timestamp = timestamp;
	}

	uint32_t ServerHello::get_milliseconds() const
	{
		return m_milliseconds;
	}

	void ServerHello::set_milliseconds(uint32_t milliseconds)
	{
		m_milliseconds = milliseconds;
	}

	void ServerHello::write_to(std::ostream& ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("m_session_id")->set_value(m_session_id);
		record.add_field<dml::UINT>("unknown");
		record.add_field<dml::UINT>("m_timestamp")->set_value(m_timestamp);
		record.add_field<dml::UINT>("m_milliseconds")->set_value(m_milliseconds);
		record.write_to(ostream);
	}

	void ServerHello::read_from(std::istream& istream)
	{
		dml::Record record;
		auto *session_id = record.add_field<dml::USHRT>("m_session_id");
		record.add_field<dml::UINT>("unknown");
		auto *timestamp = record.add_field<dml::UINT>("m_timestamp");
		auto *milliseconds = record.add_field<dml::UINT>("m_milliseconds");
		record.read_from(istream);

		m_session_id = session_id->get_value();
		m_timestamp = timestamp->get_value();
		m_milliseconds = milliseconds->get_value();
	}

	size_t ServerHello::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::GID) +
			sizeof(dml::UINT);
	}

	Packet *ServerHello::create_packet(uint16_t session_id,
		uint32_t timestamp, uint32_t milliseconds)
	{
		const ServerHello data(session_id, timestamp, milliseconds);
		auto *packet = new Packet(true, (uint8_t)Opcode::SERVER_HELLO);
		packet->set_payload_data(data);
		return packet;
	}
}
}
}