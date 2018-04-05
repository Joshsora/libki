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
		virtual ~Message();

		uint8_t get_service_id() const;
		void set_service_id(uint8_t service_id);

		uint8_t get_type() const;
		void set_type(uint8_t type);

		ki::dml::Record *get_record();
		const ki::dml::Record *get_record() const;

		/**
		 * Sets the record to a copy of the specified record.
		 */
		void set_record(const ki::dml::Record &record);

		/**
		 * If raw data is present, then this uses the specified record 
		 * to parse the raw DML message payload into a new Record.
		 * If raw data is not present, this is equivalent to set_record.
		 * 
		 * If the raw data is parsed successfully, the internal raw 
		 * data is cleared, and calls to get_record will return a valid
		 * Record pointer.
		 * 
		 * However, if the raw data is not parsed successfully, then
		 * calls to get_record will still return nullptr.
		 */
		void use_template_record(const ki::dml::Record &record);

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		uint8_t m_service_id;
		uint8_t m_type;
		std::vector<char> m_raw_data;
		ki::dml::Record *m_record;
	};
}
}
}