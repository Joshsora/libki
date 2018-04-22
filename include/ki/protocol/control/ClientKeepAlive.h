#pragma once
#include "../../util/Serializable.h"
#include <cstdint>
#include <iostream>

namespace ki
{
namespace protocol
{
namespace control
{
	class ClientKeepAlive final : public util::Serializable
	{
	public:
		ClientKeepAlive(uint16_t session_id = 0,
			uint16_t milliseconds = 0, uint16_t minutes = 0);
		virtual ~ClientKeepAlive() = default;

		uint16_t get_session_id() const;
		void set_session_id(uint16_t session_id);

		uint16_t get_milliseconds() const;
		void set_milliseconds(uint16_t milliseconds);

		uint16_t get_minutes() const;
		void set_minutes(uint16_t minutes);

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		uint16_t m_session_id;
		uint16_t m_milliseconds;
		uint16_t m_minutes;
	};
}
}
}
