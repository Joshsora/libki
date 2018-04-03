#pragma once
#include "../util/Serializable.h"
#include <cstdint>
#include <vector>
#include <sstream>
#include <type_traits>

namespace ki
{
namespace protocol
{
	class Packet final : public util::Serializable
	{
	public:
		Packet(bool control = false, uint8_t opcode = 0);
		virtual ~Packet() = default;

		bool is_control() const;
		void set_control(bool control);

		uint8_t get_opcode() const;
		void set_opcode(uint8_t opcode);

		template <typename DataT>
		void set_payload_data(const DataT &data)
		{
			static_assert(std::is_base_of<Serializable, DataT>::value,
				"DataT must derive from Serializable.");

			std::ostringstream oss;
			data.write_to(oss);
			std::string data_string = oss.str();
			m_payload.assign(data_string.begin(), data_string.end());
		}

		template <typename DataT>
		DataT *get_payload_data() const
		{
			static_assert(std::is_base_of<Serializable, DataT>::value,
				"DataT must derive from Serializable.");

			std::istringstream iss(std::string(m_payload.data(), m_payload.size()));
			DataT *data = new DataT();
			data->read_from(iss);
			return data;
		}

		void write_to(std::ostream &ostream) const override final;
		void read_from(std::istream &istream) override final;
		size_t get_size() const override final;
	private:
		bool m_control;
		uint8_t m_opcode;
		std::vector<char> m_payload;
	};
}
}
