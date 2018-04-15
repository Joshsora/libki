#pragma once
#include "Session.h"

#define KI_SERVER_HEARTBEAT 60

namespace ki
{
namespace protocol
{
namespace net
{
	/**
	 * Implements client-sided session logic.
	 */
	class ClientSession : public virtual Session
	{
	public:
		explicit ClientSession(uint16_t id);
		virtual ~ClientSession() = default;

		void send_keep_alive();
		bool is_alive() const override;
	protected:
		void on_connected();
		virtual void on_established() {}
		void on_control_message(const PacketHeader& header) override;
	private:
		void on_session_offer();
		void on_keep_alive();
		void on_keep_alive_response();
	};
}
}
}