#include "ki/protocol/net/DMLSession.h"

namespace ki
{
namespace protocol
{
namespace net
{
	DMLSession::DMLSession(const uint16_t id, const dml::MessageManager& manager)
		: Session(id), m_manager(manager) {}

	void DMLSession::send_message(const dml::Message& message)
	{
		send_packet(false, 0, message);
	}

	void DMLSession::on_application_message(const PacketHeader& header)
	{
		const auto *message = m_manager.message_from_binary(m_data_stream);
		if (!message)
		{
			on_invalid_message();
			return;
		}

		// Are we sufficiently authenticated to handle this message?
		if (get_access_level() >= message->get_access_level())
			on_message(*message);
		else
			on_invalid_message();
		delete message;
	}
}
}
}
