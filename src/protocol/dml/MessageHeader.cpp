#include "ki/protocol/dml/MessageHeader.h"
#include "ki/dml/Record.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	MessageHeader::MessageHeader(const uint8_t service_id,
		const uint8_t type, const uint16_t size)
	{
		m_service_id = service_id;
		m_type = type;
		m_size = size;
	}

	uint8_t MessageHeader::get_service_id() const
	{
		return m_service_id;
	}

	void MessageHeader::set_service_id(const uint8_t service_id)
	{
		m_service_id = service_id;
	}

	uint8_t MessageHeader::get_type() const
	{
		return m_type;
	}

	void MessageHeader::set_type(const uint8_t type)
	{
		m_type = type;
	}

	uint16_t MessageHeader::get_message_size() const
	{
		return m_size;
	}

	void MessageHeader::set_message_size(const uint16_t size)
	{
		m_size = size;
	}

	void MessageHeader::write_to(std::ostream& ostream) const
	{
		ki::dml::Record record;
		record.add_field<ki::dml::UBYT>("m_service_id")->set_value(m_service_id);
		record.add_field<ki::dml::UBYT>("m_type")->set_value(m_type);
		record.add_field<ki::dml::USHRT>("m_size")->set_value(m_size + 4);
		record.write_to(ostream);
	}

	void MessageHeader::read_from(std::istream& istream)
	{
		ki::dml::Record record;
		const auto *service_id = record.add_field<ki::dml::UBYT>("m_service_id");
		const auto *type = record.add_field<ki::dml::UBYT>("m_type");
		const auto size = record.add_field<ki::dml::USHRT>("m_size");

		try
		{
			record.read_from(istream);
		}
		catch (ki::dml::parse_error &e)
		{
			std::ostringstream oss;
			oss << "Error reading MessageHeader: " << e.what();
			throw parse_error(oss.str(), parse_error::INVALID_HEADER_DATA);
		}

		m_service_id = service_id->get_value();
		m_type = type->get_value();
		m_size = size->get_value() - 4;
	}

	size_t MessageHeader::get_size() const
	{
		return sizeof(ki::dml::UBYT) + sizeof(ki::dml::UBYT) +
			sizeof(ki::dml::USHRT);
	}
}
}
}
