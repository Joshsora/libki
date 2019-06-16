#pragma once
#include <cstdint>
#include <type_traits>
#include <sstream>
#include <memory>
#include "ki/util/BitTypes.h"
#include "ki/util/exception.h"

#define KI_BITBUFFER_DEFAULT_SIZE 0x2000

namespace ki
{
	// Forward declaration so that buffers can create segments of themselves
	class BitBufferSegment;

	/**
	 *
	 */
	class IBitBuffer
	{
		friend BitBufferSegment;

	public:
		/**
		 * 
		 */
		struct buffer_pos
		{
			explicit buffer_pos(uint32_t byte = 0, int bit = 0);
			buffer_pos(const buffer_pos &cp);

			uint32_t as_bytes() const;
			uint32_t as_bits() const;
			uint32_t get_byte() const;
			uint8_t get_bit() const;

			buffer_pos operator +(const buffer_pos &rhs) const;
			buffer_pos operator -(const buffer_pos &rhs) const;
			buffer_pos operator +(const int &rhs) const;
			buffer_pos operator -(const int &rhs) const;
			buffer_pos &operator +=(buffer_pos lhs);
			buffer_pos &operator -=(buffer_pos lhs);
			buffer_pos &operator +=(int bits);
			buffer_pos &operator -=(int bits);
			buffer_pos &operator ++();
			buffer_pos &operator --();
			buffer_pos operator ++(int increment);
			buffer_pos operator --(int increment);

		private:
			uint32_t m_byte;
			uint8_t m_bit;

			void set_bit(int bit);
		};

		virtual ~IBitBuffer() {}

		/**
		 * @returns 
		 */
		virtual uint8_t *data() const = 0;

		/**
		 * @returns The size of the buffer in bytes.
		 */
		virtual std::size_t size() const = 0;

		/**
		* Resize the buffer to the specified size.
		* If the current size is larger than the specified new size, then the data is truncated.
		* @param[in] new_size The size to resize the buffer to in bytes.
		*/
		virtual void resize(std::size_t new_size) = 0;

		/**
		 * @param[in] from The position of the start of the segment.
		 * @param[in] bitsize The size of the segment in bits.
		 * @returns A new segment of this buffer.
		 */
		virtual std::unique_ptr<BitBufferSegment> segment(
			buffer_pos from, std::size_t bitsize
		);

		/**
		 * Reads a value from the buffer.
		 * @tparam ValueT The type of value (must be an integral type).
		 * @param[in] position Where in the buffer to retrieve the value from.
		 * @param[in] bits The number of bits the value occupies in the buffer.
 		 * @returns The n-bit value retrieved from the buffer.
 		 * @throws ki::runtime_error Buffer is not large enough to read the specified number of bits at the specified position.
 		 */
		template <typename ValueT>
		ValueT read(const buffer_pos position,
			const uint8_t bits = bitsizeof<ValueT>::value) const
		{
			// Check for buffer overflow
			if ((position + bits).as_bytes() > size())
			{
				std::ostringstream oss;
				oss << "Buffer is not large enough to read a "
					<< static_cast<uint16_t>(bits) << "-bit value at specified position. "
					<< "(byte=" << position.get_byte() << ", bit=" << static_cast<uint16_t>(position.get_bit())
					<< ", size=" << size() << ")";
				throw runtime_error(oss.str());
			}

			return static_cast<ValueT>(read(position, bits));
		}

		/**
		 * Writes a value to the buffer.
		 * @tparam ValueT The type of value (must be an integral type).
		 * @param[in] value The value to write.
		 * @param[in] position Where in the buffer to write the value to.
		 * @param[in] bits The number of bits the value should occupy in the buffer. Defaults to the bitsize of ValueT.
		 * @throws ki::runtime_error Buffer is not large enough to write the specified number of bits at the specified position.
		 */
		template <typename ValueT>
		void write(const ValueT value, const buffer_pos position,
			const uint8_t bits = bitsizeof<ValueT>::value)
		{
			// Check for buffer overflow
			if ((position + bits).as_bytes() > size())
			{
				std::ostringstream oss;
				oss << "Buffer is not large enough to write a "
					<< static_cast<uint16_t>(bits) << "-bit value at specified position. "
					<< "(byte=" << position.get_byte() << ", bit=" << static_cast<uint16_t>(position.get_bit())
					<< ", size=" << size() << ")";
				throw runtime_error(oss.str());
			}

			write(static_cast<uint64_t>(value), position, bits);
		}

		/**
		 * Copy memory from this buffer into an external buffer.
		 * @param[out] dst The destination buffer to copy data to.
		 * @param[in] position Where in the buffer to start copying data from.
		 * @param[in] bitsize The number of bits to copy into the destination buffer.
		 */
		void read_copy(uint8_t *dst, buffer_pos position, std::size_t bitsize) const;

		/**
		 * Copy memory from an external buffer into this buffer at the specified position.
		 * @param[in] src The source buffer to copy data from.
		 * @param[in] position Where in the buffer to start copying data to.
		 * @param[in] bitsize The number of bits to copy from the source buffer.
		 */
		void write_copy(uint8_t *src, buffer_pos position, std::size_t bitsize);

	protected:
		virtual uint64_t read(buffer_pos position, uint8_t bits) const = 0;
		virtual void write(uint64_t value, buffer_pos position, uint8_t bits) = 0;
	};

	/**
	 * TODO: Documentation
	 */
	class BitBuffer : public IBitBuffer
	{
	public:
		explicit BitBuffer(std::size_t buffer_size = KI_BITBUFFER_DEFAULT_SIZE);
		~BitBuffer();

		BitBuffer(const BitBuffer &that);
		BitBuffer &operator=(const BitBuffer &that);

