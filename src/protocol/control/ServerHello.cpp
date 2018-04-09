#include "ki/protocol/control/ServerHello.h"
#include "ki/dml/Record.h"

namespace ki
{
namespace protocol
{
namespace control
{
	ServerHello::ServerHello(const uint16_t session_id,
		const int32_t timestamp, const uint32_t milliseconds)
	{
		m_session_id = session_id;
		m_timestamp = timestamp;
		m_milliseconds = milliseconds;
	}

	uint16_t ServerHello::get_session_id() const
	{
		return m_session_id;
	}

	void ServerHello::set_session_id(const uint16_t session_id)
	{
		m_session_id = session_id;
	}

	int32_t ServerHello::get_timestamp() const
	{
		return m_timestamp;
	}

	void ServerHello::set_timestamp(const int32_t timestamp)
	{
		m_timestamp = timestamp;
	}

	uint32_t ServerHello::get_milliseconds() const
	{
		return m_milliseconds;
	}

	void ServerHello::set_milliseconds(const uint32_t milliseconds)
	{
		m_milliseconds = milliseconds;
	}

	void ServerHello::write_to(std::ostream& ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("m_session_id")->set_value(m_session_id);
		record.add_field<dml::UINT>("unknown");
		record.add_field<dml::INT>("m_timestamp")->set_value(m_timestamp);
		record.add_field<dml::UINT>("m_milliseconds")->set_value(m_milliseconds);
		record.write_to(ostream);
	}

	void ServerHello::read_from(std::istream& istream)
	{
		dml::Record record;
		auto *session_id = record.add_field<dml::USHRT>("m_session_id");
		record.add_field<dml::UINT>("unknown");
		auto *timestamp = record.add_field<dml::INT>("m_timestamp");
		auto *milliseconds = record.add_field<dml::UINT>("m_milliseconds");
		record.read_from(istream);

		m_session_id = session_id->get_value();
		m_timestamp = timestamp->get_value();
		m_milliseconds = milliseconds->get_value();
	}

	size_t ServerHello::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::UINT) + 
			sizeof(dml::INT) + sizeof(dml::UINT);
	}
}
}
}