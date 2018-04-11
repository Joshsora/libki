#include "ki/protocol/net/DMLSession.h"

namespace ki
{
namespace protocol
{
namespace net
{
	DMLSession::DMLSession(const ParticipantType type, const  uint16_t id,
		const dml::MessageManager& manager)
		: Session(type, id), m_manager(manager) {}

	void DMLSession::on_application_message(const PacketHeader& header)
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
