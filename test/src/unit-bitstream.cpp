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
#define KI_TEST_BITSTREAM_BOOL true
#define KI_TEST_BITSTREAM_U8 0x01
#define KI_TEST_BITSTREAM_U16 0x0302
#define KI_TEST_BITSTREAM_U24 0x060504
#define KI_TEST_BITSTREAM_U32 0x0A090807
#define KI_TEST_BITSTREAM_U64 0x1211100F0E0D0C0BL

#define KI_TEST_WRITE_BUI(n) bit_stream.write<bui<n>>(KI_TEST_BITSTREAM_BUI##n)
#define KI_TEST_READ_BUI(n) bit_stream.read<bui<n>>() == KI_TEST_BITSTREAM_BUI##n

using namespace ki;

TEST_CASE("BitStream::stream_pos Functionality", "[bit-stream]")
{
	BitStream::stream_pos position(1, 4);

	SECTION("Increment single bit")
	{
		++position;
		REQUIRE(position.get_byte() == 1);
		REQUIRE(position.get_bit() == 5);
	}

	SECTION("Decrement single bit")
	{
		--position;
		REQUIRE(position.get_byte() == 1);
		REQUIRE(position.get_bit() == 3);
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

		// Make sure tell is reporting the right position
		auto position = bit_stream.tell();
		REQUIRE(position.get_byte() == 4);
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
		// Write some values
		bit_stream.write<bool>(KI_TEST_BITSTREAM_BOOL);
		bit_stream.write<uint8_t>(KI_TEST_BITSTREAM_U8);
		bit_stream.write<uint16_t>(KI_TEST_BITSTREAM_U16);
		bit_stream.write<bui<24>>(KI_TEST_BITSTREAM_U24);
		bit_stream.write<uint32_t>(KI_TEST_BITSTREAM_U32);
		bit_stream.write<uint64_t>(KI_TEST_BITSTREAM_U64);

		// Make sure tell is reporting the right position
		auto position = bit_stream.tell();
		REQUIRE(position.get_byte() == 19);
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
		REQUIRE(KI_TEST_READ_BUI(1));
		REQUIRE(KI_TEST_READ_BUI(2));
		REQUIRE(KI_TEST_READ_BUI(3));
		REQUIRE(KI_TEST_READ_BUI(4));
		REQUIRE(KI_TEST_READ_BUI(5));
		REQUIRE(KI_TEST_READ_BUI(6));
		REQUIRE(KI_TEST_READ_BUI(7));
		REQUIRE(KI_TEST_READ_BUI(4));
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
		REQUIRE(bit_stream.read<bool>() == KI_TEST_BITSTREAM_BOOL);
		REQUIRE(bit_stream.read<uint8_t>() == KI_TEST_BITSTREAM_U8);
		REQUIRE(bit_stream.read<uint16_t>() == KI_TEST_BITSTREAM_U16);
		REQUIRE(bit_stream.read<bui<24>>() == KI_TEST_BITSTREAM_U24);
		REQUIRE(bit_stream.read<uint32_t>() == KI_TEST_BITSTREAM_U32);
		REQUIRE(bit_stream.read<uint64_t>() == KI_TEST_BITSTREAM_U64);
	}

	SECTION("Overwriting values")
	{
		bit_stream.write<bui<2>>(0b11);
		bit_stream.seek(BitStream::stream_pos(0, 0));
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

/**
* TODO: Test reading outside of buffer (should throw)
* TODO: Test read/write copy on BitStream
* TODO: Test BitStreamSection
*/
