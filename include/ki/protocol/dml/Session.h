#pragma once
#include "../net/Session.h"
#include "MessageManager.h"

namespace ki
{
namespace protocol
{
namespace dml
{
	class Session : public net::Session
	{
	public:
		Session(net::ParticipantType type, uint16_t id,
			const MessageManager &manager);
		~Session() = default;
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