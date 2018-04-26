#include "ki/protocol/dml/MessageManager.h"
#include "ki/protocol/dml/MessageHeader.h"
#include "ki/protocol/exception.h"
#include "ki/dml/Record.h"
#include "ki/util/ValueBytes.h"
#include <fstream>
#include <sstream>
#include <rapidxml.hpp>

namespace ki
{
namespace protocol
{
namespace dml
{
	MessageManager::~MessageManager()
	{
		for (auto it = m_modules.begin();
			it != m_modules.end(); ++it)
			delete *it;
		m_modules.clear();
		m_service_id_map.clear();
		m_protocol_type_map.clear();
	}

	const MessageModule *MessageManager::load_module(std::string filepath)
	{
		// Open the file
		std::ifstream ifs(filepath, std::ios::ate);
		if (!ifs.is_open())
		{
			std::ostringstream oss;
			oss << "Could not open file: " << filepath;
			throw value_error(oss.str(), value_error::MISSING_FILE);
		}

		// Load contents into memory
		size_t size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		char *data = new char[size + 1] { 0 };
		ifs.read(data, size);

		// Parse the contents
		rapidxml::xml_document<> doc;
		try
		{
			doc.parse<0>(data);
		}
		catch (rapidxml::parse_error &e)
		{
			delete[] data;

			std::ostringstream oss;
			oss << "Failed to parse: " << filepath;
			throw parse_error(oss.str(), parse_error::INVALID_XML_DATA);
		}

		// It's safe to allocate the module we're working on now
		auto *message_module = new MessageModule();

		// Get the root node and iterate through children
		// Each child is a MessageTemplate
		auto *root = doc.first_node();
		for (auto *node = root->first_node();
			node; node = node->next_sibling())
		{
			// Parse the record node inside this node
			auto *record_node = node->first_node();
			if (!record_node)
				continue;
			auto *record = new ki::dml::Record();
			record->from_xml(record_node);

			// The message name is initially based on the element name
			const std::string message_name = node->name();
			if (message_name == "_ProtocolInfo")
			{
				auto *service_id_field = record->get_field<ki::dml::UBYT>("ServiceID");
				auto *type_field = record->get_field<ki::dml::STR>("ProtocolType");
				auto *description_field = record->get_field<ki::dml::STR>("ProtocolDescription");

				// Set the module metadata from this template
				if (service_id_field)
					message_module->set_service_id(service_id_field->get_value());
				if (type_field)
					message_module->set_protocol_type(type_field->get_value());
				if (description_field)
					message_module->set_protocol_description(description_field->get_value());
			}
			else
			{
				// Only do sorting after we've reached the final message
				// This only affects modules that aren't ordered with _MsgOrder.
				const bool auto_sort = node->next_sibling() == nullptr;

				// The template will use the record itself to figure out name and type;
				// we only give the XML data incase the record doesn't have it defined.
				auto *message_template = message_module->add_message_template(message_name, record, auto_sort);
				if (!message_template)
				{
					delete[] data;
					delete message_module;
					delete record;

					std::ostringstream oss;
					oss << "Failed to create message template for ";
					oss << message_name;
					throw value_error(oss.str());
				}
			}
		}

		// Make sure we aren't overwriting another module
		if (m_service_id_map.count(message_module->get_service_id()) == 1)
		{
			delete[] data;
			delete message_module;

			std::ostringstream oss;
			oss << "Message Module has already been loaded with Service ID ";
			oss << (uint16_t)message_module->get_service_id();
			throw value_error(oss.str(), value_error::OVERWRITES_LOOKUP);
		}

		if (m_protocol_type_map.count(message_module->get_protocol_type()) == 1)
		{
			delete[] data;
			delete message_module;

			std::ostringstream oss;
			oss << "Message Module has already been loaded with Protocol Type ";
			oss << message_module->get_protocol_type();
			throw value_error(oss.str(), value_error::OVERWRITES_LOOKUP);
		}

		// Add it to our maps
		m_modules.push_back(message_module);
		m_service_id_map.insert({ message_module->get_service_id(), message_module });
		m_protocol_type_map.insert({ message_module->get_protocol_type(), message_module });

		delete[] data;
		return message_module;
	}

