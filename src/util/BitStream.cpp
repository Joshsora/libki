#include "ki/util/BitStream.h"
#include "ki/util/exception.h"
#include <limits>
#include <cstring>
#include <cmath>

namespace ki
{
	BitStream::stream_pos::stream_pos(const intmax_t byte, const int bit)
	{
		m_byte = byte;
		set_bit(bit);
	}

	BitStream::stream_pos::stream_pos(const stream_pos& cp)
	{
		m_byte = cp.m_byte;
		set_bit(cp.m_bit);
	}

	intmax_t BitStream::stream_pos::as_bits() const
	{
		return (m_byte * 8) + m_bit;
	}

	intmax_t BitStream::stream_pos::get_byte() const
	{
		return m_byte;
	}

	uint8_t BitStream::stream_pos::get_bit() const
	{
		return m_bit;
	}

	void BitStream::stream_pos::set_bit(int bit)
	{
		if (bit < 0)
		{
			bit = -bit;
			m_byte -= (bit / 8) + 1;
			m_bit = 8 - (bit % 8);
		}
		else if (bit >= 8)
		{
			m_byte += bit / 8;
			m_bit = bit % 8;
		}
		else
			m_bit = bit;
	}

	BitStream::stream_pos BitStream::stream_pos::operator+(
		const stream_pos &rhs) const
	{
		return stream_pos(
			m_byte + rhs.m_byte, m_bit + rhs.m_bit
		);
	}

	BitStream::stream_pos BitStream::stream_pos::operator-(
		const stream_pos &rhs) const
	{
		return stream_pos(
			m_byte - rhs.m_byte, m_bit - rhs.m_bit
		);
	}

	BitStream::stream_pos BitStream::stream_pos::operator+(
		const int& rhs) const
	{
		return stream_pos(
			m_byte, m_bit + rhs
		);
	}

	BitStream::stream_pos BitStream::stream_pos::operator-(
		const int& rhs) const
	{
		return stream_pos(
			m_byte, m_bit - rhs
		);
	}

	BitStream::stream_pos& BitStream::stream_pos::operator+=(
		const stream_pos rhs)
	{
		m_byte += rhs.m_byte;
		set_bit(m_bit + rhs.m_bit);
		return *this;
	}

	BitStream::stream_pos& BitStream::stream_pos::operator-=(
		const stream_pos rhs)
	{
		m_byte -= rhs.m_byte;
		set_bit(m_bit - rhs.m_bit);
		return *this;
	}

	BitStream::stream_pos& BitStream::stream_pos::operator+=(const int bits)
	{
		set_bit(m_bit + bits);
		return *this;
	}

	BitStream::stream_pos& BitStream::stream_pos::operator-=(const int bits)
	{
		set_bit(m_bit - bits);
		return *this;
	}

	BitStream::stream_pos& BitStream::stream_pos::operator++()
	{
		set_bit(m_bit + 1);
		return *this;
	}

	BitStream::stream_pos& BitStream::stream_pos::operator--()
	{
		set_bit(m_bit - 1);
		return *this;
	}

	BitStream::stream_pos BitStream::stream_pos::operator++(int increment)
	{
		auto copy(*this);
		++(*this);
		return copy;
	}

	BitStream::stream_pos BitStream::stream_pos::operator--(int increment)
	{
		auto copy(*this);
		--(*this);
		return copy;
	}

	void BitStreamBase::write_copy(uint8_t *src, const std::size_t bitsize)
	{
		// Copy all whole bytes
		const auto bytes = bitsize / 8;
		auto written_bytes = 0;
		while (written_bytes < bytes)
		{
			write<uint8_t>(src[written_bytes]);
			written_bytes++;
		}

		// Copy left over bits
		const auto bits = bitsize % 8;
		if (bits > 0)
			write<uint8_t>(src[bytes + 1], bits);
	}

	void BitStreamBase::read_copy(uint8_t *dst, const std::size_t bitsize)
	{
		// Copy all whole bytes
		const auto bytes = bitsize / 8;
		auto read_bytes = 0;
		while (read_bytes < bytes)
		{
			dst[read_bytes] = read<uint8_t>();
			read_bytes++;
		}

		// Copy left over bits
		const auto bits = bitsize % 8;
		if (bits > 0)
			dst[bytes + 1] = read<uint8_t>(bits);
	}

