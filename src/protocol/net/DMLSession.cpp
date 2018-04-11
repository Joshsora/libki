#include "ki/protocol/net/DMLSession.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	DMLSession::DMLSession(const net::ParticipantType type, const  uint16_t id,
		const MessageManager& manager)
		: net::Session(type, id), m_manager(manager) {}

	void DMLSession::on_application_message(const net::PacketHeader& header)
	{
		const auto *message = m_manager.message_from_binary(m_data_stream);
		if (!message)
		{
			on_invalid_message();
			return;
		}

		on_message(*message);
		delete message;
	}
}
}
}
