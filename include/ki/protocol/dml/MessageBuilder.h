#pragma once
#include "Message.h"
#include "ki/protocol/exception.h"
#include <string>
#include <sstream>

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
			auto *field = m_message->get_record()->get_field<ValueT>(name);
			if (!field)
			{
				std::ostringstream oss;
				oss << "No field with name " << name << " exists with specified type.";
				throw value_error(oss.str());
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
