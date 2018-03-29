#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <ki/dml/Record.h>

using namespace ki::dml;

TEST_CASE("Fields can be added to and retrieved from Records", "[dml]")
{
	auto *record = new Record();

	SECTION("Adding fields should return a new Field")
	{
		auto *field = record->add_field<BYT>("TestField");
		REQUIRE(field != nullptr);
	}

	SECTION("Adding fields with the same name and type should return the previously created Field")
	{
		auto *field = record->add_field<BYT>("TestField");
		REQUIRE(record->add_field<BYT>("TestField") == field);
	}

	SECTION("Adding fields with the same name but different types should return nullptr")
	{
		auto *field = record->add_field<BYT>("TestField");
		REQUIRE(record->add_field<SHRT>("TestField") != nullptr);
	}

	SECTION("Retreiving fields with the correct type should return the previously added Field")
	{
		auto *field = record->add_field<BYT>("TestField");
		REQUIRE(record->get_field<BYT>("TestField") == field);
	}

	SECTION("Retreiving fields with an incorrect type should return nullptr")
	{
		record->add_field<BYT>("TestField");
		REQUIRE(record->get_field<SHRT>("TestField") == nullptr);
	}

	delete record;
}

TEST_CASE("Field Serialization", "[dml]")
{
	auto *record = new Record();
	std::stringstream ss;

	SECTION("BYT Fields")
	{
		record->add_field<BYT>("TestByt")->set_value(0x56);
		record->write_to(ss);
		REQUIRE(ss.str() == "\x56");
	}

	SECTION("UBYT Fields")
	{
		record->add_field<UBYT>("TestUByt")->set_value(0xAA);
		record->write_to(ss);
		REQUIRE(ss.str() == "\xAA");
	}

	SECTION("SHRT Fields")
	{
		record->add_field<SHRT>("TestShrt")->set_value(0xAABB);
		record->write_to(ss);
		REQUIRE(ss.str() == "\xBB\xAA");
	}

	SECTION("USHRT Fields")
	{
		record->add_field<USHRT>("TestUShrt")->set_value(0xAABB);
		record->write_to(ss);
		REQUIRE(ss.str() == "\xBB\xAA");
	}

	SECTION("INT Fields")
	{
		record->add_field<INT>("TestInt")->set_value(0xAABBCCDD);
		record->write_to(ss);
		REQUIRE(ss.str() == "\xDD\xCC\xBB\xAA");
	}

	SECTION("UINT Fields")
	{
		record->add_field<UINT>("TestUInt")->set_value(0xAABBCCDD);
		record->write_to(ss);
		REQUIRE(ss.str() == "\xDD\xCC\xBB\xAA");
	}

	SECTION("STR Fields")
	{
		record->add_field<STR>("TestStr")->set_value("TEST");
		record->write_to(ss);

		union
		{
			char buff[2];
			SHRT length;
		} lengthBytes;
		memcpy(lengthBytes.buff, ss.str().data(), 2);
		REQUIRE(lengthBytes.length == 0x4);

		char valueBuff[4];
		memcpy(valueBuff, ss.str().substr(2).data(), 4);
		REQUIRE(valueBuff == "TEST");
	}

	SECTION("WSTR Fields")
	{
		record->add_field<WSTR>("TestWStr")->set_value(L"TEST");
		record->write_to(ss);

		union
		{
			char buff[2];
			SHRT length;
		} lengthBytes;
		memcpy(lengthBytes.buff, ss.str().data(), 2);
		REQUIRE(lengthBytes.length == 0x4);

		wchar_t valueBuff[4];
		memcpy(valueBuff, ss.str().substr(2).data(), 8);
		REQUIRE(valueBuff == L"TEST");
	}

	SECTION("FLT Fields")
	{
		record->add_field<FLT>("TestFlt")->set_value(152.4f);
		record->write_to(ss);
		REQUIRE(ss.str() == "\x66\x66\x18\x43");
	}

	SECTION("DBL Fields")
	{
		record->add_field<DBL>("TestDbl")->set_value(152.4);
		record->write_to(ss);
		REQUIRE(ss.str() == "\xCD\xCC\xCC\xCC\xCC\x0C\x64\x40");
	}

	SECTION("GID Fields")
	{
		record->add_field<GID>("TestGid")->set_value(0x8899AABBCCDDEEFF);
		record->write_to(ss);
		REQUIRE(ss.str() == "xFF\xEE\xDD\xCC\xBB\xAA\x99\x88");
	}

	delete record;
}

TEST_CASE("Field Deserialization", "[dml]")
{
	auto *record = new Record();
	std::stringstream ss;

	SECTION("BYT Fields")
	{
		ss.write(new char[1] { (char)0xAA }, 1);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<BYT>("TestByt");
		record->read_from(ss);
		REQUIRE((UBYT)field->get_value() == 0xAA);
	}

	delete record;
}
