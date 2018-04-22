#include "ki/protocol/dml/MessageModule.h"
#include "ki/protocol/exception.h"
#include <sstream>

namespace ki
{
namespace protocol
{
namespace dml
{
	MessageModule::MessageModule(uint8_t service_id, std::string protocol_type)
	{
		m_service_id = service_id;
		m_protocol_type = protocol_type;
		m_protocol_description = "";
		m_last_message_type = 0;
	}

	MessageModule::~MessageModule()
	{
		for (auto it = m_templates.begin();
			it != m_templates.end(); ++it)
			delete *it;
		m_message_type_map.clear();
		m_message_name_map.clear();
	}

	uint8_t MessageModule::get_service_id() const
	{
		return m_service_id;
	}

	void MessageModule::set_service_id(uint8_t service_id)
	{
		m_service_id = service_id;
	}

	std::string MessageModule::get_protocol_type() const
	{
		return m_protocol_type;
	}

	void MessageModule::set_protocol_type(std::string protocol_type)
	{
		m_protocol_type = protocol_type;
	}

	std::string MessageModule::get_protocol_desription() const
	{
		return m_protocol_description;
	}

	void MessageModule::set_protocol_description(std::string protocol_description)
	{
		m_protocol_description = protocol_description;
	}

	const MessageTemplate *MessageModule::add_message_template(std::string name,
		ki::dml::Record *record, bool auto_sort)
	{
		if (!record)
			return nullptr;

		// If the field exists, get the name from the record rather than the XML
		auto *name_field = record->get_field<ki::dml::STR>("_MsgName");
		if (name_field) 
			name = name_field->get_value();

		// Do we already have a message template with this name?
		if (m_message_name_map.count(name) == 1)
			return m_message_name_map.at(name);

		// Message type is based on the _MsgOrder field if it's present
		// Otherwise it's based on the alphabetical order of template names
		uint8_t message_type = 0;
		auto *order_field = record->get_field<ki::dml::UBYT>("_MsgOrder");
		if (order_field)
		{
			message_type = order_field->get_value();

			// Don't allow message type to be 0
			if (message_type == 0)
				return nullptr;

			// Do we already have a template with this type?
			if (m_message_type_map.count(message_type) == 1)
				return nullptr;
		}

		// Create the message template and add it to our lookups
		auto *message_template = new MessageTemplate(name, message_type, m_service_id, record);
		m_templates.push_back(message_template);
		m_message_name_map.insert({ name, message_template });

		// Is this module ordered?
		if (message_type != 0)
			m_message_type_map.insert({ message_type, message_template });
		else if (auto_sort)
			sort_lookup();

		return message_template;
	}

	const MessageTemplate *MessageModule::get_message_template(uint8_t type) const
	{
		if (m_message_type_map.count(type) == 1)
			return m_message_type_map.at(type);
		return nullptr;
	}

	const MessageTemplate *MessageModule::get_message_template(std::string name) const
	{
		if (m_message_name_map.count(name) == 1)
			return m_message_name_map.at(name);
		return nullptr;
	}

	void MessageModule::sort_lookup()
	{
		uint8_t message_type = 1;

		// First, clear the message type map since we're going to be
		// moving everything around
		m_message_type_map.clear();

		// Iterating over a map with std::string as the key
		// is guaranteed to be in alphabetical order
		for (auto it = m_message_name_map.begin();
			it != m_message_name_map.end(); ++it)
		{
			auto *message_template = it->second;
			message_template->set_type(message_type);
			m_message_type_map.insert({ message_type, message_template });
			message_type++;

			// Make sure we haven't overflowed
			if (message_type == 0)
				throw value_error("Module has more than 254 messages.", value_error::EXCEEDS_LIMIT);
		}
	}

	Message *MessageModule::create_message(uint8_t message_type) const
	{
		auto *message_template = get_message_template(message_type);
		if (!message_template)
		{
			std::ostringstream oss;
			oss << "No message exists with type: " << message_type;
			oss << "(service=" << m_protocol_type << ")";
			throw value_error(oss.str(), value_error::DML_INVALID_MESSAGE_TYPE);
		}

		return message_template->create_message();
	}

	Message *MessageModule::create_message(std::string message_name) const
	{
		auto *message_template = get_message_template(message_name);
		if (!message_template)
		{
			std::ostringstream oss;
			oss << "No message exists with name: " << message_name;
			oss << "(service=" << m_protocol_type << ")";
			throw value_error(oss.str(), value_error::DML_INVALID_MESSAGE_NAME);
		}

		return message_template->create_message();
	}
}
}
}