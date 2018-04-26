#include "ki/protocol/control/SessionOffer.h"
#include "ki/dml/Record.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace control
{
	SessionOffer::SessionOffer(const uint16_t session_id,
		const int32_t timestamp, const uint32_t milliseconds)
	{
		m_session_id = session_id;
		m_timestamp = timestamp;
		m_milliseconds = milliseconds;
	}

	uint16_t SessionOffer::get_session_id() const
	{
		return m_session_id;
	}

	void SessionOffer::set_session_id(const uint16_t session_id)
	{
		m_session_id = session_id;
	}

	int32_t SessionOffer::get_timestamp() const
	{
		return m_timestamp;
	}

	void SessionOffer::set_timestamp(const int32_t timestamp)
	{
		m_timestamp = timestamp;
	}

	uint32_t SessionOffer::get_milliseconds() const
	{
		return m_milliseconds;
	}

	void SessionOffer::set_milliseconds(const uint32_t milliseconds)
	{
		m_milliseconds = milliseconds;
	}

	void SessionOffer::write_to(std::ostream& ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("m_session_id")->set_value(m_session_id);
		record.add_field<dml::UINT>("unknown");
		record.add_field<dml::INT>("m_timestamp")->set_value(m_timestamp);
		record.add_field<dml::UINT>("m_milliseconds")->set_value(m_milliseconds);
		record.write_to(ostream);
	}

	void SessionOffer::read_from(std::istream& istream)
	{
		dml::Record record;
		auto *session_id = record.add_field<dml::USHRT>("m_session_id");
		record.add_field<dml::UINT>("unknown");
		auto *timestamp = record.add_field<dml::INT>("m_timestamp");
		auto *milliseconds = record.add_field<dml::UINT>("m_milliseconds");
		try
		{
			record.read_from(istream);
		}
		catch (dml::parse_error &e)
		{
			std::ostringstream oss;
			oss << "Error reading SessionOffer payload: " << e.what();
			throw parse_error(oss.str(), parse_error::INVALID_MESSAGE_DATA);
		}

		m_session_id = session_id->get_value();
		m_timestamp = timestamp->get_value();
		m_milliseconds = milliseconds->get_value();
	}

	size_t SessionOffer::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::UINT) +
			sizeof(dml::INT) + sizeof(dml::UINT);
	}
}
}
}
