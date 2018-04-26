#include "ki/protocol/dml/Message.h"
#include "ki/protocol/dml/MessageTemplate.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	Message::Message(const MessageTemplate *message_template)
	{
		m_template = message_template;
		if (m_template)
			m_record = new ki::dml::Record(m_template->get_record());
		else
			m_record = nullptr;
	}

	Message::~Message()
	{
		delete m_record;
	}

	const MessageTemplate *Message::get_template() const
	{
		return m_template;
	}

	void Message::set_template(const MessageTemplate *message_template)
	{
		m_template = message_template;
		if (!m_template)
			return;

		m_record = new ki::dml::Record(message_template->get_record());
		if (!m_raw_data.empty())
		{
			std::istringstream iss(std::string(m_raw_data.data(), m_raw_data.size()));
			try
			{
				m_record->read_from(iss);
				m_raw_data.clear();
			}
			catch (ki::dml::parse_error &e)
			{
				delete m_record;
				m_template = nullptr;
				m_record = nullptr;

				std::ostringstream oss;
				oss << "Error reading DML message payload: " << e.what();
				throw parse_error(oss.str(), parse_error::INVALID_MESSAGE_DATA);
			}
		}
	}

	uint8_t Message::get_service_id() const
	{
		if (m_template)
			return m_template->get_service_id();
		return m_header.get_service_id();
	}

	uint8_t Message::get_type() const
	{
		if (m_template)
			return m_template->get_type();
		return m_header.get_type();
	}

	uint16_t Message::get_message_size() const
	{
		if (m_record)
			return m_record->get_size();
		return m_raw_data.size();
	}

	std::string Message::get_handler() const
	{
		if (m_template)
			return m_template->get_handler();
		return "";
	}

	uint8_t Message::get_access_level() const
	{
		if (m_template)
			return m_template->get_access_level();
		return 0;
	}

	ki::dml::Record *Message::get_record()
	{
		return m_record;
	}

	const ki::dml::Record *Message::get_record() const
	{
		return m_record;
	}

	ki::dml::FieldBase* Message::get_field(std::string name)
	{
		if  (m_record)
			return m_record->get_field(name);
		return nullptr;
	}

	const ki::dml::FieldBase* Message::get_field(std::string name) const
	{
		if (m_record)
			return m_record->get_field(name);
		return nullptr;
	}

	void Message::write_to(std::ostream &ostream) const
	{
		// Write the header
		if (m_template)
		{
			MessageHeader header(
				get_service_id(), get_type(), get_message_size());
			header.write_to(ostream);
		}
		else
			m_header.write_to(ostream);

		// Write the payload
		if (m_record)
			m_record->write_to(ostream);
		else
			ostream.write(m_raw_data.data(), m_raw_data.size());
	}

	void Message::read_from(std::istream &istream)
	{
		m_header.read_from(istream);
		if (m_template)
		{
			// Check for mismatches between the header and template
			if (m_header.get_service_id() != m_template->get_service_id())
				throw value_error("ServiceID mismatch between MessageHeader and assigned template.",
					value_error::DML_INVALID_SERVICE);
			if (m_header.get_type() != m_template->get_type())
				throw value_error("Message Type mismatch between MessageHeader and assigned template.",
					value_error::DML_INVALID_MESSAGE_TYPE);
			
			// Read the payload into the record
			m_record->read_from(istream);
		}
		else
		{
			// We don't have a template for the record structure, so
			// just read the raw data into a buffer.
			const auto size = m_header.get_message_size();
			m_raw_data.resize(size);
			istream.read(m_raw_data.data(), size);
			if (istream.fail())
				throw parse_error("Not enough data was available to read DML message payload.",
					parse_error::INSUFFICIENT_MESSAGE_DATA);
		}
	}

	size_t Message::get_size() const
	{
		if (m_record)
			return m_header.get_size() + m_record->get_size();
		return 4 + m_raw_data.size();
	}
}
}
}