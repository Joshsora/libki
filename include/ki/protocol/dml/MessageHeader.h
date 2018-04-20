#pragma once
#include "../../util/Serializable.h"
#include <iostream>

namespace ki
{
namespace protocol
{
namespace dml
{
	class MessageHeader : public util::Serializable
	{
	public:
		MessageHeader(uint8_t service_id = 0,
			uint8_t type = 0, uint16_t size = 0);
		virtual ~MessageHeader() = default;

		uint8_t get_service_id() const;
		void set_service_id(uint8_t service_id);

		uint8_t get_type() const;
		void set_type(uint8_t type);

		uint16_t get_message_size() const;
		void set_message_size(uint16_t size);

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		uint8_t m_service_id;
		uint8_t m_type;
		uint16_t m_size;
	};
}
}
}