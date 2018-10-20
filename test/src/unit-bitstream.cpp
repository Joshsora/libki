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
#define KI_TEST_BITSTREAM_U8 0x01
#define KI_TEST_BITSTREAM_U16 0x0302
#define KI_TEST_BITSTREAM_U24 0x060504
#define KI_TEST_BITSTREAM_U32 0x0A090807
#define KI_TEST_BITSTREAM_U64 0x1211100F0E0D0C0BL

#define KI_TEST_WRITE_BUI(n) bit_stream->write(KI_TEST_BITSTREAM_BUI##n, n)
#define KI_TEST_READ_BUI(n) bit_stream->read<uint8_t>(n) == KI_TEST_BITSTREAM_BUI##n

using namespace ki;

TEST_CASE("BitStream Functionality", "[bit-stream]")
{
	auto *bit_stream = new BitStream();

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
		auto position = bit_stream->tell();
		if (position.get_byte() != 4 || position.get_bit() != 0)
			FAIL();
		const auto size = position.get_byte();

		// Validate what we've got here with a hand-written sample
		std::ifstream sample("samples/bitstream1.bin", std::ios::binary);
		if (!sample.is_open())
			FAIL();

		// Load the sample data and compare
		auto *sample_data = new char[size + 1]{ 0 };
		sample.read(sample_data, size);
		if (strcmp(sample_data, (char *)bit_stream->data()) != 0)
			FAIL();

		// Free resources
		delete[] sample_data;
		SUCCEED();
	}

	SECTION("Writing values with a size greater than 8 bits")
	{
		// Write some values
		bit_stream->write<uint8_t>(KI_TEST_BITSTREAM_U8, 8);
		bit_stream->write<uint16_t>(KI_TEST_BITSTREAM_U16, 16);
		bit_stream->write<uint32_t>(KI_TEST_BITSTREAM_U24, 24);
		bit_stream->write<uint32_t>(KI_TEST_BITSTREAM_U32, 32);
		bit_stream->write<uint64_t>(KI_TEST_BITSTREAM_U64, 64);

		// Make sure tell is reporting the right position
		auto position = bit_stream->tell();
		if (position.get_byte() != 18 || position.get_bit() != 0)
			FAIL();
		const auto size = position.get_byte();

		// Validate what we've got here with a hand-written sample
		std::ifstream sample("samples/bitstream2.bin", std::ios::binary);
		if (!sample.is_open())
			FAIL();

		// Load the sample data and compare
		auto *sample_data = new char[size + 1]{ 0 };
		sample.read(sample_data, size);
		if (strcmp(sample_data, (char *)bit_stream->data()) != 0)
			FAIL();

		// Free resources
		delete[] sample_data;
		SUCCEED();
	}

	SECTION("Reading values with a size lower than 8 bits")
	{
		// Open a previously hand-written sample
		std::ifstream sample("samples/bitstream1.bin", std::ios::binary);
		if (!sample.is_open())
			FAIL();

		// Load the sample data into the bit stream
		const auto begin = sample.tellg();
		sample.seekg(0, std::ios::end);
		const auto end = sample.tellg();
		const size_t size = end - begin;
		sample.seekg(std::ios::beg);
		sample.read((char *)bit_stream->data(), size);

		// Read the values and check they are what we are expecting
		if (!KI_TEST_READ_BUI(1))
			FAIL();
		if (!KI_TEST_READ_BUI(2))
			FAIL();
		if (!KI_TEST_READ_BUI(3))
			FAIL();
		if (!KI_TEST_READ_BUI(4))
			FAIL();
		if (!KI_TEST_READ_BUI(5))
			FAIL();
		if (!KI_TEST_READ_BUI(6))
			FAIL();
		if (!KI_TEST_READ_BUI(7))
			FAIL();
		if (!KI_TEST_READ_BUI(4))
			FAIL();

		SUCCEED();
	}

	SECTION("Reading values with a size greater than 8 bits")
	{
		// Open a previously hand-written sample
		std::ifstream sample("samples/bitstream2.bin", std::ios::binary);
		if (!sample.is_open())
			FAIL();

		// Load the sample data into the bit stream
		const auto begin = sample.tellg();
		sample.seekg(0, std::ios::end);
		const auto end = sample.tellg();
		const size_t size = end - begin;
		sample.seekg(std::ios::beg);
		sample.read((char *)bit_stream->data(), size);

		// Read the values and check they are what we are expecting
		if (bit_stream->read<uint8_t>(8) != KI_TEST_BITSTREAM_U8)
			FAIL();
		if (bit_stream->read<uint16_t>(16) != KI_TEST_BITSTREAM_U16)
			FAIL();
		if (bit_stream->read<uint32_t>(24) != KI_TEST_BITSTREAM_U24)
			FAIL();
		if (bit_stream->read<uint32_t>(32) != KI_TEST_BITSTREAM_U32)
			FAIL();
		if (bit_stream->read<uint64_t>(64) != KI_TEST_BITSTREAM_U64)
			FAIL();

		SUCCEED();
	}

	SECTION("Buffer underflow")
	{
		try
		{
			// Attempt to set the position less than 0
			bit_stream->seek(BitStream::stream_pos(-1, 0));
			FAIL();
		}
		catch (std::runtime_error &e)
		{
			// An exception was thrown, which is intended behaviour
			SUCCEED();
		}
	}

	// Free resources
	delete bit_stream;
}

TEST_CASE("BitStream buffer expansion", "[bit-stream]")
{
	// Create a very small buffer that we know is going to expand
	auto *bit_stream = new BitStream(1);

	// Write a byte, and then check if the capacity grows
	bit_stream->write<uint8_t>(0x55, 8);
	if (bit_stream->capacity() != 6)
		FAIL();
	
	// Go back to the first byte, and make sure what we wrote stuck
	// around after the expansion
	bit_stream->seek(BitStream::stream_pos(0, 0));
	if (bit_stream->read<uint8_t>(8) != 0x55)
		FAIL();

	// Free resources
	delete bit_stream;
	SUCCEED();
}
