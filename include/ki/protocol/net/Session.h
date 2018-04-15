#pragma once
#include "PacketHeader.h"
#include "../control/Opcode.h"
#include "../../util/Serializable.h"
#include <cstdint>
#include <sstream>
#include <chrono>
#include <type_traits>

#define KI_DEFAULT_MAXIMUM_RECEIVE_SIZE 0x2000
#define KI_START_SIGNAL 0xF00D
#define KI_CONNECTION_TIMEOUT 3

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

	/**
	 * This class implements session and packet framing logic
	 * when sending and receiving data to/from an external
	 * source.
	 */
	class Session
	{
	public:
		explicit Session(uint16_t id = 0);
		virtual ~Session() = default;

		uint16_t get_maximum_packet_size() const;
		void set_maximum_packet_size(uint16_t maximum_packet_size);

		uint16_t get_id() const;
		bool is_established() const;

		uint8_t get_access_level() const;
		void set_access_level(uint8_t access_level);

		uint16_t get_latency() const;

		virtual bool is_alive() const = 0;

		void send_packet(bool is_control, uint8_t opcode,
			const util::Serializable &data);
	protected:
		/* Higher-level session members */
		uint16_t m_id;
		bool m_established;
		uint8_t m_access_level;

		/* Timing members */
		std::chrono::steady_clock::time_point m_creation_time;
		std::chrono::steady_clock::time_point m_connection_time;
		std::chrono::steady_clock::time_point m_establish_time;
		std::chrono::steady_clock::time_point m_last_received_heartbeat_time;
		std::chrono::steady_clock::time_point m_last_sent_heartbeat_time;
		bool m_waiting_for_keep_alive_response;
		uint16_t m_latency;

		// The packet data stream
		std::stringstream m_data_stream;

		/**
		 * Reads a serializable structure from the data stream.
		 */
		template <typename DataT>
		DataT read_data()
		{
			static_assert(std::is_base_of<util::Serializable, DataT>::value,
				"DataT must inherit Serializable.");

			DataT data = DataT();
			data.read_from(m_data_stream);
			return data;
		}

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

		/* Event handlers */
		virtual void on_invalid_packet() {}
		virtual void on_control_message(const PacketHeader &header) {}
		virtual void on_application_message(const PacketHeader &header) {}

		/* Low-level socket methods */
		virtual void send_packet_data(const char *data, const size_t size) = 0;
		virtual void close() = 0;
	private:
		/* Low-level networking members */
		uint16_t m_maximum_packet_size;
		ReceiveState m_receive_state;
		uint16_t m_start_signal;
		uint16_t m_incoming_packet_size;
		uint8_t m_shift;

		void on_packet_available();
	};
}
}
}