		/**
		* Create a new BitBuffer from an existing buffer.
		* @param[in] buffer The buffer to take ownership of.
		* @param[in] buffer_size The size of the buffer in bytes.
		*/
		explicit BitBuffer(uint8_t *buffer, std::size_t buffer_size);

		std::size_t size() const override;
		uint8_t* data() const override;
		void resize(std::size_t new_size) override;

		/**
		 * @copydoc IBitBuffer::read<ValueT>(buffer_pos, uint8_t)
		 */
		template <typename ValueT>
		ValueT read(const buffer_pos position,
			const uint8_t bits = bitsizeof<ValueT>::value) const
		{
			return IBitBuffer::read<ValueT>(position, bits);
		}

		/**
		 * @copydoc IBitBuffer::write<ValueT>(ValueT, buffer_pos, uint8_t)
		 */
		template <typename ValueT>
		void write(const ValueT value, const buffer_pos position,
			const uint8_t bits = bitsizeof<ValueT>::value)
		{
			return IBitBuffer::write<ValueT>(value, position, bits);
		}

	protected:
		uint64_t read(buffer_pos position, uint8_t bits) const override;
		void write(uint64_t value, buffer_pos position, uint8_t bits) override;

	private:
		uint8_t *m_buffer;
		std::size_t m_buffer_size;
	};
	
	/**
	 * TODO: Documentation
	 */
	class BitBufferSegment : public IBitBuffer
	{
	public:
		BitBufferSegment(IBitBuffer &buffer, buffer_pos from, std::size_t bitsize);

		std::size_t size() const override;
		void resize(std::size_t new_size) override;
		uint8_t *data() const override;
		std::unique_ptr<BitBufferSegment> segment(
			buffer_pos from, std::size_t bitsize
		) override;

		/**
		 * @copydoc IBitBuffer::read<ValueT>(buffer_pos, uint8_t)
		 */
		template <typename ValueT>
		ValueT read(const buffer_pos position,
			const uint8_t bits = bitsizeof<ValueT>::value) const
		{
			return IBitBuffer::read<ValueT>(position, bits);
		}

		/**
		 * @copydoc IBitBuffer::write<ValueT>(ValueT, buffer_pos, uint8_t)
		 */
		template <typename ValueT>
		void write(const ValueT value, const buffer_pos position,
			const uint8_t bits = bitsizeof<ValueT>::value)
		{
			return IBitBuffer::write<ValueT>(value, position, bits);
		}

	protected:
		uint64_t read(buffer_pos position, uint8_t bits) const override;
		void write(uint64_t value, buffer_pos position, uint8_t bits) override;

	private:
		IBitBuffer *m_buffer;
		buffer_pos m_from;
		std::size_t m_bitsize;
	};

	/**
	 * A read/write-able stream of bits.
	 */
	class BitStream
	{
	public:
		using stream_pos = BitBuffer::buffer_pos;

		explicit BitStream(IBitBuffer &buffer);
		virtual ~BitStream();

		BitStream(const BitStream &that);
		BitStream &operator=(const BitStream &that);

		/**
		 * @returns The stream's current position.
		 */
		stream_pos tell() const;

		/**
		 * Sets the position of the stream.
		 * @param[in] position The new position of the stream.
		 * @param[in] expand Whether or not to expand the buffer.
		 */
		void seek(stream_pos position, bool expand = true);

		/**
		* Realigns the position of the stream so that it lies directly on a byte.
		*/
		void realign();

		/**
		 * @returns The current size of the internal buffer.
		 */
		std::size_t capacity() const;

		/**
		* @returns The BitBuffer that the stream is reading/writing to.
		*/
		IBitBuffer &buffer() const;

		/**
		 * Reads a value from the buffer.
		 * @tparam ValueT The type used to store the value.
		 * @param[in] bits The number of bits to read. Defaults to the bitsize of ValueT.
		 * @returns The value read from the buffer.
		 */
		template <
			typename ValueT,
			typename = std::enable_if<is_integral<ValueT>::value>
		>
		ValueT read(const uint8_t bits = bitsizeof<ValueT>::value)
		{
			ValueT value = m_buffer->read<ValueT>(m_position, bits);
			m_position += bits;
			return value;
		}

		/**
		 * Writes a value to the buffer.
		 * If the buffer is not big enough to write the value, then the buffer is resized.
		 * @tparam ValueT The type of value (must be an integral type).
		 * @param[in] value The value to write.
		 * @param[in] bits The number of bits to use. Defaults to the bitsize of ValueT.
		 */
		template <
			typename ValueT,
			typename = std::enable_if<is_integral<ValueT>::value>
		>
		void write(ValueT value, const uint8_t bits = bitsizeof<ValueT>::value)
		{
			expand_buffer(m_position + bits);
			m_buffer->write<ValueT>(value, m_position, bits);
			m_position += bits;
		}

		/**
		 * Copy memory from the BitStream's buffer into an external buffer at the current position.
		 * @param[out] dst The destination buffer to copy data to.
		 * @param[in] bitsize The number of bits to copy into the destination buffer.
		 */
		void read_copy(uint8_t *dst, std::size_t bitsize);

		/**
		 * Copy memory from an external buffer into the BitStream's buffer from the current position.
		 * @param[in] src The source buffer to copy data from.
		 * @param[in] bitsize The number of bits to copy from the source buffer.
		 */
		void write_copy(uint8_t *src, std::size_t bitsize);

	private:
		IBitBuffer *m_buffer;
		stream_pos m_position;

		/**
		 * Expand the buffer such that a position becomes valid.
		 * @param[in] position The minimum position that should be accessible after expanding.
		 */
		void expand_buffer(stream_pos position) const;
	};
}
