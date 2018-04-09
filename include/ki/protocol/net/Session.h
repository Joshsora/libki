#pragma once
#include "Participant.h"
#include "PacketHeader.h"
#include "ki/protocol/control/Opcode.h"
#include "../../util/Serializable.h"
#include <cstdint>
#include <chrono>
#include <type_traits>

namespace ki
{
namespace protocol
{
namespace net
{
	/**
	 * This class implements session logic on top of the
	 * low-level Participant class.
	 */
	class Session : public Participant
	{
	public:
		Session(ParticipantType type, uint16_t id);

		uint16_t get_id() const;
		bool is_established() const;

		uint8_t get_access_level() const;
		void set_access_level(uint8_t access_level);

		uint16_t get_latency() const;

		bool is_alive() const;
	protected:
		template <typename DataT>
		void send_packet(const bool is_control, const control::Opcode opcode,
			const DataT &data)
		{
			static_assert(std::is_base_of<util::Serializable, DataT>::value,
				"DataT must inherit Serializable.");

			std::ostringstream ss;
			PacketHeader header(is_control, (uint8_t)opcode);
			header.write_to(ss);
			data.write_to(ss);

			const auto buffer = ss.str();
			send_data(buffer.c_str(), buffer.length());
		}

		template <typename DataT>
		DataT read_data()
		{
			static_assert(std::is_base_of<util::Serializable, DataT>::value,
				"DataT must inherit Serializable.");
			
			DataT data = DataT();
			data.read_from(m_data_stream);
			return data;
		}

		void on_connected();
		virtual void on_established() {};
		virtual void on_application_message(const PacketHeader &header) {};
		virtual void on_invalid_packet() {};
	private:
		uint16_t m_id;
		bool m_established;
		uint8_t m_access_level;
		uint16_t m_latency;

		std::chrono::steady_clock::time_point m_creation_time;
		std::chrono::steady_clock::time_point m_establish_time;
		std::chrono::steady_clock::time_point m_last_heartbeat;

		void on_packet_available() override final;
		void on_control_message(const PacketHeader &header);
		void on_server_hello();
		void on_client_hello();
		void on_ping();
		void on_ping_response();

		void on_hello(uint16_t session_id, uint32_t timestamp,
			uint16_t milliseconds);
	};
}
}
}
