#include "ki/protocol/control/Ping.h"
#include "ki/dml/Record.h"

namespace ki
{
namespace protocol
{
namespace control
{
	Ping::Ping(uint16_t session_id, uint16_t milliseconds, uint8_t minutes)
	{
		m_session_id = session_id;
		m_milliseconds = milliseconds;
		m_minutes = minutes;
	}

	uint16_t Ping::get_session_id() const
	{
		return m_session_id;
	}

	void Ping::set_session_id(uint16_t session_id)
	{
		m_session_id = session_id;
	}

	uint16_t Ping::get_milliseconds() const
	{
		return m_milliseconds;
	}

	void Ping::set_milliseconds(uint16_t milliseconds)
	{
		m_milliseconds = milliseconds;
	}

	uint8_t Ping::get_minutes() const
	{
		return m_minutes;
	}

	void Ping::set_minutes(uint8_t minutes)
	{
		m_minutes = minutes;
	}

	void Ping::write_to(std::ostream &ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("m_session_id")->set_value(m_session_id);
		record.add_field<dml::USHRT>("m_milliseconds")->set_value(m_milliseconds);
		record.add_field<dml::UBYT>("m_minutes")->set_value(m_minutes);
		record.write_to(ostream);
	}

	void Ping::read_from(std::istream &istream)
	{
		dml::Record record;
		auto *session_id = record.add_field<dml::USHRT>("m_session_id");
		auto *milliseconds = record.add_field<dml::USHRT>("m_milliseconds");
		auto *minutes = record.add_field<dml::UBYT>("m_minutes");
		record.read_from(istream);

		m_session_id = session_id->get_value();
		m_milliseconds = milliseconds->get_value();
		m_minutes = minutes->get_value();
	}

	size_t Ping::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::USHRT) +
			sizeof(dml::UBYT);
	}
}
}
}