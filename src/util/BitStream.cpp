#include "ki/util/BitStream.h"
#include "ki/util/exception.h"
#include <limits>
#include <cstring>
#include <cmath>

namespace ki
{
	BitBufferBase::buffer_pos::buffer_pos(const uint32_t byte, const int bit)
	{
		m_byte = byte;
		set_bit(bit);
	}

	BitBufferBase::buffer_pos::buffer_pos(const buffer_pos& cp)
	{
		m_byte = cp.m_byte;
		set_bit(cp.m_bit);
	}

	uint32_t BitBufferBase::buffer_pos::as_bytes() const
	{
		return m_byte + (m_bit > 0 ? 1 : 0);
	}

	uint32_t BitBufferBase::buffer_pos::as_bits() const
	{
		return (m_byte * 8) + m_bit;
	}

	uint32_t BitBufferBase::buffer_pos::get_byte() const
	{
		return m_byte;
	}

	uint8_t BitBufferBase::buffer_pos::get_bit() const
	{
		return m_bit;
	}

	void BitBufferBase::buffer_pos::set_bit(int bit)
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

	BitBufferBase::buffer_pos BitBufferBase::buffer_pos::operator+(
		const buffer_pos &rhs) const
	{
		return buffer_pos(
			m_byte + rhs.m_byte, m_bit + rhs.m_bit
		);
	}

	BitBufferBase::buffer_pos BitBufferBase::buffer_pos::operator-(
		const buffer_pos &rhs) const
	{
		return buffer_pos(
			m_byte - rhs.m_byte, m_bit - rhs.m_bit
		);
	}

	BitBufferBase::buffer_pos BitBufferBase::buffer_pos::operator+(
		const int& rhs) const
	{
		return buffer_pos(
			m_byte, m_bit + rhs
		);
	}

	BitBufferBase::buffer_pos BitBufferBase::buffer_pos::operator-(
		const int& rhs) const
	{
		return buffer_pos(
			m_byte, m_bit - rhs
		);
	}

	BitBufferBase::buffer_pos& BitBufferBase::buffer_pos::operator+=(
		const buffer_pos rhs)
	{
		m_byte += rhs.m_byte;
		set_bit(m_bit + rhs.m_bit);
		return *this;
	}

	BitBufferBase::buffer_pos& BitBufferBase::buffer_pos::operator-=(
		const buffer_pos rhs)
	{
		m_byte -= rhs.m_byte;
		set_bit(m_bit - rhs.m_bit);
		return *this;
	}

	BitBufferBase::buffer_pos& BitBufferBase::buffer_pos::operator+=(const int bits)
	{
		set_bit(m_bit + bits);
		return *this;
	}

	BitBufferBase::buffer_pos& BitBufferBase::buffer_pos::operator-=(const int bits)
	{
		set_bit(m_bit - bits);
		return *this;
	}

	BitBufferBase::buffer_pos& BitBufferBase::buffer_pos::operator++()
	{
		set_bit(m_bit + 1);
		return *this;
	}

	BitBufferBase::buffer_pos& BitBufferBase::buffer_pos::operator--()
	{
		set_bit(m_bit - 1);
		return *this;
	}

	BitBufferBase::buffer_pos BitBufferBase::buffer_pos::operator++(int increment)
	{
		auto copy(*this);
		++(*this);
		return copy;
	}

	BitBufferBase::buffer_pos BitBufferBase::buffer_pos::operator--(int increment)
	{
		auto copy(*this);
		--(*this);
		return copy;
	}

	BitBufferSegment *BitBufferBase::segment(
		const buffer_pos from, const std::size_t bitsize)
	{
		return new BitBufferSegment(*this, from, bitsize);
	}

	void BitBufferBase::write_copy(uint8_t *src,
		buffer_pos position, const std::size_t bitsize)
	{
		auto bits_left = bitsize;
		while (bits_left > 0)
		{
			const auto bits = bits_left >= 8 ? 8 : bits_left;
			write<uint8_t>(*(src++), position, bits);
			bits_left -= bits;
			position += bits;
		}
	}

