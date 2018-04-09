#pragma once
#include "../../util/Serializable.h"
#include <iostream>
#include <cstdint>

namespace ki
{
namespace protocol
{
namespace control
{
	class ServerHello final : public util::Serializable
	{
	public:
		ServerHello(uint16_t session_id = 0,
			int32_t timestamp = 0, uint32_t milliseconds = 0);
		virtual ~ServerHello() = default;

		uint16_t get_session_id() const;
		void set_session_id(uint16_t session_id);

		int32_t get_timestamp() const;
		void set_timestamp(int32_t timestamp);

		uint32_t get_milliseconds() const;
		void set_milliseconds(uint32_t milliseconds);

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		uint16_t m_session_id;
		int32_t m_timestamp;
		uint32_t m_milliseconds;
	};
}
}
}
