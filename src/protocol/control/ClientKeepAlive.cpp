#include "ki/protocol/control/ClientKeepAlive.h"
#include "ki/dml/Record.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace control
{
	ClientKeepAlive::ClientKeepAlive(const  uint16_t session_id, const uint16_t milliseconds,
		const uint16_t minutes)
	{
		m_session_id = session_id;
		m_milliseconds = milliseconds;
		m_minutes = minutes;
	}

	uint16_t ClientKeepAlive::get_session_id() const
	{
		return m_session_id;
	}

	void ClientKeepAlive::set_session_id(const uint16_t session_id)
	{
		m_session_id = session_id;
	}

	uint16_t ClientKeepAlive::get_milliseconds() const
	{
		return m_milliseconds;
	}

	void ClientKeepAlive::set_milliseconds(const uint16_t milliseconds)
	{
		m_milliseconds = milliseconds;
	}

	uint16_t ClientKeepAlive::get_minutes() const
	{
		return m_minutes;
	}

	void ClientKeepAlive::set_minutes(const uint16_t minutes)
	{
		m_minutes = minutes;
	}

	void ClientKeepAlive::write_to(std::ostream &ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("m_session_id")->set_value(m_session_id);
		record.add_field<dml::USHRT>("m_milliseconds")->set_value(m_milliseconds);
		record.add_field<dml::USHRT>("m_minutes")->set_value(m_minutes);
		record.write_to(ostream);
	}

	void ClientKeepAlive::read_from(std::istream &istream)
	{
		dml::Record record;
		auto *session_id = record.add_field<dml::USHRT>("m_session_id");
		auto *milliseconds = record.add_field<dml::USHRT>("m_milliseconds");
		auto *minutes = record.add_field<dml::USHRT>("m_minutes");
		try
		{
			record.read_from(istream);
		}
		catch (dml::parse_error &e)
		{
			std::ostringstream oss;
			oss << "Error reading ClientKeepAlive payload: " << e.what();
			throw parse_error(oss.str());
		}

		m_session_id = session_id->get_value();
		m_milliseconds = milliseconds->get_value();
		m_minutes = minutes->get_value();
	}

	size_t ClientKeepAlive::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::USHRT) +
			sizeof(dml::USHRT);
	}
}
}
}
