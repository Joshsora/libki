#pragma once
#include "MessageModule.h"
#include "MessageBuilder.h"
#include "../../dml/Record.h"
#include <string>

namespace ki
{
namespace protocol
{
namespace dml
{
	class MessageManager
	{
	public:
		MessageManager();
		~MessageManager();

		static MessageManager &get_singleton();

		const MessageModule &load_module(std::string filepath);
		const MessageModule &get_module(uint8_t service_id) const;
		const MessageModule &get_module(std::string protocol_type) const;

		MessageBuilder &build_message(uint8_t service_id, uint8_t message_type) const;
		MessageBuilder &build_message(uint8_t service_id, std::string message_name) const;
		MessageBuilder &build_message(std::string service_type, uint8_t message_type) const;
		MessageBuilder &build_message(std::string service_type, std::string message_name) const;

		const Message *from_binary(std::istream &istream);
	private:
		static MessageManager *g_instance;

		MessageModuleList m_modules;
		MessageModuleServiceIdMap m_service_id_map;
		MessageModuleProtocolTypeMap m_protocol_type_map;
	};
}
}
}
