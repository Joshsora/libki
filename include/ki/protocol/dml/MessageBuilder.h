#pragma once
#include "Message.h"
#include <string>

namespace ki
{
namespace protocol
{
namespace dml
{
	class MessageBuilder
	{
	public:
		MessageBuilder(uint8_t service_id = 0, uint8_t type = 0);

		MessageBuilder &set_service_id(uint8_t service_id);
		MessageBuilder &set_message_type(uint8_t type);
		MessageBuilder &use_template_record(const ki::dml::Record &record);

		template <typename ValueT>
		MessageBuilder &set_field_value(std::string name, ValueT value)
		{
			auto *field = m_message->get_record().get_field<ValueT>(name);
			if (!field)
			{
				// TODO: Exceptions
			}
			field->set_value(value);
			return *this;
		}

		Message *get_message() const;
	private:
		Message *m_message;
	};
}
}
}