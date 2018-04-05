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
		MessageManager() = default;
		~MessageManager();

		const MessageModule *load_module(std::string filepath);
		const MessageModule *get_module(uint8_t service_id) const;
		const MessageModule *get_module(const std::string &protocol_type) const;

		MessageBuilder &build_message(uint8_t service_id, uint8_t message_type) const;
		MessageBuilder &build_message(uint8_t service_id, const std::string &message_name) const;
		MessageBuilder &build_message(const std::string &protocol_type, uint8_t message_type) const;
		MessageBuilder &build_message(const std::string &protocol_type, const std::string &message_name) const;

		/**
		 * If the DML message header cannot be read, then a nullptr
		 * is returned; otherwise, a valid Message pointer is always returned.
		 * However, that does not mean that the message itself is valid.
		 * 
		 * To verify if the record was completely parsed, get_record 
		 * should return a valid Record pointer, rather than nullptr.
		 */
		const Message *message_from_binary(std::istream &istream) const;
	private:
		MessageModuleList m_modules;
		MessageModuleServiceIdMap m_service_id_map;
		MessageModuleProtocolTypeMap m_protocol_type_map;
	};
}
}
}
