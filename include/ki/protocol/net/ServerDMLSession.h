#pragma once
#include "ServerSession.h"
#include "DMLSession.h"

// Disable inheritance via dominance warning
#if _MSC_VER
#pragma warning(disable: 4250)
#endif

namespace ki
{
namespace protocol
{
namespace net
{
	class ServerDMLSession : public ServerSession, public DMLSession
	{
		// Explicitly specify that we are intentionally inheritting
		// via dominance.
		using DMLSession::on_application_message;
		using ServerSession::on_control_message;
		using ServerSession::is_alive;
	public:
		ServerDMLSession(const uint16_t id, const dml::MessageManager &manager)
			: Session(id), ServerSession(id), DMLSession(id, manager) {}
		virtual ~ServerDMLSession() = default;
	};
}
}
}

// Re-enable inheritance via dominance warning
#if _MSC_VER
#pragma warning(default: 4250)
#endif
