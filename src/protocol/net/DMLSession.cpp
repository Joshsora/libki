#include "ki/protocol/net/DMLSession.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace net
{
	DMLSession::DMLSession(const uint16_t id, const dml::MessageManager& manager)
		: Session(id), m_manager(manager) {}

	const dml::MessageManager& DMLSession::get_manager() const
	{
		return m_manager;
	}

	void DMLSession::send_message(const dml::Message& message)
	{
		send_packet(false, 0, message);
	}

	void DMLSession::on_application_message(const PacketHeader& header)
	{
		// Attempt to create a Message instance from the data in the stream
		auto error_code = InvalidDMLMessageErrorCode::NONE;
		const dml::Message *message = nullptr;
		try
		{
			message = m_manager.message_from_binary(m_data_stream);
		}
		catch (parse_error &e)
		{
			switch (e.get_error_code())
			{
			case parse_error::INVALID_HEADER_DATA:
				error_code = InvalidDMLMessageErrorCode::INVALID_HEADER_DATA;
				break;
			case parse_error::INSUFFICIENT_MESSAGE_DATA:
			case parse_error::INVALID_MESSAGE_DATA:
				error_code = InvalidDMLMessageErrorCode::INVALID_MESSAGE_DATA;
				break;
			default:
				error_code = InvalidDMLMessageErrorCode::UNKNOWN;
			}
		}
		catch (value_error &e)
		{
			switch (e.get_error_code())
			{
			case value_error::DML_INVALID_SERVICE:
				error_code = InvalidDMLMessageErrorCode::INVALID_SERVICE;
				break;
			case value_error::DML_INVALID_MESSAGE_TYPE:
				error_code = InvalidDMLMessageErrorCode::INVALID_MESSAGE_TYPE;
				break;
			default:
				error_code = InvalidDMLMessageErrorCode::UNKNOWN;
			}
		}

		if (!message)
		{
			on_invalid_message(error_code);
			return;
		}

		// Are we sufficiently authenticated to handle this message?
		if (get_access_level() >= message->get_access_level())
			on_message(message);
		else
			on_invalid_message(InvalidDMLMessageErrorCode::INSUFFICIENT_ACCESS);

		// WARNING: Refactor, and utilize smart pointers to avoid memory leaks in DML messages.
		// The following line breaks asynchronous code.
		// delete message;
	}
}
}
}
