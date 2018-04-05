#pragma once
#include "MessageTemplate.h"
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <map>

namespace ki
{
namespace protocol
{
namespace dml
{
	class MessageModule
	{
	public:
		MessageModule(uint8_t service_id = 0, std::string protocol_type = "");
		~MessageModule();

		uint8_t get_service_id() const;
		void set_service_id(uint8_t service_id);

		std::string get_protocol_type() const;
		void set_protocol_type(std::string protocol_type);

		std::string get_protocol_desription() const;
		void set_protocol_description(std::string protocol_description);

		const MessageTemplate *add_message_template(std::string name, ki::dml::Record *record);
		const MessageTemplate *get_message_template(uint8_t type) const;
		const MessageTemplate *get_message_template(std::string name) const;

		MessageBuilder &build_message(uint8_t message_type) const;
		MessageBuilder &build_message(std::string message_name) const;
	private:
		uint8_t m_service_id;
		std::string m_protocol_type;
		std::string m_protocol_description;
		uint8_t m_last_message_type;

		std::array<MessageTemplate *, 255> m_templates;
		std::map<std::string, MessageTemplate *> m_message_name_map;
	};

	typedef std::vector<MessageModule *> MessageModuleList;
	typedef std::map<uint8_t, MessageModule *> MessageModuleServiceIdMap;
	typedef std::map<std::string, MessageModule *> MessageModuleProtocolTypeMap;
}
}
}
