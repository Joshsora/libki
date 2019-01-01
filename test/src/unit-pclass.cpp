#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <memory>
#include "ki/pclass/TypeSystem.h"
#include "ki/pclass/HashCalculator.h"

using namespace ki;

PCLASS(TestClassTypeA)
{
public:
	using PropertyClass::PropertyClass;
};

PCLASS(TestClassTypeB)
{
public:
	using PropertyClass::PropertyClass;
};

TEST_CASE("TypeSystem and Type definition", "[pclass]")
{
	std::unique_ptr<pclass::IHashCalculator> hash_calculator
		= ki::make_unique<pclass::WizardHashCalculator>();
	auto type_system = ki::make_unique<pclass::TypeSystem>(hash_calculator);

	SECTION("Define class")
	{
		const auto class_name = "class TestClassTypeA";
		const auto class_hash = type_system->get_hash_calculator()
			.calculate_type_hash(class_name);

		auto &defined_class = type_system->define_class<TestClassTypeA>(class_name);
		REQUIRE(type_system->has_type(class_name));
		REQUIRE(type_system->has_type(class_hash));

		auto &retrieved_class_name = type_system->get_type(class_name);
		auto &retrieved_class_hash = type_system->get_type(class_hash);
		REQUIRE(&retrieved_class_name == &defined_class);
		REQUIRE(&retrieved_class_hash == &defined_class);
	}

	SECTION("Define derived class")
	{
		const auto base_class_name = "class TestClassTypeA";
		auto &base_class = type_system->define_class<TestClassTypeA>(base_class_name);
		REQUIRE(type_system->has_type(base_class_name));

		const auto derived_class_name = "class TestClassTypeB";
		auto &derived_class = type_system->define_class<TestClassTypeB>(derived_class_name, base_class);
		REQUIRE(type_system->has_type(derived_class_name));
	}

	SECTION("Define class with non-class ancestor")
	{
		try
		{
			// Attempt to define a type that inherits "int".
			type_system->define_class<TestClassTypeA>(
				"class TestClassTypeA", type_system->get_type("int")
			);
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}

	SECTION("Get a type that does not exist by name")
	{
		try
		{
			auto &made_up_struct = type_system->get_type("struct MadeUp");
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}

	SECTION("Get a type that does not exist by hash")
	{
		try
		{
			auto &made_up_struct = type_system->get_type(0xDEADA55);
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}

	SECTION("Define type with the same name")
	{
		try
		{
			const auto class_name = "class TestClassTypeA";
			type_system->define_class<TestClassTypeA>(class_name);
			type_system->define_class<TestClassTypeA>(class_name);
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}

	SECTION("Define type with the same name")
	{
		try
		{
			const auto class_name = "class TestClassTypeA";
			type_system->define_class<TestClassTypeA>(class_name);
			type_system->define_class<TestClassTypeA>(class_name);
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}

	SECTION("Define type with the same hash")
	{
		try
		{
			type_system->define_class<TestClassTypeA>("a");
			type_system->define_class<TestClassTypeA>("a ");
			FAIL();
		}
		catch (runtime_error &e)
		{
			SUCCEED();
		}
	}
}

TEST_CASE("PropertyClass and PropertyList", "[pclass]")
{
	// TODO: Test PropertyClass instances
}

TEST_CASE("Enums", "[pclass]")
{
	// TODO: Test Enums
}
