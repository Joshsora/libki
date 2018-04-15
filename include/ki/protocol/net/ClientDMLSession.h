#pragma once
#include "ClientSession.h"
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
	class ClientDMLSession : public ClientSession, public DMLSession
	{
		// Explicitly specify that we are intentionally inheritting
		// via dominance.
		using DMLSession::on_application_message;
		using ClientSession::on_control_message;
		using ClientSession::is_alive;
	public:
		ClientDMLSession(const uint16_t id, const dml::MessageManager &manager)
			: Session(id), ClientSession(id), DMLSession(id, manager) {}
		virtual ~ClientDMLSession() = default;
	};
}
}
}

// Re-enable inheritance via dominance warning
#if _MSC_VER
#pragma warning(default: 4250)
#endif
