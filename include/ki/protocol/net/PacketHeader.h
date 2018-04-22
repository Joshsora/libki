#pragma once
#include "../../util/Serializable.h"
#include <cstdint>
#include <iostream>

namespace ki
{
namespace protocol
{
namespace net
{
	class PacketHeader final : public util::Serializable
	{
	public:
		PacketHeader(bool control = false, uint8_t opcode = 0);
		virtual ~PacketHeader() = default;

		bool is_control() const;
		void set_control(bool control);

		uint8_t get_opcode() const;
		void set_opcode(uint8_t opcode);

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		bool m_control;
		uint8_t m_opcode;
	};
}
}
}