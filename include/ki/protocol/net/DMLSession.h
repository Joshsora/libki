#pragma once
#include "Session.h"
#include "../dml/MessageManager.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	class DMLSession : public net::Session
	{
	public:
		DMLSession(net::ParticipantType type, uint16_t id,
			const MessageManager &manager);
		~DMLSession() = default;
	protected:
		void on_application_message(const net::PacketHeader& header) override;
		virtual void on_message(const Message &message) {}
		virtual void on_invalid_message() {}
	private:
		const MessageManager &m_manager;
	};
}
}
}