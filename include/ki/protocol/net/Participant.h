#pragma once
#include <cstdint>
#include <sstream>

#define KI_DEFAULT_MAXIMUM_RECEIVE_SIZE 0x2000
#define KI_START_SIGNAL 0xF00D

namespace ki
{
namespace protocol
{
namespace net
{
	enum class ReceiveState
	{
		// Waiting for the 0xF00D start signal.
		WAITING_FOR_START_SIGNAL,

		// Waiting for the 2-byte length.
		WAITING_FOR_LENGTH,

		// Waiting for the packet data.
		WAITING_FOR_PACKET
	};

	enum class ParticipantType
	{
		SERVER,
		CLIENT
	};

	/**
	 * This class implements the packet framing logic when
	 * sending and receiving data to/from an external source.
	 */
	class Participant
	{
	public:
		Participant(ParticipantType type);
		virtual ~Participant() = default;

		ParticipantType get_type() const;
		void set_type(ParticipantType type);

		uint16_t get_maximum_packet_size() const;
		void set_maximum_packet_size(uint16_t maximum_packet_size);
	protected:
		std::stringstream m_data_stream;

		/**
		* Frames raw data into a Packet, and transmits it.
		*/
		void send_data(const char *data, size_t size);

		/**
		* Process incoming raw data into Packets.
		* Once a packet is read into the internal data
		* stream, handle_packet_available is called.
		*/
		void process_data(const char *data, size_t size);

		virtual void close() = 0;
	private:
		ParticipantType m_type;
		uint16_t m_maximum_packet_size;

		ReceiveState m_receive_state;
		uint16_t m_start_signal;
		uint16_t m_incoming_packet_size;
		uint8_t m_shift;

		virtual void send_packet_data(const char *data, const size_t size) = 0;
		virtual void on_packet_available() {};
	};
}
}
}
