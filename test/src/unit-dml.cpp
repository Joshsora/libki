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
		REQUIRE(record->add_field<SHRT>("TestField") == nullptr);
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
		} length_bytes;
		memcpy(length_bytes.buff, ss.str().c_str(), 2);
		REQUIRE(length_bytes.length == 0x4);

		char value_buff[5] = { 0 };
		memcpy(value_buff, ss.str().substr(2).c_str(), 4);
		REQUIRE(strcmp(value_buff, "TEST") == 0);
	}

	SECTION("WSTR Fields")
	{
		record->add_field<WSTR>("TestWStr")->set_value(u"TEST");
		record->write_to(ss);

		union
		{
			char buff[2];
			SHRT length;
		} length_bytes;
		memcpy(length_bytes.buff, ss.str().data(), 2);
		REQUIRE(length_bytes.length == 0x4);

		char16_t value_buff[5] = { 0 };
		memcpy(value_buff, ss.str().substr(2).data(), 8);
		std::u16string value(value_buff);
		REQUIRE(value == u"TEST");
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
		REQUIRE(ss.str() == "\xCD\xCC\xCC\xCC\xCC\x0C\x63\x40");
	}

	SECTION("GID Fields")
	{
		record->add_field<GID>("TestGid")->set_value(0x8899AABBCCDDEEFF);
		record->write_to(ss);
		REQUIRE(ss.str() == "\xFF\xEE\xDD\xCC\xBB\xAA\x99\x88");
	}

	delete record;
}

TEST_CASE("Field Deserialization", "[dml]")
{
	auto *record = new Record();
	std::stringstream ss;

	SECTION("BYT Fields")
	{
		ss.write("\xAA", 1);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<BYT>("TestByt");
		record->read_from(ss);
		REQUIRE((UBYT)field->get_value() == 0xAA);
	}

	SECTION("UBYT Fields")
	{
		ss.write("\xAA", 1);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<BYT>("TestUByt");
		record->read_from(ss);
		REQUIRE((UBYT)field->get_value() == 0xAA);
	}

	SECTION("SHRT Fields")
	{
		ss.write("\xBB\xAA", 2);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<SHRT>("TestShrt");
		record->read_from(ss);
		REQUIRE((USHRT)field->get_value() == 0xAABB);
	}

	SECTION("USHRT Fields")
	{
		ss.write("\xBB\xAA", 2);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<USHRT>("TestUShrt");
		record->read_from(ss);
		REQUIRE(field->get_value() == 0xAABB);
	}

	SECTION("INT Fields")
	{
		ss.write("\xDD\xCC\xBB\xAA", 4);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<INT>("TestInt");
		record->read_from(ss);
		REQUIRE(field->get_value() == 0xAABBCCDD);
	}

	SECTION("UINT Fields")
	{
		ss.write("\xDD\xCC\xBB\xAA", 4);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<UINT>("TestUInt");
		record->read_from(ss);
		REQUIRE(field->get_value() == 0xAABBCCDD);
	}

	SECTION("STR Fields")
	{
		ss.write("\x04\x00TEST", 6);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<STR>("TestStr");
		record->read_from(ss);
		REQUIRE(field->get_value() == "TEST");
	}

	SECTION("WSTR Fields")
	{
		char buff[10];
		memcpy(buff, "\x04\x00", sizeof(USHRT));
		memcpy(&buff[2], (char *)u"TEST", 8);
		ss.write(buff, 10);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<WSTR>("TestWStr");
		record->read_from(ss);
		REQUIRE(field->get_value() == u"TEST");
	}

	SECTION("FLT Fields")
	{
		ss.write("\x66\x66\x18\x43", 4);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<FLT>("TestFlt");
		record->read_from(ss);
		REQUIRE(field->get_value() == 152.4f);
	}

	SECTION("DBL Fields")
	{
		ss.write("\xCD\xCC\xCC\xCC\xCC\x0C\x63\x40", 8);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<DBL>("TestDbl");
		record->read_from(ss);
		REQUIRE(field->get_value() == 152.4);
	}

	SECTION("GID Fields")
	{
		ss.write("\xFF\xEE\xDD\xCC\xBB\xAA\x99\x88", 8);
		ss.seekg(std::stringstream::beg);

		auto *field = record->add_field<GID>("TestGid");
		record->read_from(ss);
		REQUIRE(field->get_value() == 0x8899AABBCCDDEEFF);
	}

	delete record;
}
