#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <ki/util/BitStream.h>

using namespace ki;

TEST_CASE("Write bits", "[bit-stream]")
{
	auto *bit_stream = new BitStream();

	// Write an alternating pattern of bits
	bit_stream->write(0b1, 1);
	bit_stream->write(0b10, 2);
	bit_stream->write(0b010, 3);
	bit_stream->write(0b0101, 4);
	bit_stream->write(0b10101, 5);
	bit_stream->write(0b101010, 6);
	bit_stream->write(0b0101010, 7);
	bit_stream->write(0b0101, 4);

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
	auto *sample_data = new char[size + 1] { 0 };
	sample.read(sample_data, size);
	if (strcmp(sample_data, (char *)bit_stream->data()) != 0)
		FAIL();

	// Free resources
	delete bit_stream;
	delete[] sample_data;
}

TEST_CASE("Write bytes", "[bit-stream]")
{
	auto *bit_stream = new BitStream();

	// Write an alternating pattern of bits
	bit_stream->write<uint8_t>(0x01, 8);
	bit_stream->write<uint16_t>(0x0302, 16);
	bit_stream->write<uint32_t>(0x060504, 24);
	bit_stream->write<uint32_t>(0x0A090807, 32);
	bit_stream->write<uint64_t>(0x1211100F0E0D0C0BL, 64);

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
	delete bit_stream;
	delete[] sample_data;
}

TEST_CASE("Read bits", "[bit-stream]")
{
	auto *bit_stream = new BitStream();

	// Validate what we've got here with a hand-written sample
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
	if (bit_stream->read<uint8_t>(1) != 0b1)
		FAIL();
	if (bit_stream->read<uint8_t>(2) != 0b10)
		FAIL();
	if (bit_stream->read<uint8_t>(3) != 0b010)
		FAIL();
	if (bit_stream->read<uint8_t>(4) != 0b0101)
		FAIL();
	if (bit_stream->read<uint8_t>(5) != 0b10101)
		FAIL();
	if (bit_stream->read<uint8_t>(6) != 0b101010)
		FAIL();
	if (bit_stream->read<uint8_t>(7) != 0b0101010)
		FAIL();
	if (bit_stream->read<uint8_t>(4) != 0b0101)
		FAIL();

	// Free resources
	delete bit_stream;
}

TEST_CASE("Read bytes", "[bit-stream]")
{
	auto *bit_stream = new BitStream();

	// Validate what we've got here with a hand-written sample
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
	if (bit_stream->read<uint8_t>(8) != 0x01)
		FAIL();
	if (bit_stream->read<uint16_t>(16) != 0x0302)
		FAIL();
	if (bit_stream->read<uint32_t>(24) != 0x060504)
		FAIL();
	if (bit_stream->read<uint32_t>(32) != 0x0A090807)
		FAIL();
	if (bit_stream->read<uint64_t>(64) != 0x1211100F0E0D0C0BU)
		FAIL();

	// Free resources
	delete bit_stream;
}
