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
	class ServerKeepAlive final : public util::Serializable
	{
	public:
		ServerKeepAlive(uint32_t timestamp = 0);
		virtual ~ServerKeepAlive() = default;

		uint32_t get_timestamp() const;
		void set_timestamp(uint32_t timestamp);

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		uint32_t m_timestamp;
	};
}
}
}
