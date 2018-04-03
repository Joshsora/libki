#pragma once
#include "../../util/Serializable.h"
#include "../../dml/Record.h"
#include <iostream>

namespace ki
{
namespace protocol
{
namespace dml
{
	class Message final : public util::Serializable
	{
	public:
		Message(uint8_t service_id = 0, uint8_t type = 0);
		virtual ~Message() = default;

		uint8_t get_service_id() const;
		void set_service_id(uint8_t service_id);

		uint8_t get_type() const;
		void set_type(uint8_t type);

		ki::dml::Record &get_record();
		const ki::dml::Record &get_record() const;
		void set_record(const ki::dml::Record &record);

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		uint8_t m_service_id;
		uint8_t m_type;
		ki::dml::Record *m_record;
	};
}
}
}