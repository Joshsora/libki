#include "ki/protocol/net/Participant.h"
#include "ki/protocol/exception.h"

namespace ki
{
namespace protocol
{
namespace net
{
	Participant::Participant(const ParticipantType type)
	{		
		m_type = type;
		m_maximum_packet_size = KI_DEFAULT_MAXIMUM_RECEIVE_SIZE;

		m_receive_state = ReceiveState::WAITING_FOR_START_SIGNAL;
		m_start_signal = 0;
		m_incoming_packet_size = 0;
		m_shift = 0;
	}

	ParticipantType Participant::get_type() const
	{
		return m_type;
	}

	void Participant::set_type(const ParticipantType type)
	{
		m_type = type;
	}


	uint16_t Participant::get_maximum_packet_size() const
	{
		return m_maximum_packet_size;
	}

	void Participant::set_maximum_packet_size(const uint16_t maximum_packet_size)
	{
		m_maximum_packet_size = maximum_packet_size;
	}

	void Participant::send_data(const char* data, const size_t size)
	{
		// Allocate the entire buffer
		char *packet_data = new char[size + 4];
		
		// Add the frame header
		((uint16_t *)packet_data)[0] = KI_START_SIGNAL;
		((uint16_t *)packet_data)[1] = size;

		// Copy the payload into the buffer and send it
		memcpy(&packet_data[4], data, size);
		send_packet_data(packet_data, size + 4);
		delete[] packet_data;
	}

	void Participant::process_data(const char *data, const size_t size)
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

}
}
}
