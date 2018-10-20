#include "ki/util/BitStream.h"
#include <limits>
#include <exception>
#include <cstring>
#include <stdexcept>

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

	const uint8_t* BitStream::data() const
	{
		return m_buffer;
	}

	void BitStream::expand_buffer()
	{
		// Work out a new buffer size
		auto new_size = (m_buffer_size << 1) + 2;
		if (new_size < m_buffer_size)
			new_size = std::numeric_limits<size_t>::max();

		// Has the buffer reached maximum size?
		if (new_size == m_buffer_size)
			throw std::runtime_error("Buffer cannot be expanded as it has reached maximum size.");

		// Allocate a new buffer, copy everything over, and then delete the old buffer
		auto *new_buffer = new uint8_t[new_size] { 0 };
		std::memcpy(new_buffer, m_buffer, m_buffer_size);
		delete[] m_buffer;
		m_buffer = new_buffer;
	}

	void BitStream::validate_buffer()
	{
		// Make sure we haven't underflowed
		if (m_position.get_byte() < 0)
			throw std::runtime_error("Position of buffer is less than 0!");

		// Expand the buffer if we've overflowed
		if (m_position.get_byte() >= m_buffer_size)
			expand_buffer();
	}
}
