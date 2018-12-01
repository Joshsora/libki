#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <ki/util/BitStream.h>

/*
 * Define values we expect during testing.
 */
#define KI_TEST_BITSTREAM_BUI1 0b1
#define KI_TEST_BITSTREAM_BUI2 0b10
#define KI_TEST_BITSTREAM_BUI3 0b010
#define KI_TEST_BITSTREAM_BUI4 0b0101
#define KI_TEST_BITSTREAM_BUI5 0b10101
#define KI_TEST_BITSTREAM_BUI6 0b101010
#define KI_TEST_BITSTREAM_BUI7 0b0101010
#define KI_TEST_BITSTREAM_BI1 -1
#define KI_TEST_BITSTREAM_BI2 -2
#define KI_TEST_BITSTREAM_BI3 -3
#define KI_TEST_BITSTREAM_BI4 -6
#define KI_TEST_BITSTREAM_BI5 -11
#define KI_TEST_BITSTREAM_BI6 -22
#define KI_TEST_BITSTREAM_BI7 -43
#define KI_TEST_BITSTREAM_BOOL true
#define KI_TEST_BITSTREAM_U8 0x01
#define KI_TEST_BITSTREAM_U16 0x0302
#define KI_TEST_BITSTREAM_U24 0x060504
#define KI_TEST_BITSTREAM_U32 0x0A090807
#define KI_TEST_BITSTREAM_U64 0x1211100F0E0D0C0BL
#define KI_TEST_BITSTREAM_I8 -8
#define KI_TEST_BITSTREAM_I16 -2048
#define KI_TEST_BITSTREAM_I24 -524288
#define KI_TEST_BITSTREAM_I32 -134217728
#define KI_TEST_BITSTREAM_I64 -576460752303423488L

#define KI_TEST_WRITE_BUI(n) bit_stream.write<bui<n>>(KI_TEST_BITSTREAM_BUI##n)
#define KI_TEST_READ_BUI(n) bit_stream.read<bui<n>>() == KI_TEST_BITSTREAM_BUI##n
#define KI_TEST_WRITE_BI(n) bit_stream.write<bi<n>>(KI_TEST_BITSTREAM_BI##n)
#define KI_TEST_READ_BI(n) bit_stream.read<bi<n>>() == KI_TEST_BITSTREAM_BI##n

using namespace ki;

TEST_CASE("BitStream::stream_pos Functionality", "[bit-stream]")
{
	BitStream::stream_pos position(1, 4);

	SECTION("Increment single bit")
	{
		const auto position1 = ++position;
		REQUIRE(position.get_byte() == 1);
		REQUIRE(position.get_bit() == 5);
		REQUIRE(position1.get_byte() == 1);
		REQUIRE(position1.get_bit() == 5);

		const auto position2 = position++;
		REQUIRE(position.get_byte() == 1);
		REQUIRE(position.get_bit() == 6);
		REQUIRE(position2.get_byte() == 1);
		REQUIRE(position2.get_bit() == 5);

		const auto position3 = position + 1;
		REQUIRE(position3.get_byte() == 1);
		REQUIRE(position3.get_bit() == 7);

		const auto position4 = position + BitStream::stream_pos(0, 1);
		REQUIRE(position4.get_byte() == 1);
		REQUIRE(position4.get_bit() == 7);
	}

	SECTION("Decrement single bit")
	{
		const auto position1 = --position;
		REQUIRE(position.get_byte() == 1);
		REQUIRE(position.get_bit() == 3);
		REQUIRE(position1.get_byte() == 1);
		REQUIRE(position1.get_bit() == 3);

		const auto position2 = position--;
		REQUIRE(position.get_byte() == 1);
		REQUIRE(position.get_bit() == 2);
		REQUIRE(position2.get_byte() == 1);
		REQUIRE(position2.get_bit() == 3);

		const auto position3 = position - 1;
		REQUIRE(position3.get_byte() == 1);
		REQUIRE(position3.get_bit() == 1);

		const auto position4 = position - BitStream::stream_pos(0, 1);
		REQUIRE(position4.get_byte() == 1);
		REQUIRE(position4.get_bit() == 1);
	}

	SECTION("Increment bits and move to next byte")
	{
		position += 4;
		REQUIRE(position.get_byte() == 2);
		REQUIRE(position.get_bit() == 0);
	}

	SECTION("Decrement bits and move to previous byte")
	{
		position -= 5;
		REQUIRE(position.get_byte() == 0);
		REQUIRE(position.get_bit() == 7);
	}

	SECTION("Increment byte")
	{
		position += BitStream::stream_pos(1, 0);
		REQUIRE(position.get_byte() == 2);
		REQUIRE(position.get_bit() == 4);
	}

	SECTION("Decrement byte")
	{
		position -= BitStream::stream_pos(1, 0);
		REQUIRE(position.get_byte() == 0);
		REQUIRE(position.get_bit() == 4);
	}

	SECTION("As number of bytes")
	{
		REQUIRE(position.as_bytes() == 2);
		position -= 4;
		REQUIRE(position.as_bytes() == 1);
	}

	SECTION("As number of bits")
	{
		REQUIRE(position.as_bits() == 12);
		position -= 4;
		REQUIRE(position.as_bits() == 8);
	}
}

