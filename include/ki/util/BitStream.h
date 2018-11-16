#pragma once
#include <cstdint>
#include <type_traits>
#include "ki/util/BitTypes.h"

#define KI_BITSTREAM_DEFAULT_BUFFER_SIZE 0x2000

namespace ki
{
	/**
	 * A readable/writeable stream of bits.
	 */
	class BitStream
	{
	public:
		/**
		 * Represents a position in a BitStream's buffer.
		 */
		struct stream_pos
		{
			explicit stream_pos(intmax_t byte = 0, int bit = 0);
			stream_pos(const stream_pos &cp);

			intmax_t as_bits() const;
			intmax_t get_byte() const;
			uint8_t get_bit() const;
			
			stream_pos operator +(const stream_pos &rhs) const;
			stream_pos operator -(const stream_pos &rhs) const;
			stream_pos operator +(const int &rhs) const;
			stream_pos operator -(const int &rhs) const;
			stream_pos &operator +=(stream_pos lhs);
			stream_pos &operator -=(stream_pos lhs);
			stream_pos &operator +=(int bits);
			stream_pos &operator -=(int bits);
			stream_pos &operator ++();
			stream_pos &operator --();
			stream_pos operator ++(int increment);
			stream_pos operator --(int increment);

		private:
			intmax_t m_byte;
			uint8_t m_bit;

			void set_bit(int bit);
		};

		explicit BitStream(std::size_t buffer_size = KI_BITSTREAM_DEFAULT_BUFFER_SIZE);
		~BitStream();

		/**
		 * @return The stream's current position.
		 */
		stream_pos tell() const;

		/**
		 * Sets the position of the stream.
		 * @param position The new position of the stream.
		 */
		void seek(stream_pos position);

		/**
		 * @return The current size of the internal buffer.
		 */
		std::size_t capacity() const;

		/**
		 * @return A pointer to the start of the internal buffer.
		 */
		const uint8_t *data() const;

		/**
		 * Reads a value from the buffer given a defined number of bits.
		 * @param bits The number of bits to read.
		 */
		template <
			typename IntegerT,
			typename = std::enable_if<is_integral<IntegerT>::value>
		>
		IntegerT read(const uint8_t bits = bitsizeof<IntegerT>::value)
		{
			IntegerT value = 0;

			// Iterate until we've read all of the bits
			auto unread_bits = bits;
			while (unread_bits > 0)
			{
				// Calculate how many bits to read from the current byte based on how many bits 
				// are left and how many bits we still need to read
				const uint8_t bits_available = (8 - m_position.get_bit());
				const auto bit_count = unread_bits < bits_available ? unread_bits : bits_available;

				// Find the bit-mask based on how many bits are being read
				const uint8_t bit_mask = ((1 << bit_count) - 1) << m_position.get_bit();
				
				// Read the bits from the current byte and position them on the least-signficant bit
				const uint8_t bits_value = (m_buffer[m_position.get_byte()] & bit_mask) >> m_position.get_bit();

				// Position the value of the bits we just read based on how many bits of the value 
				// we've already read
				const uint8_t read_bits = bits - unread_bits;
				value |= (IntegerT)bits_value << read_bits;

				// Remove the bits we just read from the count of unread bits
				unread_bits -= bit_count;

				// Move forward the number of bits we just read
				seek(tell() + bit_count);
			}

			return value;
		}

		/**
		 * Writes a value to the buffer that occupies a defined number of bits.
		 * @param value The value to write.
		 * @param bits The number of bits to use.
		 */
		template <
			typename IntegerT,
			typename = std::enable_if<is_integral<IntegerT>::value>
		>
		void write(IntegerT value, const uint8_t bits = bitsizeof<IntegerT>::value)
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
				IntegerT bit_mask = (IntegerT)((1 << bit_count) - 1) << written_bits;

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

		/**
		 * Copy memory from an external buffer into the bitstream's buffer from the current position.
		 * @param src The buffer to copy data from.
		 * @param bitsize The number of bits to copy from the src buffer.
		 */
		void write_copy(uint8_t *src, std::size_t bitsize);

		/**
		 * Copy memory from the bitstream's buffer into an external buffer.
		 * @param dst The destination buffer to copy data to.
		 * @param bitsize The number of bits to copy into the dst buffer.
		 */
		void read_copy(uint8_t *dst, std::size_t bitsize);

	private:
		uint8_t *m_buffer;
		std::size_t m_buffer_size;
		stream_pos m_position;

		void expand_buffer();
		void validate_buffer();
	};
}
