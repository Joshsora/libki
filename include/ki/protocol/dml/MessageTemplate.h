#pragma once
#include "../../dml/Record.h"
#include "MessageBuilder.h"
#include <string>

namespace ki
{
namespace protocol
{
namespace dml
{
	class MessageTemplate
	{
	public:
		MessageTemplate(std::string name, uint8_t type,
			uint8_t service_id, ki::dml::Record *record);
		~MessageTemplate();

		std::string get_name() const;
		void set_name(std::string name);

		uint8_t get_type() const;
		void set_type(uint8_t type);

		uint8_t get_service_id() const;
		void set_service_id(uint8_t service_id);

		const ki::dml::Record &get_record() const;
		void set_record(ki::dml::Record *record);

		MessageBuilder &build_message() const;
	private:
		std::string m_name;
		uint8_t m_type;
		uint8_t m_service_id;
		ki::dml::Record *m_record;
	};
}
}
}