TEST_CASE("BitBuffer Functionality", "[bit-stream]")
{
	BitBuffer bit_buffer(1);

	SECTION("Create valid segment")
	{
		auto *segment = bit_buffer.segment(
			BitBuffer::buffer_pos(0, 0), 8
		);
		REQUIRE(segment->size() == 1);
		delete segment;
	}

	SECTION("Create invalid segment (invalid position)")
	{
		try
		{
			auto *segment = bit_buffer.segment(
				BitBuffer::buffer_pos(1, 0), 8
			);
			delete segment;
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}

	SECTION("Create invalid segment (invalid size)")
	{
		try
		{
			auto *segment = bit_buffer.segment(
				BitBuffer::buffer_pos(0, 0), 16
			);
			delete segment;
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}
}

TEST_CASE("BitStream Functionality", "[bit-stream]")
{
	BitBuffer bit_buffer;
	auto bit_stream = BitStream(bit_buffer);

	SECTION("Writing values with a size less than 8 bits")
	{
		// Write an alternating pattern of bits
		KI_TEST_WRITE_BUI(1);
		KI_TEST_WRITE_BUI(2);
		KI_TEST_WRITE_BUI(3);
		KI_TEST_WRITE_BUI(4);
		KI_TEST_WRITE_BUI(5);
		KI_TEST_WRITE_BUI(6);
		KI_TEST_WRITE_BUI(7);
		KI_TEST_WRITE_BUI(4);

		// Write signed values
		KI_TEST_WRITE_BI(1);
		KI_TEST_WRITE_BI(2);
		KI_TEST_WRITE_BI(3);
		KI_TEST_WRITE_BI(4);
		KI_TEST_WRITE_BI(5);
		KI_TEST_WRITE_BI(6);
		KI_TEST_WRITE_BI(7);
		KI_TEST_WRITE_BI(4);

		// Make sure tell is reporting the right position
		auto position = bit_stream.tell();
		REQUIRE(position.get_byte() == 8);
		REQUIRE(position.get_bit() == 0);
		const auto size = position.get_byte();

		// Validate what we've got here with a hand-written sample
		std::ifstream sample("samples/bitstream1.bin", std::ios::binary);
		REQUIRE(sample.is_open());

		// Load the sample data and compare
		auto *sample_data = new char[size];
		auto *stream_data = bit_buffer.data();
		sample.read(sample_data, size);
		REQUIRE(memcmp(sample_data, stream_data, size) == 0);

		// Free resources
		delete[] sample_data;
	}

	SECTION("Writing values with a size greater than 8 bits")
	{
		// Write unsigned values
		bit_stream.write<bool>(KI_TEST_BITSTREAM_BOOL);
		bit_stream.write<uint8_t>(KI_TEST_BITSTREAM_U8);
		bit_stream.write<uint16_t>(KI_TEST_BITSTREAM_U16);
		bit_stream.write<bui<24>>(KI_TEST_BITSTREAM_U24);
		bit_stream.write<uint32_t>(KI_TEST_BITSTREAM_U32);
		bit_stream.write<uint64_t>(KI_TEST_BITSTREAM_U64);

		// Write signed values
		bit_stream.write<int8_t>(KI_TEST_BITSTREAM_I8);
		bit_stream.write<int16_t>(KI_TEST_BITSTREAM_I16);
		bit_stream.write<bi<24>>(KI_TEST_BITSTREAM_I24);
		bit_stream.write<int32_t>(KI_TEST_BITSTREAM_I32);
		bit_stream.write<int64_t>(KI_TEST_BITSTREAM_I64);

		// Make sure tell is reporting the right position
		auto position = bit_stream.tell();
		REQUIRE(position.get_byte() == 37);
		REQUIRE(position.get_bit() == 0);
		const auto size = position.get_byte();

		// Validate what we've got here with a hand-written sample
		std::ifstream sample("samples/bitstream2.bin", std::ios::binary);
		REQUIRE(sample.is_open());

		// Load the sample data and compare
		auto *stream_data = bit_buffer.data();
		auto *sample_data = new char[size];
		sample.read(sample_data, size);
		REQUIRE(memcmp(sample_data, stream_data, size) == 0);

		// Free resources
		delete[] sample_data;
	}

	SECTION("Reading values with a size lower than 8 bits")
	{
		// Open a previously hand-written sample
		std::ifstream sample("samples/bitstream1.bin", std::ios::binary);
		REQUIRE(sample.is_open());

		// Load the sample data into the bit stream
		const auto begin = sample.tellg();
		sample.seekg(0, std::ios::end);
		const auto end = sample.tellg();
		const size_t size = end - begin;
		sample.seekg(std::ios::beg);
		auto *stream_data = reinterpret_cast<char *>(bit_buffer.data());
		sample.read(stream_data, size);

		// Read the values and check they are what we are expecting
		// Unsigned values
		REQUIRE(KI_TEST_READ_BUI(1));
		REQUIRE(KI_TEST_READ_BUI(2));
		REQUIRE(KI_TEST_READ_BUI(3));
		REQUIRE(KI_TEST_READ_BUI(4));
		REQUIRE(KI_TEST_READ_BUI(5));
		REQUIRE(KI_TEST_READ_BUI(6));
		REQUIRE(KI_TEST_READ_BUI(7));
		REQUIRE(KI_TEST_READ_BUI(4));

		// Signed values
		REQUIRE(KI_TEST_READ_BI(1));
		REQUIRE(KI_TEST_READ_BI(2));
		REQUIRE(KI_TEST_READ_BI(3));
		REQUIRE(KI_TEST_READ_BI(4));
		REQUIRE(KI_TEST_READ_BI(5));
		REQUIRE(KI_TEST_READ_BI(6));
		REQUIRE(KI_TEST_READ_BI(7));
		REQUIRE(KI_TEST_READ_BI(4));
	}

	SECTION("Reading values with a size greater than 8 bits")
	{
		// Open a previously hand-written sample
		std::ifstream sample("samples/bitstream2.bin", std::ios::binary);
		REQUIRE(sample.is_open());

		// Load the sample data into the bit stream
		const auto begin = sample.tellg();
		sample.seekg(0, std::ios::end);
		const auto end = sample.tellg();
		const size_t size = end - begin;
		sample.seekg(std::ios::beg);
		auto *stream_data = reinterpret_cast<char *>(bit_buffer.data());
		sample.read(stream_data, size);

		// Read the values and check they are what we are expecting
		// Unsigned values
		REQUIRE(bit_stream.read<bool>() == KI_TEST_BITSTREAM_BOOL);
		REQUIRE(bit_stream.read<uint8_t>() == KI_TEST_BITSTREAM_U8);
		REQUIRE(bit_stream.read<uint16_t>() == KI_TEST_BITSTREAM_U16);
		REQUIRE(bit_stream.read<bui<24>>() == KI_TEST_BITSTREAM_U24);
		REQUIRE(bit_stream.read<uint32_t>() == KI_TEST_BITSTREAM_U32);
		REQUIRE(bit_stream.read<uint64_t>() == KI_TEST_BITSTREAM_U64);

		// Signed values
		REQUIRE(bit_stream.read<int8_t>() == KI_TEST_BITSTREAM_I8);
		REQUIRE(bit_stream.read<int16_t>() == KI_TEST_BITSTREAM_I16);
		REQUIRE(bit_stream.read<bi<24>>() == KI_TEST_BITSTREAM_I24);
		REQUIRE(bit_stream.read<int32_t>() == KI_TEST_BITSTREAM_I32);
		REQUIRE(bit_stream.read<int64_t>() == KI_TEST_BITSTREAM_I64);
	}

	SECTION("Copying memory in/out of the stream")
	{
		// Copy the string into the stream
		char *str = "Hello, world!";
		const auto size = bitsizeof<char>::value * (strlen(str) + 1);
		bit_stream.write_copy(reinterpret_cast<uint8_t *>(str), size);

		// Copy the string from the stream into a new buffer
		auto *str_copy = new char[size];
		bit_stream.seek(BitStream::stream_pos(0, 0));
		bit_stream.read_copy(reinterpret_cast<uint8_t *>(str_copy), size);
		REQUIRE(strcmp(str, str_copy) == 0);

		delete[] str_copy;
	}

	SECTION("Overwriting values")
	{
		bit_stream.write<bui<2>>(0b0011);
		bit_stream.seek(BitStream::stream_pos(0, 0));
		bit_stream.write<bui<4>>(0b1100);
		bit_stream.seek(BitStream::stream_pos(0, 0));
		REQUIRE(
			static_cast<uint16_t>(bit_stream.read<bui<4>>()) == 0b1100
		);
	}
}

TEST_CASE("BitStream buffer expansion", "[bit-stream]")
{
	// Create a very small buffer that we know is going to expand
	auto bit_buffer = BitBuffer(1);
	auto bit_stream = BitStream(bit_buffer);

	// Write a byte, and then check if the capacity grows
	bit_stream.write<uint8_t>(0x55, 8);
	REQUIRE(bit_stream.capacity() == 6);
	
	// Go back to the first byte, and make sure what we wrote stuck
	// around after the expansion
	bit_stream.seek(BitStream::stream_pos(0, 0));
	REQUIRE(bit_stream.read<uint8_t>(8) == 0x55);
}

TEST_CASE("BitStream read overflow", "[bit-stream]")
{
	// Create a 1-byte large buffer, and attempt to read a 32-bit value
	auto bit_buffer = BitBuffer(1);
	auto bit_stream = BitStream(bit_buffer);
	try
	{
		bit_stream.read<uint32_t>();
		FAIL();
	}
	catch (runtime_error &e)
	{
		SUCCEED();
	}
}
