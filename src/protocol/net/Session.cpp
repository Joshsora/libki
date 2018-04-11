#include "ki/protocol/net/Session.h"
#include "ki/protocol/exception.h"
#include "ki/protocol/control/ServerHello.h"
#include "ki/protocol/control/ClientHello.h"
#include "ki/protocol/control/Ping.h"

namespace ki
{
namespace protocol
{
namespace net
{
	Session::Session(const ParticipantType type, const uint16_t id)
		: Participant(type)
	{
		m_id = id;
		m_established = false;
		m_access_level = 0;
		m_latency = 0;
		m_creation_time = std::chrono::steady_clock::now();
	}

	uint16_t Session::get_id() const
	{
		return m_id;
	}

	bool Session::is_established() const
	{
		return m_established;
	}

	uint8_t Session::get_access_level() const
	{
		return m_access_level;
	}

	void Session::set_access_level(const uint8_t access_level)
	{
		m_access_level = access_level;
	}

	uint16_t Session::get_latency() const
	{
		return m_latency;
	}

	bool Session::is_alive() const
	{
		// If the session isn't established yet, use the time of
		// creation to decide whether this session is alive.
		if (!m_established)
			return std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::steady_clock::now() - m_creation_time
			).count() <= 3;

		// Otherwise, use the last time we received a heartbeat.
		return std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::steady_clock::now() - m_last_heartbeat
		).count() <= 10;
	}

	void Session::send_packet(const bool is_control, const control::Opcode opcode,
		const util::Serializable& data)
	{
		std::ostringstream ss;
		PacketHeader header(is_control, (uint8_t)opcode);
		header.write_to(ss);
		data.write_to(ss);

		const auto buffer = ss.str();
		send_data(buffer.c_str(), buffer.length());
	}

	void Session::on_connected()
	{
		// If this is the server-side of a Session
		// we need to send SERVER_HELLO first.
		if (get_type() == ParticipantType::SERVER)
		{
			// Work out the current timestamp and how many milliseconds
			// have elapsed in the current second.
			auto now = std::chrono::system_clock::now();
			const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
				now.time_since_epoch()
			).count();
			const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
				now.time_since_epoch()
			).count() - (timestamp * 1000);

			// Send a SERVER_HELLO packet to the client
			const control::ServerHello hello(m_id, timestamp, milliseconds);
			send_packet(true, control::Opcode::SERVER_HELLO, hello);
		}
	}

	void Session::on_packet_available()
	{
		// Read the packet header
		PacketHeader header;
		try
		{
			header.read_from(m_data_stream);
		}
		catch (parse_error &e)
		{
			on_invalid_packet();
			return;
		}

		// Hand off to the right handler based on
		// whether this is a control packet or not
		if (header.is_control())
			on_control_message(header);
		else if (m_established)
			on_application_message(header);
		else
			close();
	}

	void Session::on_control_message(const PacketHeader& header)
	{
		switch ((control::Opcode)header.get_opcode())
		{
		case (control::Opcode::SERVER_HELLO):
			on_server_hello();
			break;

		case (control::Opcode::CLIENT_HELLO):
			on_client_hello();
			break;

		case (control::Opcode::PING):
			on_ping();
			break;

		case (control::Opcode::PING_RSP):
			on_ping_response();
			break;

		default:
			break;
		}
	}

	void Session::on_server_hello()
	{
		// If this is the server-side of a Session
		// we can't handle a SERVER_HELLO
		if (get_type() != ParticipantType::CLIENT)
		{
			close();
			return;
		}

		// Read the payload data into a structure
		try
		{
			// We've been given our id from the server now
			const auto server_hello = read_data<control::ServerHello>();
			m_id = server_hello.get_session_id();
			on_hello(m_id,
				server_hello.get_timestamp(),
				server_hello.get_milliseconds());

			// Work out the current timestamp and how many milliseconds
			// have elapsed in the current second.
			auto now = std::chrono::system_clock::now();
			const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
				now.time_since_epoch()
				).count();
			const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
				now.time_since_epoch()
				).count() - (timestamp * 1000);

			// Send a CLIENT_HELLO packet to the server
			const control::ClientHello hello(m_id, timestamp, milliseconds);
			send_packet(true, control::Opcode::CLIENT_HELLO, hello);
		}
		catch (parse_error &e)
		{
			// The CLIENT_HELLO wasn't valid...
			// Close the session
			close();
		}
	}

	void Session::on_client_hello()
	{
		// If this is the client-side of a Session
		// we can't handle a CLIENT_HELLO
		if (get_type() != ParticipantType::SERVER)
		{
			close();
			return;
		}

		// Read the payload data into a structure
		try
		{
			// The session is now established!
			const auto client_hello = read_data<control::ClientHello>();
			on_hello(client_hello.get_session_id(),
				client_hello.get_timestamp(),
				client_hello.get_milliseconds());
		}
		catch (parse_error &e)
		{
			// The CLIENT_HELLO wasn't valid...
			// Close the session
			close();
		}
	}

	void Session::on_ping()
	{
		// Read the payload data into a structure
		try
		{
			const auto ping = read_data<control::Ping>();
			if (get_type() == ParticipantType::SERVER)
			{
				// Calculate latency
				const auto send_time = m_establish_time +
					std::chrono::milliseconds(ping.get_milliseconds()) +
					std::chrono::minutes(ping.get_minutes());
				m_latency = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::steady_clock::now() - send_time
				).count();
			}

			// Send the response
			send_packet(true, control::Opcode::PING_RSP, ping);
		}
		catch (parse_error &e)
		{
			// The CLIENT_HELLO wasn't valid...
			// Close the session
			close();
		}
	}

	void Session::on_ping_response()
	{
		// Read the payload data into a structure
		try
		{
			const auto ping = read_data<control::Ping>();
		}
		catch (parse_error &e)
		{
			// The CLIENT_HELLO wasn't valid...
			// Close the session
			close();
		}
	}

	void Session::on_hello(const uint16_t session_id,
		const uint32_t timestamp, const uint16_t milliseconds)
	{
		// Make sure they're accepting this session
		if (session_id != m_id)
		{
			close();
			return;
		}

		// Calculate initial latency
		const std::chrono::system_clock::time_point epoch;
		const auto send_time = epoch + (std::chrono::seconds(timestamp) +
			std::chrono::milliseconds(milliseconds));
		m_latency = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now() - send_time
		).count();

		// The session is successfully established
		m_established = true;
		m_establish_time = std::chrono::steady_clock::now();
		m_last_heartbeat = m_establish_time;
		on_established();
	}
}
}
}
