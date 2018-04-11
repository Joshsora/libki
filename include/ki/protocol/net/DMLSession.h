#pragma once
#include "Session.h"
#include "../dml/MessageManager.h"

namespace ki
{
namespace protocol
{
namespace net
{
	class DMLSession : public Session
	{
	public:
		DMLSession(ParticipantType type, uint16_t id,
			const dml::MessageManager &manager);
		~DMLSession() = default;
	protected:
		void on_application_message(const PacketHeader& header) override;
		virtual void on_message(const dml::Message &message) {}
		virtual void on_invalid_message() {}
	private:
		const dml::MessageManager &m_manager;
	};
}
}
}