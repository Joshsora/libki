#include "ki/protocol/net/Session.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace net
{
	Session::Session(const uint16_t id)
	{
		m_id = id;
		m_established = false;
		m_access_level = 0;
		m_latency = 0;
		m_creation_time = std::chrono::steady_clock::now();
		m_waiting_for_keep_alive_response = false;

		m_maximum_packet_size = KI_DEFAULT_MAXIMUM_RECEIVE_SIZE;
		m_receive_state = ReceiveState::WAITING_FOR_START_SIGNAL;
		m_start_signal = 0;
		m_incoming_packet_size = 0;
		m_shift = 0;
	}

	uint16_t Session::get_maximum_packet_size() const
	{
		return m_maximum_packet_size;
	}

	void Session::set_maximum_packet_size(const uint16_t maximum_packet_size)
	{
		m_maximum_packet_size = maximum_packet_size;
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

	void Session::send_packet(const bool is_control, const uint8_t opcode,
		const util::Serializable& data)
	{
		std::ostringstream ss;
		PacketHeader header(is_control, opcode);
		header.write_to(ss);
		data.write_to(ss);

		const auto buffer = ss.str();
		send_data(buffer.c_str(), buffer.length());
	}

	void Session::send_data(const char* data, const size_t size)
	{
		// Allocate the entire buffer
		char *packet_data = new char[size + 4];

		// Add the frame header
		((uint16_t *)packet_data)[0] = KI_START_SIGNAL;
		((uint16_t *)packet_data)[1] = size;

		// Copy the payload into the buffer and send it
		std::memcpy(&packet_data[4], data, size);
		send_packet_data(packet_data, size + 4);
		delete[] packet_data;
	}

	void Session::process_data(const char *data, const size_t size)
	{
		size_t position = 0;
		while (position < size)
		{
			switch (m_receive_state)
			{
			case ReceiveState::WAITING_FOR_START_SIGNAL:
				m_start_signal |= ((uint8_t)data[position] << m_shift);
				if (m_shift == 0)
					m_shift = 8;
				else
				{
					// If the start signal isn't correct, we've either
					// gotten out of sync, or they are not framing packets
					// correctly.
					if (m_start_signal != KI_START_SIGNAL)
					{
						close();
						return;
					}

					// Reset the shift and incoming packet size
					m_shift = 0;
					m_incoming_packet_size = 0;
					m_receive_state = ReceiveState::WAITING_FOR_LENGTH;
				}
				position++;
				break;

			case ReceiveState::WAITING_FOR_LENGTH:
				m_incoming_packet_size |= ((uint8_t)data[position] << m_shift);
				if (m_shift == 0)
					m_shift = 8;
				else
				{
					// If the incoming packet is larger than we are accepting
					// stop processing data.
					if (m_incoming_packet_size > m_maximum_packet_size)
					{
						close();
						return;
					}

					// Reset read and write positions
					m_data_stream.seekp(0, std::ios::beg);
					m_data_stream.seekg(0, std::ios::beg);
					m_receive_state = ReceiveState::WAITING_FOR_PACKET;
				}
				position++;
				break;

			case ReceiveState::WAITING_FOR_PACKET:
				// Work out how much data we should read into our stream
				const size_t data_available = (size - position);
				const size_t read_size = (data_available >= m_incoming_packet_size) ?
					m_incoming_packet_size : data_available;

				//  Write the data to the data stream
				m_data_stream.write(&data[position], read_size);
				position += read_size;
				m_incoming_packet_size -= read_size;

				// Have we received the entire packet?
				if (m_incoming_packet_size == 0)
				{
					on_packet_available();

					// Reset the shift and start signal
					m_shift = 0;
					m_start_signal = 0;
					m_receive_state = ReceiveState::WAITING_FOR_START_SIGNAL;
				}
				break;
			}
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
}
}
}
