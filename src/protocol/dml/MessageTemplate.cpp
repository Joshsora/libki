#include "ki/protocol/dml/MessageTemplate.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	MessageTemplate::MessageTemplate(std::string name, uint8_t type,
		uint8_t service_id, ki::dml::Record* record)
	{
		m_name = name;
		m_type = type;
		m_service_id = service_id;
		m_record = record;
	}

	MessageTemplate::~MessageTemplate()
	{
		delete m_record;
	}

	std::string MessageTemplate::get_name() const
	{
		return m_name;
	}

	void MessageTemplate::set_name(std::string name)
	{
		m_name = name;
	}

	uint8_t MessageTemplate::get_type() const
	{
		return m_type;
	}

	void MessageTemplate::set_type(uint8_t type)
	{
		m_type = type;
	}

	uint8_t MessageTemplate::get_service_id() const
	{
		return m_service_id;
	}

	void MessageTemplate::set_service_id(uint8_t service_id)
	{
		m_service_id = service_id;
	}

	std::string MessageTemplate::get_handler() const
	{
		const auto field = m_record->get_field<ki::dml::STR>("_MsgHandler");
		if (field)
			return field->get_value();
		return m_name;
	}

	void MessageTemplate::set_handler(std::string handler)
	{
		m_record->add_field<ki::dml::STR>("_MsgHandler")->set_value(handler);
	}

	uint8_t MessageTemplate::get_access_level() const
	{
		const auto field = m_record->get_field<ki::dml::UBYT>("_MsgAccessLvl");
		if (field)
			return field->get_value();
		return 0;
	}

	void MessageTemplate::set_access_level(uint8_t access_level)
	{
		m_record->add_field<ki::dml::UBYT>("_MsgAccessLvl")->set_value(access_level);
	}

	const ki::dml::Record& MessageTemplate::get_record() const
	{
		return *m_record;
	}

	void MessageTemplate::set_record(ki::dml::Record* record)
	{
		m_record = record;
	}

	Message *MessageTemplate::create_message() const
	{
		return new Message(this);
	}
}
}
}