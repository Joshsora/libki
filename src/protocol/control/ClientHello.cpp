#include "ki/protocol/control/ClientHello.h"
#include "ki/dml/Record.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace control
{
	ClientHello::ClientHello(const uint16_t session_id,
		const int32_t timestamp, const uint32_t milliseconds)
	{
		m_session_id = session_id;
		m_timestamp = timestamp;
		m_milliseconds = milliseconds;
	}

	uint16_t ClientHello::get_session_id() const
	{
		return m_session_id;
	}

	void ClientHello::set_session_id(const uint16_t session_id)
	{
		m_session_id = session_id;
	}

	int32_t ClientHello::get_timestamp() const
	{
		return m_timestamp;
	}

	void ClientHello::set_timestamp(const int32_t timestamp)
	{
		m_timestamp = timestamp;
	}

	uint32_t ClientHello::get_milliseconds() const
	{
		return m_milliseconds;
	}

	void ClientHello::set_milliseconds(const uint32_t milliseconds)
	{
		m_milliseconds = milliseconds;
	}

	void ClientHello::write_to(std::ostream& ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("unknown");
		record.add_field<dml::UINT>("unknown2");
		record.add_field<dml::INT>("m_timestamp")->set_value(m_timestamp);
		record.add_field<dml::UINT>("m_milliseconds")->set_value(m_milliseconds);
		record.add_field<dml::USHRT>("m_session_id")->set_value(m_session_id);
		record.write_to(ostream);
	}

	void ClientHello::read_from(std::istream& istream)
	{
		dml::Record record;
		record.add_field<dml::USHRT>("unknown");
		record.add_field<dml::UINT>("unknown2");
		auto *timestamp = record.add_field<dml::INT>("m_timestamp");
		auto *milliseconds = record.add_field<dml::UINT>("m_milliseconds");
		auto *session_id = record.add_field<dml::USHRT>("m_session_id");
		try
		{
			record.read_from(istream);
		}
		catch (dml::parse_error &e)
		{
			std::ostringstream oss;
			oss << "Error reading ClientHello payload: " << e.what();
			throw parse_error(oss.str());
		}

		m_timestamp = timestamp->get_value();
		m_milliseconds = milliseconds->get_value();
		m_session_id = session_id->get_value();
	}

	size_t ClientHello::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::UINT) + 
			sizeof(dml::INT) + sizeof(dml::UINT) +
			sizeof(dml::USHRT);
	}
}
}
}
