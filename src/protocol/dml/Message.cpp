#include "ki/protocol/dml/Message.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	Message::Message(uint8_t service_id, uint8_t type)
	{
		m_service_id = service_id;
		m_type = type;
		m_record = nullptr;
	}

	Message::~Message()
	{
		delete m_record;
	}

	uint8_t Message::get_service_id() const
	{
		return m_service_id;
	}

	void Message::set_service_id(uint8_t service_id)
	{
		m_service_id = service_id;
	}

	uint8_t Message::get_type() const
	{
		return m_type;
	}

	void Message::set_type(uint8_t type)
	{
		m_type = type;
	}

	ki::dml::Record *Message::get_record()
	{
		return m_record;
	}

	const ki::dml::Record *Message::get_record() const
	{
		return m_record;
	}

	void Message::set_record(const ki::dml::Record &record)
	{
		m_record = new ki::dml::Record(record);
	}

	void Message::use_template_record(const ki::dml::Record &record)
	{
		set_record(record);
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
				m_record = nullptr;

				std::ostringstream oss;
				oss << "Error reading DML message payload: " << e.what();
				throw parse_error(oss.str());
			}
		}
	}

	void Message::write_to(std::ostream &ostream) const
	{
		ki::dml::Record record;
		record.add_field<ki::dml::UBYT>("m_service_id")->set_value(m_service_id);
		record.add_field<ki::dml::UBYT>("m_type")->set_value(m_type);
		auto *size_field = record.add_field<ki::dml::USHRT>("size");
		if (m_record)
			size_field->set_value(m_record->get_size() + 4);
		else
			size_field->set_value(m_raw_data.size() + 4);
		record.write_to(ostream);

		if (m_record)
			record.write_to(ostream);
		else
			ostream.write(m_raw_data.data(), m_raw_data.size());
	}

	void Message::read_from(std::istream &istream)
	{
		ki::dml::Record record;
		auto *service_id_field = record.add_field<ki::dml::UBYT>("ServiceID");
		auto *message_type_field = record.add_field<ki::dml::UBYT>("MsgType");
		auto *size_field = record.add_field<ki::dml::USHRT>("Length");
		try
		{
			record.read_from(istream);
		}
		catch (ki::dml::parse_error &e)
		{
			std::ostringstream oss;
			oss << "Error reading DML message header: " << e.what();
			throw parse_error(oss.str());
		}

		m_service_id = service_id_field->get_value();
		m_type = message_type_field->get_value();
		const ki::dml::USHRT size = size_field->get_value() - 4;
		m_raw_data.resize(size);
		istream.read(m_raw_data.data(), size);
		if (istream.fail())
			throw parse_error("Not enough data was available to read DML message payload.");
	}

	size_t Message::get_size() const
	{
		if (m_record)
			return 4 + m_record->get_size();
		return 4 + m_raw_data.size();
	}
}
}
}