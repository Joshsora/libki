#include "ki/protocol/net/ClientSession.h"
#include "ki/protocol/control/SessionOffer.h"
#include "ki/protocol/control/SessionAccept.h"
#include "ki/protocol/control/ClientKeepAlive.h"
#include "ki/protocol/control/ServerKeepAlive.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace net
{
	ClientSession::ClientSession(const uint16_t id)
		: Session(id) {}

	void ClientSession::send_keep_alive()
	{
		// Don't send a keep alive if we're waiting for a response
		if (m_waiting_for_keep_alive_response)
			return;
		m_waiting_for_keep_alive_response = true;

		// Work out how many minutes have been since the establish time, and
		// how many milliseconds we are in to the current minute.
		const auto time_since_establish = std::chrono::steady_clock::now() - m_establish_time;
		const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(time_since_establish);
		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			time_since_establish - minutes
		).count();

		// Send a KEEP_ALIVE packet
		control::ClientKeepAlive keep_alive(m_id, milliseconds, minutes.count());
		send_packet(true, (uint8_t)control::Opcode::KEEP_ALIVE, keep_alive);
		m_last_sent_heartbeat_time = std::chrono::steady_clock::now();
	}

	bool ClientSession::is_alive() const
	{
		// If the session isn't established yet, use the time of
		// creation to decide whether this session is alive.
		if (!m_established)
			return std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::steady_clock::now() - m_creation_time
			).count() <= (KI_CONNECTION_TIMEOUT * 2);

		// Otherwise, use the last time we received a heartbeat.
		return std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::steady_clock::now() - m_last_received_heartbeat_time
		).count() <= (KI_SERVER_HEARTBEAT * 2);
	}

	void ClientSession::on_connected()
	{
		m_connection_time = std::chrono::steady_clock::now();
	}

	void ClientSession::on_control_message(const PacketHeader& header)
	{
		switch ((control::Opcode)header.get_opcode())
		{
		case control::Opcode::SESSION_OFFER:
			on_session_offer();
			break;

		case control::Opcode::KEEP_ALIVE:
			on_keep_alive();
			break;

		case control::Opcode::KEEP_ALIVE_RSP:
			on_keep_alive_response();
			break;

		default:
			close(SessionCloseErrorCode::UNHANDLED_CONTROL_MESSAGE);
			break;
		}
	}

	void ClientSession::on_session_offer()
	{
		// Read the payload data into a structure
		control::SessionOffer offer;
		try
		{
			offer = read_data<control::SessionOffer>();
		}
		catch (parse_error &e)
		{
			// The SESSION_OFFER wasn't valid...
			// Close the session
			close(SessionCloseErrorCode::INVALID_MESSAGE);
			return;
		}

		// Should this session have already timed out?
		if (std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::steady_clock::now() - m_connection_time
			).count() > KI_CONNECTION_TIMEOUT)
		{
			close(SessionCloseErrorCode::SESSION_OFFER_TIMED_OUT);
			return;
		}

		// Work out the current timestamp and how many milliseconds 
		// have elapsed in the current second. 
		auto now = std::chrono::system_clock::now();
		const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
			now.time_since_epoch()
		).count();
		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()
		).count() - (timestamp * 1000);

		// Accept the session
		m_id = offer.get_session_id();
		control::SessionAccept accept(m_id, timestamp, milliseconds);
		send_packet(true, (uint8_t)control::Opcode::SESSION_ACCEPT, accept);

		// The session is successfully established
		m_established = true;
		m_establish_time = std::chrono::steady_clock::now();
		m_last_received_heartbeat_time = m_establish_time;
		on_established();
	}

	void ClientSession::on_keep_alive()
	{
		// Read the payload data into a structure
		control::ServerKeepAlive keep_alive;
		try
		{
			keep_alive = read_data<control::ServerKeepAlive>();
		}
		catch (parse_error &e)
		{
			// The KEEP_ALIVE wasn't valid...
			// Close the session
			close(SessionCloseErrorCode::INVALID_MESSAGE);
			return;
		}

		// Send the response
		m_last_received_heartbeat_time = std::chrono::steady_clock::now();
		send_packet(true, (uint8_t)control::Opcode::KEEP_ALIVE_RSP, keep_alive);
	}

	void ClientSession::on_keep_alive_response()
	{
		// Read the payload data into a structure
		try
		{
			// We don't actually need the data inside, but
			// read it to check if the structure is right.
			read_data<control::ClientKeepAlive>();
		}
		catch (parse_error &e)
		{
			// The KEEP_ALIVE_RSP wasn't valid...
			// Close the session
			close(SessionCloseErrorCode::INVALID_MESSAGE);
			return;
		}

		// Calculate latency and allow for KEEP_ALIVE packets to be sent again
		m_latency = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - m_last_sent_heartbeat_time
		).count();
		m_waiting_for_keep_alive_response = false;
	}
}
}
}