	const MessageModule *MessageManager::get_module(uint8_t service_id) const
	{
		if (m_service_id_map.count(service_id) == 1)
			return m_service_id_map.at(service_id);
		return nullptr;
	}

	const MessageModule *MessageManager::get_module(const std::string &protocol_type) const
	{
		if (m_protocol_type_map.count(protocol_type) == 1)
			return m_protocol_type_map.at(protocol_type);
		return nullptr;
	}

	Message *MessageManager::create_message(uint8_t service_id, uint8_t message_type) const
	{
		auto *message_module = get_module(service_id);
		if (!message_module)
		{
			std::ostringstream oss;
			oss << "No service exists with id: " << (uint16_t)service_id;
			throw value_error(oss.str(), value_error::DML_INVALID_SERVICE);
		}

		return message_module->create_message(message_type);
	}

	Message *MessageManager::create_message(uint8_t service_id, const std::string& message_name) const
	{
		auto *message_module = get_module(service_id);
		if (!message_module)
		{
			std::ostringstream oss;
			oss << "No service exists with id: " << (uint16_t)service_id;
			throw value_error(oss.str(), value_error::DML_INVALID_SERVICE);
		}

		return message_module->create_message(message_name);
	}

	Message *MessageManager::create_message(const std::string& protocol_type, uint8_t message_type) const
	{
		auto *message_module = get_module(protocol_type);
		if (!message_module)
		{
			std::ostringstream oss;
			oss << "No service exists with protocol type: " << protocol_type;
			throw value_error(oss.str(), value_error::DML_INVALID_PROTOCOL_TYPE);
		}

		return message_module->create_message(message_type);
	}

	Message *MessageManager::create_message(const std::string& protocol_type, const std::string& message_name) const
	{
		auto *message_module = get_module(protocol_type);
		if (!message_module)
		{
			std::ostringstream oss;
			oss << "No service exists with protocol type: " << protocol_type;
			throw value_error(oss.str(), value_error::DML_INVALID_PROTOCOL_TYPE);
		}

		return message_module->create_message(message_name);
	}

	const Message *MessageManager::message_from_binary(std::istream& istream) const
	{
		// Read the message header
		MessageHeader header;
		header.read_from(istream);

		// Get the message module that uses the specified service id
		auto *message_module = get_module(header.get_service_id());
		if (!message_module)
		{
			std::ostringstream oss;
			oss << "No service exists with id: " << (uint16_t)header.get_service_id();
			throw value_error(oss.str(), value_error::DML_INVALID_SERVICE);
		}

		// Get the message template for this message type
		auto *message_template = message_module->get_message_template(header.get_type());
		if (!message_template)
		{
			std::ostringstream oss;
			oss << "No message exists with type: " << (uint16_t)header.get_service_id();
			oss << "(service=" << message_module->get_protocol_type() << ")";
			throw value_error(oss.str(), value_error::DML_INVALID_MESSAGE_TYPE);
		}

		// Make sure that the size specified is enough to read this message
		if (header.get_message_size() < message_template->get_record().get_size())
		{
			std::ostringstream oss;
			oss << "No message exists with type: " << (uint16_t)header.get_service_id();
			oss << "(service=" << message_module->get_protocol_type() << ")";
			throw value_error(oss.str(), value_error::DML_INVALID_MESSAGE_TYPE);
		}

		// Create a new Message from the template
		auto *message = new Message(message_template);
		try
		{
			message->get_record()->read_from(istream);
		}
		catch (ki::dml::parse_error &e)
		{
			delete message;
			throw parse_error("Failed to read DML message payload.", parse_error::INVALID_MESSAGE_DATA);
		}
		return message;
	}
}
}
}
