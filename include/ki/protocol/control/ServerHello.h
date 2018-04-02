#pragma once
#include "../../util/Serializable.h"
#include "../Packet.h"
#include "Opcode.h"
#include <iostream>
#include <cstdint>

namespace ki
{
namespace protocol
{
namespace control
{
	class ServerHello : public util::Serializable
	{
	public:
		ServerHello(uint16_t session_id = 0,
			uint64_t timestamp = 0, uint32_t milliseconds = 0);
		virtual ~ServerHello() = default;

		uint16_t get_session_id() const;
		void set_session_id(uint16_t session_id);

		uint64_t get_timestamp() const;
		void set_timestamp(uint64_t timestamp);

		uint32_t get_milliseconds() const;
		void set_milliseconds(uint32_t milliseconds);

		void write_to(std::ostream &ostream) const final;
		void read_from(std::istream &istream) final;
		size_t get_size() const final;

		static Packet *create_packet(uint16_t session_id = 0,
			uint64_t timestamp = 0, uint32_t milliseconds = 0)
		{
			ServerHello data(session_id, timestamp, milliseconds);
			auto *packet = new Packet(true, (uint8_t)Opcode::SERVER_HELLO);
			packet->set_payload_data(data);
			return packet;
		}
	private:
		uint16_t m_session_id;
		uint64_t m_timestamp;
		uint32_t m_milliseconds;
	};
}
}
}
