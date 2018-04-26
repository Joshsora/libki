#pragma once
#include "Session.h"

#define KI_CLIENT_HEARTBEAT 10

namespace ki
{
namespace protocol
{
namespace net
{
	/**
	 * Implements server-sided session logic.
	 */
	class ServerSession : public virtual Session
	{
	public:
		explicit ServerSession(uint16_t id);
		virtual ~ServerSession() = default;

		void send_keep_alive(uint32_t milliseconds_since_startup);
		bool is_alive() const override;
	protected:
		void on_connected();
		virtual void on_established() {}
		void on_control_message(const PacketHeader& header) override;
	private:
		void on_session_accept();
		void on_keep_alive();
		void on_keep_alive_response();
	};
}
}
}