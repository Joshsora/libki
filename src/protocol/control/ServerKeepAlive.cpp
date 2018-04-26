#include "ki/protocol/control/ServerKeepAlive.h"
#include "ki/dml/Record.h"
#include "ki/protocol/exception.h"
#include <chrono>

namespace ki
{
namespace protocol
{
namespace control
{
	ServerKeepAlive::ServerKeepAlive(const uint32_t timestamp)
	{
		m_timestamp = timestamp;
	}

	uint32_t ServerKeepAlive::get_timestamp() const
	{
		return m_timestamp;
	}

	void ServerKeepAlive::set_timestamp(const uint32_t timestamp)
	{
		m_timestamp = timestamp;
	}

	void ServerKeepAlive::write_to(std::ostream& ostream) const
	{
		dml::Record record;
		record.add_field<dml::USHRT>("m_session_id");
		record.add_field<dml::INT>("m_timestamp")->set_value(m_timestamp);
		record.write_to(ostream);
	}

	void ServerKeepAlive::read_from(std::istream& istream)
	{
		dml::Record record;
		record.add_field<dml::USHRT>("m_session_id");
		auto *timestamp = record.add_field<dml::INT>("m_timestamp");
		try
		{
			record.read_from(istream);
		}
		catch (dml::parse_error &e)
		{
			std::ostringstream oss;
			oss << "Error reading ServerKeepAlive payload: " << e.what();
			throw parse_error(oss.str(), parse_error::INVALID_MESSAGE_DATA);
		}

		m_timestamp = timestamp->get_value();
	}

	size_t ServerKeepAlive::get_size() const
	{
		return sizeof(dml::USHRT) + sizeof(dml::INT);
	}

}
}
}
