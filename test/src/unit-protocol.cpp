#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <fstream>

#include <ki/protocol/control/SessionOffer.h>
#include <ki/protocol/control/SessionAccept.h>
#include <ki/protocol/control/ClientKeepAlive.h>
#include <ki/protocol/control/ServerKeepAlive.h>

using namespace ki::protocol;

TEST_CASE("Control Message Serialization", "[control]")
{
	std::ostringstream oss;

	SECTION("SessionOffer")
	{
		control::SessionOffer offer(0xABCD, 0xAABBCCDD, 0xAABBCCDD);
		offer.write_to(oss);

		const char expected_bytes[] = {
			// Session ID
			'\xCD', '\xAB',
			
			// Unknown
			'\x00', '\x00', '\x00', '\x00',

			// Timestamp
			'\xDD', '\xCC', '\xBB', '\xAA',

			// Milliseconds
			'\xDD', '\xCC', '\xBB', '\xAA'
		};
		REQUIRE(oss.str() == std::string(expected_bytes, sizeof(expected_bytes)));
	}

	SECTION("SessionAccept")
	{
		control::SessionAccept accept(0xABCD, 0xAABBCCDD, 0xAABBCCDD);
		accept.write_to(oss);

		const char expected_bytes[] = {
			// Unknown
			'\x00', '\x00',

			// Unknown
			'\x00', '\x00', '\x00', '\x00',

			// Timestamp
			'\xDD', '\xCC', '\xBB', '\xAA',

			// Milliseconds
			'\xDD', '\xCC', '\xBB', '\xAA',

			// Session ID
			'\xCD', '\xAB'
		};
		REQUIRE(oss.str() == std::string(expected_bytes, sizeof(expected_bytes)));
	}

	SECTION("ClientKeepAlive")
	{
		control::ClientKeepAlive keep_alive(0xABCD, 0xABCD, 0xABCD);
		keep_alive.write_to(oss);

		const char expected_bytes[] = {
			// Session ID
			'\xCD', '\xAB',

			// Milliseconds
			'\xCD', '\xAB',

			// Minutes
			'\xCD', '\xAB'
		};
		REQUIRE(oss.str() == std::string(expected_bytes, sizeof(expected_bytes)));
	}

	SECTION("ServerKeepAlive")
	{
		control::ServerKeepAlive keep_alive(0xAABBCCDD);
		keep_alive.write_to(oss);

		const char expected_bytes[] = {
			// Unknown
			'\x00', '\x00',

			// Timestamp
			'\xDD', '\xCC', '\xBB', '\xAA'
		};
		REQUIRE(oss.str() == std::string(expected_bytes, sizeof(expected_bytes)));
	}
}

TEST_CASE("Control Message Deserialization", "[control]")
{
	SECTION("SessionOffer")
	{
		const char bytes[] = {
			// Session ID
			'\xCD', '\xAB',

			// Unknown
			'\x00', '\x00', '\x00', '\x00',

			// Timestamp
			'\xDD', '\xCC', '\xBB', '\xAA',

			// Milliseconds
			'\xDD', '\xCC', '\xBB', '\xAA'
		};
		std::istringstream iss(std::string(bytes, sizeof(bytes)));

		control::SessionOffer offer;
		offer.read_from(iss);

		REQUIRE(offer.get_session_id() == 0xABCD);
		REQUIRE(offer.get_timestamp() == 0xAABBCCDD);
		REQUIRE(offer.get_milliseconds() == 0xAABBCCDD);
	}

	SECTION("SessionAccept")
	{
		const char bytes[] = {
			// Unknown
			'\x00', '\x00',

			// Unknown
			'\x00', '\x00', '\x00', '\x00',

			// Timestamp
			'\xDD', '\xCC', '\xBB', '\xAA',

			// Milliseconds
			'\xDD', '\xCC', '\xBB', '\xAA',

			// Session ID
			'\xCD', '\xAB'
		};
		std::istringstream iss(std::string(bytes, sizeof(bytes)));

		control::SessionAccept accept;
		accept.read_from(iss);

		REQUIRE(accept.get_session_id() == 0xABCD);
		REQUIRE(accept.get_timestamp() == 0xAABBCCDD);
		REQUIRE(accept.get_milliseconds() == 0xAABBCCDD);
	}

	SECTION("ClientKeepAlive")
	{
		const char bytes[] = {
			// Session ID
			'\xCD', '\xAB',

			// Milliseconds
			'\xCD', '\xAB',

			// Minutes
			'\xCD', '\xAB'
		};
		std::istringstream iss(std::string(bytes, sizeof(bytes)));

		control::ClientKeepAlive keep_alive;
		keep_alive.read_from(iss);

		REQUIRE(keep_alive.get_session_id() == 0xABCD);
		REQUIRE(keep_alive.get_milliseconds() == 0xABCD);
		REQUIRE(keep_alive.get_minutes() == 0xABCD);
	}

	SECTION("ServerKeepAlive")
	{
		const char bytes[] = {
			// Unknown
			'\x00', '\x00',

			// Timestamp
			'\xDD', '\xCC', '\xBB', '\xAA'
		};
		std::istringstream iss(std::string(bytes, sizeof(bytes)));

		control::ServerKeepAlive keep_alive;
		keep_alive.read_from(iss);

		REQUIRE(keep_alive.get_timestamp() == 0xAABBCCDD);
	}
}
