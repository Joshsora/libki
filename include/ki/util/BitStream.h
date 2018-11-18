#pragma once
#include <cstdint>
#include <type_traits>
#include <sstream>
#include "ki/util/BitTypes.h"

#define KI_BITSTREAM_DEFAULT_BUFFER_SIZE 0x2000

namespace ki
{
	/**
	 * An abstract base class that provides a common interface for
	 * writing to, reading from, and querying bit streams.
	 */
	class BitStreamBase
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

		virtual ~BitStreamBase() {}

		/**
		 * @returns The stream's current position.
		 */
		virtual stream_pos tell() const = 0;

		/**
		 * Sets the position of the stream.
		 * @param position The new position of the stream.
		 */
		virtual void seek(stream_pos position) = 0;

		/**
		 * @returns The current size of the internal buffer.
		 */
		virtual std::size_t capacity() const = 0;

		/**
		 * @returns A pointer to the start of the internal buffer.
		 */
		virtual const uint8_t *data() const = 0;

		/**
		 * Reads a value from the buffer given a defined number of bits.
		 * @param[in] bits The number of bits to read. Defaults to the bitsize of IntegerT.
		 * @returns The value read from the buffer.
		 */
		template <
			typename IntegerT,
			typename = std::enable_if<is_integral<IntegerT>::value>
		>
		IntegerT read(const uint8_t bits = bitsizeof<IntegerT>::value)
		{
			return static_cast<IntegerT>(read(bits));
		}

		/**
		 * Writes a value to the buffer that occupies a defined number of bits.
		 * @tparam IntegerT The type of value (must be an integral type).
		 * @param[in] value The value to write.
		 * @param[in] bits The number of bits to use. Defaults to the bitsize of IntegerT.
		 */
		template <
			typename IntegerT,
			typename = std::enable_if<is_integral<IntegerT>::value>
		>
		void write(IntegerT value, const uint8_t bits = bitsizeof<IntegerT>::value)
		{
			write(static_cast<uint64_t>(value), bits);
		}

		/**
		 * Copy memory from an external buffer into the bitstream's buffer from the current position.
		 * @param[in] src The buffer to copy data from.
		 * @param[in] bitsize The number of bits to copy from the src buffer.
		 */
		void write_copy(uint8_t *src, std::size_t bitsize);

		/**
		 * Copy memory from the bitstream's buffer into an external buffer.
		 * @param[out] dst The destination buffer to copy data to.
		 * @param[in] bitsize The number of bits to copy into the dst buffer.
		 */
		void read_copy(uint8_t *dst, std::size_t bitsize);

	protected:
		virtual uint64_t read(uint8_t bits) = 0;
		virtual void write(uint64_t value, uint8_t bits) = 0;
	};

	/**
	 * A read/write-able stream of bits.
	 */
	class BitStream : public BitStreamBase
	{
	public:
		explicit BitStream(std::size_t buffer_size = KI_BITSTREAM_DEFAULT_BUFFER_SIZE);
		virtual ~BitStream();

		stream_pos tell() const override;
		void seek(stream_pos position) override;
		std::size_t capacity() const override;
		const uint8_t *data() const override;

		/**
		 * @copydoc BitStreamBase::read<IntegerT>(uint8_t)
		 * @throws ki::runtime_error Not enough data available to read the specified number of bits.
		 */
		template <
			typename IntegerT,
			typename = std::enable_if<is_integral<IntegerT>::value>
		>
		IntegerT read(const uint8_t bits = bitsizeof<IntegerT>::value)
		{
			return BitStreamBase::read<IntegerT>(bits);
		}

		template <
			typename IntegerT,
			typename = std::enable_if<is_integral<IntegerT>::value>
		>
		void write(IntegerT value, const uint8_t bits = bitsizeof<IntegerT>::value)
		{
			BitStreamBase::write<IntegerT>(value, bits);
		}

	protected:
		uint64_t read(uint8_t bits) override;
		void write(uint64_t value, uint8_t bits) override;

	private:
		uint8_t * m_buffer;
		std::size_t m_buffer_size;
		stream_pos m_position;

		void expand_buffer();
		void validate_buffer();
	};

	/**
	 * TODO: Documentation
	 */
	class BitStreamSection : public BitStreamBase
	{
	public:
		explicit BitStreamSection(BitStreamBase &stream, std::size_t size);

	private:
		BitStreamBase &m_stream;
		stream_pos m_position;
		std::size_t m_size;
	};
}