	void BitBufferBase::read_copy(uint8_t *dst,
		buffer_pos position, const std::size_t bitsize) const
	{
		auto bits_left = bitsize;
		while (bits_left > 0)
		{
			const auto bits = bits_left >= 8 ? 8 : bits_left;
			*(dst++) = read<uint8_t>(position, bits);			
			bits_left -= bits;
			position += bits;
		}
	}

	BitBuffer::BitBuffer(const size_t buffer_size)
	{
		m_buffer = new uint8_t[buffer_size]{0};
		m_buffer_size = buffer_size;
	}

	BitBuffer::~BitBuffer()
	{
		delete[] m_buffer;
		m_buffer = nullptr;
		m_buffer_size = 0;
	}

	BitBuffer::BitBuffer(const BitBuffer& that)
	{
		m_buffer_size = that.m_buffer_size;
		m_buffer = new uint8_t[m_buffer_size]{0};
		std::memcpy(m_buffer, that.m_buffer, m_buffer_size);
	}

	BitBuffer &BitBuffer::operator=(const BitBuffer &that)
	{
		m_buffer_size = that.m_buffer_size;
		m_buffer = new uint8_t[m_buffer_size]{0};
		std::memcpy(m_buffer, that.m_buffer, m_buffer_size);
		return (*this);
	}

	BitBuffer::BitBuffer(uint8_t *buffer, const std::size_t buffer_size)
	{
		m_buffer = buffer;
		m_buffer_size = buffer_size;
	}

	std::size_t BitBuffer::size() const
	{
		return m_buffer_size;
	}

	void BitBuffer::resize(const std::size_t new_size)
	{
		auto *new_buffer = new uint8_t[new_size]{0};
		std::memcpy(new_buffer, m_buffer, new_size >= m_buffer_size ? m_buffer_size : new_size);
		delete[] m_buffer;
		m_buffer = new_buffer;
		m_buffer_size = new_size;
	}

	uint8_t *BitBuffer::data() const
	{
		return m_buffer;
	}

	uint64_t BitBuffer::read(buffer_pos position, const uint8_t bits) const
	{
		// Iterate until we've read all of the bits
		uint64_t value = 0;
		auto unread_bits = bits;
		while (unread_bits > 0)
		{
			// Calculate how many bits to read from the current byte based on how many bits 
			// are left and how many bits we still need to read
			const uint8_t bits_available = (8 - position.get_bit());
			const auto bit_count = unread_bits < bits_available ? unread_bits : bits_available;

			// Find the bit-mask based on how many bits are being read
			const uint8_t bit_mask = (1 << bit_count) - 1 << position.get_bit();

			// Read the bits from the current byte and position them on the least-signficant bit
			const uint8_t bits_value = (m_buffer[position.get_byte()] & bit_mask) >> position.get_bit();

			// Position the value of the bits we just read based on how many bits of the value 
			// we've already read
			const uint8_t read_bits = bits - unread_bits;
			value |= static_cast<uint64_t>(bits_value) << read_bits;

			// Remove the bits we just read from the count of unread bits
			unread_bits -= bit_count;

			// Move forward the number of bits we just read
			position += bit_count;
		}

		return value;
	}

	void BitBuffer::write(const uint64_t value,
		buffer_pos position, const uint8_t bits)
	{
		// Iterate until we've written all of the bits
		auto unwritten_bits = bits;
		while (unwritten_bits > 0)
		{
			// Calculate how many bits to write based on how many bits are left in the current byte
			// and how many bits from the value we still need to write
			const uint8_t bits_available = (8 - position.get_bit());
			const auto bit_count = unwritten_bits < bits_available ? unwritten_bits : bits_available;

			// Find the bit-mask based on how many bits are being written, and how many bits we've
			// already written
			const uint8_t written_bits = bits - unwritten_bits;
			const auto bit_mask = static_cast<uint64_t>((1 << bit_count) - 1) << written_bits;

			// Get the bits from the value and position them at the current bit position
			uint8_t value_byte = ((value & bit_mask) >> written_bits) & 0xFF;
			value_byte <<= position.get_bit();

			// Write the bits into the byte we're currently at
			m_buffer[position.get_byte()] |= value_byte;
			unwritten_bits -= bit_count;

			// Move forward the number of bits we just wrote
			position += bit_count;
		}
	}