	BitStream::BitStream(const size_t buffer_size)
	{
		m_buffer = new uint8_t[buffer_size] { 0 };
		m_buffer_size = buffer_size;
		m_position = stream_pos(0, 0);
	}

	BitStream::~BitStream()
	{
		delete[] m_buffer;
	}

	BitStream::stream_pos BitStream::tell() const
	{
		return m_position;
	}

	void BitStream::seek(const stream_pos position)
	{
		m_position = position;
		validate_buffer();
	}

	std::size_t BitStream::capacity() const
	{
		return m_buffer_size;
	}

	const uint8_t *BitStream::data() const
	{
		return m_buffer;
	}

	uint64_t BitStream::read(const uint8_t bits)
	{
		// Do we have these bits available to read?
		if ((m_position + bits).as_bits() > m_buffer_size * 8)
		{
			std::ostringstream oss;
			oss << "Not enough data in buffer to read "
				<< static_cast<uint16_t>(bits) << "bits.";
			throw runtime_error(oss.str());
		}

		// Iterate until we've read all of the bits
		uint64_t value = 0;
		auto unread_bits = bits;
		while (unread_bits > 0)
		{
			// Calculate how many bits to read from the current byte based on how many bits 
			// are left and how many bits we still need to read
			const uint8_t bits_available = (8 - m_position.get_bit());
			const auto bit_count = unread_bits < bits_available ? unread_bits : bits_available;

			// Find the bit-mask based on how many bits are being read
			const uint8_t bit_mask = (1 << bit_count) - 1 << m_position.get_bit();

			// Read the bits from the current byte and position them on the least-signficant bit
			const uint8_t bits_value = (m_buffer[m_position.get_byte()] & bit_mask) >> m_position.get_bit();

			// Position the value of the bits we just read based on how many bits of the value 
			// we've already read
			const uint8_t read_bits = bits - unread_bits;
			value |= static_cast<uint64_t>(bits_value) << read_bits;

			// Remove the bits we just read from the count of unread bits
			unread_bits -= bit_count;

			// Move forward the number of bits we just read
			seek(tell() + bit_count);
		}

		return value;
	}

	void BitStream::write(const uint64_t value, const uint8_t bits)
	{
		// Iterate until we've written all of the bits
		auto unwritten_bits = bits;
		while (unwritten_bits > 0)
		{
			// Calculate how many bits to write based on how many bits are left in the current byte
			// and how many bits from the value we still need to write
			const uint8_t bits_available = (8 - m_position.get_bit());
			const auto bit_count = unwritten_bits < bits_available ? unwritten_bits : bits_available;

			// Find the bit-mask based on how many bits are being written, and how many bits we've
			// already written
			const uint8_t written_bits = bits - unwritten_bits;
			const auto bit_mask = static_cast<uint64_t>((1 << bit_count) - 1) << written_bits;

			// Get the bits from the value and position them at the current bit position
			uint8_t value_byte = ((value & bit_mask) >> written_bits) & 0xFF;
			value_byte <<= m_position.get_bit();

			// Write the bits into the byte we're currently at
			m_buffer[m_position.get_byte()] |= value_byte;
			unwritten_bits -= bit_count;

			// Move forward the number of bits we just wrote
			seek(tell() + bit_count);
		}
	}

	void BitStream::expand_buffer()
	{
		// Work out a new buffer size
		const auto minimum_bits = static_cast<uint64_t>(
			std::log2(m_position.get_byte())
		) + 1;
		auto new_size = (2 << minimum_bits) + 2;
		if (new_size < m_buffer_size)
			new_size = std::numeric_limits<size_t>::max();

		// Has the buffer reached maximum size?
		if (new_size == m_buffer_size)
			throw runtime_error("Buffer cannot be expanded as it has reached maximum size.");

		// Allocate a new buffer, copy everything over, and then delete the old buffer
		auto *new_buffer = new uint8_t[new_size] { 0 };
		std::memcpy(new_buffer, m_buffer, m_buffer_size);
		delete[] m_buffer;
		m_buffer = new_buffer;
		m_buffer_size = new_size;
	}

	void BitStream::validate_buffer()
	{
		// Make sure we haven't underflowed
		if (m_position.get_byte() < 0)
			throw runtime_error("Position of buffer is less than 0!");

		// Expand the buffer if we've overflowed
		if (m_position.get_byte() >= m_buffer_size)
			expand_buffer();
	}
}
