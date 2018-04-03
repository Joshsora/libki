#include "ki/protocol/dml/MessageBuilder.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	MessageBuilder::MessageBuilder(uint8_t service_id, uint8_t type)
	{
		m_message = new Message(service_id, type);
	}

	MessageBuilder &MessageBuilder::set_service_id(uint8_t service_id)
	{
		m_message->set_service_id(service_id);
		return *this;
	}

	MessageBuilder &MessageBuilder::set_message_type(uint8_t type)
	{
		m_message->set_type(type);
		return *this;
	}

	MessageBuilder &MessageBuilder::use_template_record(const ki::dml::Record& record)
	{
		m_message->set_record(record);
		return *this;
	}

	Message *MessageBuilder::get_message() const
	{
		return m_message;
	}
}
}
}