	BitBufferSegment::BitBufferSegment(BitBufferBase& buffer,
		const buffer_pos from, const std::size_t bitsize)
	{
		m_buffer = &buffer;

		// Is this a valid position to start reading from?
		if (from.get_byte() >= m_buffer->size())
			throw runtime_error("Cannot create segment of buffer from position outside the original buffer.");
		m_from = from;

		// Is the size valid?
		if ((m_from + bitsize).as_bytes() > m_buffer->size())
		{
			std::ostringstream oss;
			oss << "Not enough data in original buffer to create segment of size: "
				<< bitsize;
			throw runtime_error(oss.str());
		}
		m_bitsize = bitsize;
	}

	std::size_t BitBufferSegment::size() const
	{
		return buffer_pos(m_bitsize / 8, m_bitsize % 8).as_bytes();
	}

	void BitBufferSegment::resize(std::size_t new_size)
	{
		throw runtime_error("A BitBufferSegment cannot be resized.");
	}

	uint8_t* BitBufferSegment::data() const
	{
		return &m_buffer->data()[m_from.get_byte()];
	}

	BitBufferSegment *BitBufferSegment::segment(
		const buffer_pos from, const std::size_t bitsize)
	{
		return new BitBufferSegment(*m_buffer, m_from + from, bitsize);
	}

	uint64_t BitBufferSegment::read(
		const buffer_pos position, const uint8_t bits) const
	{
		return m_buffer->read(m_from + position, bits);
	}

	void BitBufferSegment::write(const uint64_t value,
		const buffer_pos position, const uint8_t bits)
	{
		m_buffer->write(value, m_from + position, bits);
	}

	BitStream::BitStream(BitBufferBase &buffer)
	{
		m_buffer = &buffer;
		m_position = stream_pos(0, 0);
	}

	BitStream::~BitStream()
	{
		m_buffer = nullptr;
	}

	BitStream::BitStream(const BitStream &that)
	{
		m_buffer = that.m_buffer;
		m_position = that.m_position;
	}

	BitStream& BitStream::operator=(const BitStream &that)
	{
		m_buffer = that.m_buffer;
		m_position = that.m_position;
		return *this;
	}

	BitBufferBase::buffer_pos BitStream::tell() const
	{
		return m_position;
	}

	void BitStream::seek(const stream_pos position, const bool expand)
	{
		if (expand)
			expand_buffer(position);
		m_position = position;
	}

	std::size_t BitStream::capacity() const
	{
		return m_buffer->size();
	}

	BitBufferBase &BitStream::buffer() const
	{
		return *m_buffer;
	}

	void BitStream::read_copy(uint8_t *dst, const std::size_t bitsize)
	{
		m_buffer->read_copy(dst, m_position, bitsize);
		m_position += bitsize;
	}

	void BitStream::write_copy(uint8_t *src, const std::size_t bitsize)
	{
		expand_buffer(m_position + bitsize);
		m_buffer->write_copy(src, m_position, bitsize);
		m_position += bitsize;
	}

	void BitStream::expand_buffer(const stream_pos position) const
	{
		// Expand the buffer if we've overflowed
		if (position.as_bytes() >= m_buffer->size())
		{
			// Work out a new buffer size
			const auto minimum_bits = static_cast<uint64_t>(
				std::log2(position.get_byte())
				) + 1;
			auto new_size = (2 << minimum_bits) + 2;
			if (new_size < m_buffer->size())
				new_size = std::numeric_limits<size_t>::max();

			// Has the buffer reached maximum size?
			if (new_size == m_buffer->size())
				throw runtime_error("Buffer cannot be expanded as it has reached maximum size.");

			// Resize the buffer
			m_buffer->resize(new_size);
		}
	}
}
