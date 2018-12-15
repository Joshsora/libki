#define CATCH_CONFIG_MAIN
#include <cstdio>
#include <cstdint>
#include <string>
#include <catch.hpp>
#include <ki/pclass/TypeSystem.h>
#include <ki/pclass/PropertyClass.h>
#include <ki/pclass/StaticProperty.h>
#include <ki/pclass/VectorProperty.h>
#include <ki/serialization/BinarySerializer.h>
#include "ki/util/unique.h"
#include "ki/serialization/JsonSerializer.h"

using namespace ki;

/**
 * Structure used to test custom primitive serialization.
 */
struct Vector3D
{
	// Allow json caster to access private members
	friend pclass::detail::value_caster<Vector3D, nlohmann::json>;
	friend pclass::detail::value_caster<nlohmann::json, Vector3D>;

	explicit Vector3D(
		const float x = 0.0f,
		const float y = 0.0f,
		const float z = 0.0f)
	{
		this->m_x = x;
		this->m_y = y;
		this->m_z = z;
	}

	Vector3D &operator=(const Vector3D &that)
	{
		m_x = that.m_x;
		m_y = that.m_y;
		m_z = that.m_z;
		return *this;
	}

	bool operator==(const Vector3D &that) const
	{
		return (
			m_x == that.m_x,
			m_y == that.m_y,
			m_z == that.m_z
		);
	}

	void write_to(BitStream &stream) const
	{
		pclass::detail::primitive_type_helper<float>
			::write_to(stream, m_x);
		pclass::detail::primitive_type_helper<float>
			::write_to(stream, m_y);
		pclass::detail::primitive_type_helper<float>
			::write_to(stream, m_z);
	}

	void read_from(BitStream &stream)
	{
		m_x = pclass::detail::primitive_type_helper<float>
			::read_from(stream).get<float>();
		m_y = pclass::detail::primitive_type_helper<float>
			::read_from(stream).get<float>();
		m_z = pclass::detail::primitive_type_helper<float>
			::read_from(stream).get<float>();
	}

private:
	float m_x;
	float m_y;
	float m_z;
};

namespace ki
{
namespace pclass
{
namespace detail
{
	/**
	 * Helper for custom primitive type (Vector3D).
	 * Provides write_to and read_from implementations for PrimitiveType.
	 */
	template <>
	struct primitive_type_helper<Vector3D>
	{
		static void write_to(BitStream &stream, const Vector3D &value)
		{
			value.write_to(stream);
		}

		static Value read_from(BitStream &stream)
		{
			Vector3D value;
			value.read_from(stream);
			return Value::make_value<Vector3D>(value);
		}
	};

	/**
	 * value_caster specialization for casting Vector3D to json object.
	 */
	template <>
	struct value_caster<Vector3D, nlohmann::json>
		: value_caster_impl<Vector3D, nlohmann::json>
	{
		Value cast(const Value &value) const override
		{
			auto &vector = value.get<Vector3D>();
			const nlohmann::json j = {
				{ "x", vector.m_x },
				{ "y", vector.m_y },
				{ "z", vector.m_z }
			};
			return Value::make_value<nlohmann::json>(j);
		}
	};

	/**
	 * value_caster specialization for casting json object to Vector3D.
	 */
	template <>
	struct value_caster<nlohmann::json, Vector3D>
		: value_caster_impl<nlohmann::json, Vector3D>
	{
		Value cast(const Value &value) const override
		{
			auto &j = value.get<nlohmann::json>();
			return Value::make_value<Vector3D>(
				Vector3D(
					j["x"].get<float>(),
					j["y"].get<float>(),
					j["z"].get<float>()
				)
			);
		}
	};
}
}
}

/**
 * Enumeration used to test enum serialization.
 */
enum class NestedObjectKind
{
	NONE = 0,
	OBJECT = 1,
	OBJECT_A = 2,
	OBJECT_B = 3
};

/**
 * A class used to test object nesting and inheritance.
 * get_kind() will always return OBJECT.
 */
PCLASS(NestedTestObject)
{
public:
	PCLASS_CONSTRUCTOR(NestedTestObject)
		INIT_PROPERTY(m_kind, "enum NestedObjectKind")
	{
		m_kind = NestedObjectKind::OBJECT;
	}

	PCLASS_COPY_CONSTRUCTOR(NestedTestObject)
		INIT_PROPERTY_COPY(m_kind)
	{}

	NestedObjectKind get_kind() const 
	{
		return m_kind;
	}

protected:
	/**
	 * Test statically defined enums.
	 */
	pclass::StaticProperty<NestedObjectKind> m_kind;
};

/**
* One of two nested objects used to test inheritance.
* get_kind() will always return OBJECT_A.
*/
DERIVED_PCLASS(NestedTestObjectA, NestedTestObject)
{
public:
	DERIVED_PCLASS_CONSTRUCTOR(NestedTestObjectA, NestedTestObject)
		INIT_PROPERTY(extra_value, "int")
	{
		m_kind = NestedObjectKind::OBJECT_A;
		extra_value = 10;
	}

	NestedTestObjectA &operator=(const NestedTestObjectA &that)
	{
		m_kind.get() = that.m_kind.get();
		extra_value.get() = that.extra_value.get();
		return *this;
	}

	// Define an extra property so that we have something to validate this object
	// more specifically.
	pclass::StaticProperty<int> extra_value;
};

/**
 * One of two nested objects used to test inheritance.
 * get_kind() will always return OBJECT_B.
 */
DERIVED_PCLASS(NestedTestObjectB, NestedTestObject)
{
public:
	DERIVED_PCLASS_CONSTRUCTOR(NestedTestObjectB, NestedTestObject)
	{
		m_kind = NestedObjectKind::OBJECT_B;
	}
};

/**
 * A class used to test object serialization.
 * Contains properties of all primitive types, as well as a
 * custom primitive type, and a user-defined class type.
 */
PCLASS(TestObject)
{
public:
	PCLASS_CONSTRUCTOR(TestObject)
		INIT_PROPERTY(int4, "bi4")
		INIT_PROPERTY(uint4, "bui4")
		INIT_PROPERTY(int8, "char")
		INIT_PROPERTY(int16, "short")
		INIT_PROPERTY(int24, "s24")
		INIT_PROPERTY(int32, "int")
		INIT_PROPERTY(int64, "long")
		INIT_PROPERTY(uint8, "unsigned char")
		INIT_PROPERTY(uint16, "unsigned short")
		INIT_PROPERTY(uint24, "u24")
		INIT_PROPERTY(uint32, "unsigned int")
		INIT_PROPERTY(uint64, "unsigned long")
		INIT_PROPERTY(string, "std::string")
		INIT_PROPERTY(wstring, "std::wstring")
		INIT_PROPERTY(float32, "float")
		INIT_PROPERTY(float64, "double")
		INIT_PROPERTY(vector3d, "struct Vector3D")
		INIT_PROPERTY(int_ptr, "int")
		INIT_PROPERTY(value_object, "class NestedTestObjectA")
		INIT_PROPERTY(not_null_object, "class NestedTestObject")
		INIT_PROPERTY(null_object, "class NestedTestObject")
		INIT_PROPERTY(collection, "int")
		INIT_PROPERTY(ptr_collection, "int")
		INIT_PROPERTY(objects, "class NestedTestObject")
	{}

	// Test signed and unsigned integers with a bit length less than 8
	pclass::StaticProperty<bi<4>> int4;
	pclass::StaticProperty<bui<4>> uint4;

	// Test signed and unsigned integers with a bit length greater than 8
	pclass::StaticProperty<int8_t> int8;
	pclass::StaticProperty<int16_t> int16;
	pclass::StaticProperty<bi<24>> int24;
	pclass::StaticProperty<int32_t> int32;
	pclass::StaticProperty<int64_t> int64;
	pclass::StaticProperty<uint8_t> uint8;
	pclass::StaticProperty<uint16_t> uint16;
	pclass::StaticProperty<bui<24>> uint24;
	pclass::StaticProperty<uint32_t> uint32;
	pclass::StaticProperty<uint64_t> uint64;

	// Test strings
	pclass::StaticProperty<std::string> string;
	pclass::StaticProperty<std::u16string> wstring;

	// Test single precision and double precision floating point integers
	pclass::StaticProperty<float> float32;
	pclass::StaticProperty<double> float64;

	// Test writing custom defined primitives
	pclass::StaticProperty<Vector3D> vector3d;

	// Test dereferencing when writing pointers to primitives
	pclass::StaticProperty<int *> int_ptr;

	// Test writing a single instance of another object
	pclass::StaticProperty<NestedTestObjectA> value_object;
	pclass::StaticProperty<NestedTestObject *> not_null_object;
	pclass::StaticProperty<NestedTestObject *> null_object;

	// Test writing collections of primitives
	pclass::VectorProperty<int> collection;
	pclass::VectorProperty<int *> ptr_collection;

	// Test writing multiple instances of another object
	pclass::VectorProperty<NestedTestObject *> objects;
};

// Setup a global TypeSystem instance
std::unique_ptr<pclass::HashCalculator> g_hash_calculator
	= ki::make_unique<pclass::WizardHashCalculator>();
auto g_type_system = ki::make_unique<pclass::TypeSystem>(g_hash_calculator);
bool g_types_defined = false;

/**
 * Populate the given TypeSystem with Types necessary to run the tests.
 */
void define_types()
{
	if (g_types_defined)
		return;

	g_type_system->define_primitive<Vector3D>("struct Vector3D");
	g_type_system->define_enum<NestedObjectKind>("enum NestedObjectKind");
	auto &nested_test_object = g_type_system->define_class<NestedTestObject>("class NestedTestObject");
	g_type_system->define_class<NestedTestObjectA>("class NestedTestObjectA", nested_test_object);
	g_type_system->define_class<NestedTestObjectB>("class NestedTestObjectB", nested_test_object);
	g_type_system->define_class<TestObject>("class TestObject");

	pclass::ValueCaster::declare<Vector3D, nlohmann::json>();
	pclass::ValueCaster::declare<nlohmann::json, Vector3D>();

	g_types_defined = true;
}

/**
 * Macros for configuring/validating TestObject instances.
 */
#define EXPECTED_int4 -6
#define EXPECTED_uint4 5
#define EXPECTED_int8 0x01
#define EXPECTED_int16 0x0203
#define EXPECTED_int24 0x040506
#define EXPECTED_int32 0x0708090A
#define EXPECTED_int64 0x0B0C0D0E0F101112
#define EXPECTED_uint8 0x01
#define EXPECTED_uint16 0x0203
#define EXPECTED_uint24 0x040506
#define EXPECTED_uint32 0x0708090A
#define EXPECTED_uint64 0x0B0C0D0E0F101112
#define EXPECTED_string "This is a test value"
#define EXPECTED_wstring u"This is a test value"
#define EXPECTED_float32 3.1415927410125732421875f
#define EXPECTED_float64 3.141592653589793115997963468544185161590576171875
#define EXPECTED_vector3d Vector3D(24.0f, 61.0f, 3.62f)
#define EXPECTED_int_ptr 52
#define EXPECTED_value_object_extra_value 20
#define EXPECTED_collection_size 100
#define SET_EXPECTED(object, identifier) object.identifier = EXPECTED_##identifier
#define IS_EXPECTED(object, identifier) object.identifier.get() == EXPECTED_##identifier

/**
 * Configure a TestObject instance with the expected values.
 */
void configure_test_object(TestObject &object)
{
	// Set primitive values
	SET_EXPECTED(object, int4);
	SET_EXPECTED(object, uint4);
	SET_EXPECTED(object, int8);
	SET_EXPECTED(object, int16);
	SET_EXPECTED(object, int24);
	SET_EXPECTED(object, int32);
	SET_EXPECTED(object, int64);
	SET_EXPECTED(object, uint8);
	SET_EXPECTED(object, uint16);
	SET_EXPECTED(object, uint24);
	SET_EXPECTED(object, uint32);
	SET_EXPECTED(object, uint64);
	SET_EXPECTED(object, string);
	SET_EXPECTED(object, wstring);
	SET_EXPECTED(object, float32);
	SET_EXPECTED(object, float64);
	SET_EXPECTED(object, vector3d);
	object.int_ptr = new int(EXPECTED_int_ptr);
	
	// Configure the collection of integers
	for (auto i = 0; i < EXPECTED_collection_size; ++i)
	{
		object.collection.push_back(i);
		object.ptr_collection.push_back(new int(i));
	}

	// Configure nested objects
	object.value_object.get().extra_value = EXPECTED_value_object_extra_value;
	object.not_null_object = g_type_system->instantiate<NestedTestObject>("class NestedTestObject").release();
	object.null_object = nullptr;
	object.objects.push_back(
		g_type_system->instantiate<NestedTestObjectA>("class NestedTestObjectA").release()
	);
	object.objects.push_back(
		g_type_system->instantiate<NestedTestObjectB>("class NestedTestObjectB").release()
	);
}

/**
 * Validate that a TestObject instance is configured with the expected values.
 */
void validate_test_object(TestObject &object)
{
	// Validate primitive values
	REQUIRE(IS_EXPECTED(object, int4));
	REQUIRE(IS_EXPECTED(object, uint4));
	REQUIRE(IS_EXPECTED(object, int8));
	REQUIRE(IS_EXPECTED(object, int16));
	REQUIRE(IS_EXPECTED(object, int24));
	REQUIRE(IS_EXPECTED(object, int32));
	REQUIRE(IS_EXPECTED(object, int64));
	REQUIRE(IS_EXPECTED(object, uint8));
	REQUIRE(IS_EXPECTED(object, uint16));
	REQUIRE(IS_EXPECTED(object, uint24));
	REQUIRE(IS_EXPECTED(object, uint32));
	REQUIRE(IS_EXPECTED(object, uint64));
	REQUIRE(IS_EXPECTED(object, string));
	REQUIRE(IS_EXPECTED(object, wstring));
	REQUIRE(IS_EXPECTED(object, float32));
	REQUIRE(IS_EXPECTED(object, float64));
	REQUIRE(IS_EXPECTED(object, vector3d));
	REQUIRE(*object.int_ptr == EXPECTED_int_ptr);

	// Validate both collections
	REQUIRE(object.collection.size() == EXPECTED_collection_size);
	REQUIRE(object.ptr_collection.size() == EXPECTED_collection_size);
	for (auto i = 0; i < EXPECTED_collection_size; i++)
	{
		REQUIRE(object.collection[i] == i);
		REQUIRE(*object.ptr_collection[i] == i);
	}

	// Validate nested objects
	REQUIRE(object.value_object.get().extra_value == EXPECTED_value_object_extra_value);
	REQUIRE(object.not_null_object.get() != nullptr);
	REQUIRE(object.not_null_object.get()->get_kind() == NestedObjectKind::OBJECT);
	REQUIRE(object.null_object.get() == nullptr);
	REQUIRE(object.objects.size() == 2);
	REQUIRE(object.objects[0]->get_kind() == NestedObjectKind::OBJECT_A);
	REQUIRE(object.objects[1]->get_kind() == NestedObjectKind::OBJECT_B);
}

/**
 * Conduct save/load tests with a BinarySerializer instance.
 */
void test_serializer(
	std::unique_ptr<TestObject> &test_object,
	serialization::BinarySerializer &serializer,
	const std::string &file_suffix)
{
	BitBuffer buffer;
	BitStream stream(buffer);
	const auto start_pos = stream.tell();

	// Open the sample data
	std::ifstream sample(
		"samples/serialization/" + file_suffix + ".bin",
		std::ios::binary
	);
	REQUIRE(sample.is_open());

	// Load the sample data
	const auto begin = sample.tellg();
	sample.seekg(0, std::ios::end);
	const auto end = sample.tellg();
	const size_t sample_size = end - begin;
	sample.seekg(std::ios::beg);
	auto *sample_data = new char[sample_size];
	sample.read(sample_data, sample_size);
	sample.close();

	SECTION("Saving objects")
	{
		// Create a test object, configure it, and write it to our stream
		test_object = g_type_system->instantiate<TestObject>("class TestObject");
		configure_test_object(*test_object);
		serializer.save(test_object.get(), stream);
		const auto end_pos = stream.tell();

		// Delete the test object here so that it is not
		// unnecessarily validated by the caller
		test_object = nullptr;

		// Validate the contents of the stream
		const auto stream_size = (end_pos - start_pos).as_bytes();
		REQUIRE(stream_size == sample_size);
		auto *stream_data = new uint8_t[stream_size];
		stream.seek(start_pos);
		stream.read_copy(stream_data, stream_size * 8);
		REQUIRE(memcmp(stream_data, sample_data, stream_size) == 0);

		// Cleanup
		delete[] stream_data;
	}

	SECTION("Loading objects")
	{
		// Write the sample data to the bit stream
		stream.write_copy(reinterpret_cast<uint8_t *>(sample_data), sample_size * 8);
		stream.seek(start_pos);

		// Load an object from the bit stream's contents
		std::unique_ptr<pclass::PropertyClass> object = nullptr;
		serializer.load(object, stream, sample_size);

		// Set test_object so that it is validated by the caller
		REQUIRE(object != nullptr);
		test_object = std::unique_ptr<TestObject>(
			dynamic_cast<TestObject *>(object.release())
		);
		REQUIRE(test_object != nullptr);
	}

	// Cleanup the sample
	delete[] sample_data;
}

/**
* Conduct save/load tests with a BinarySerializer instance.
*/
void test_serializer(
	std::unique_ptr<TestObject> &test_object,
	serialization::JsonSerializer &serializer,
	const std::string &file_suffix)
{
	// Open the sample data
	std::ifstream sample_file(
		"samples/serialization/" + file_suffix + ".json",
		std::ios::binary
	);
	REQUIRE(sample_file.is_open());

	// Load the sample data into a buffer
	const auto begin = sample_file.tellg();
	sample_file.seekg(0, std::ios::end);
	const auto end = sample_file.tellg();
	const size_t sample_size = end - begin;
	sample_file.seekg(std::ios::beg);
	auto *sample_data = new char[sample_size];
	sample_file.read(sample_data, sample_size);
	sample_file.close();

	// Load the sample data into a string
	const auto sample = std::string(sample_data, sample_size);
	delete[] sample_data;

	SECTION("Saving objects")
	{
		// Create a test object, configure it, and write it to our stream
		test_object = g_type_system->instantiate<TestObject>("class TestObject");
		configure_test_object(*test_object);
		const auto json_string = serializer.save(test_object.get());

		// Delete the test object here so that it is not
		// unnecessarily validated by the caller
		test_object = nullptr;

		// Validate the JSON string
		REQUIRE(json_string == sample);
	}

	SECTION("Loading objects")
	{
		// Load an object from the sample
		std::unique_ptr<pclass::PropertyClass> object = nullptr;
		serializer.load(object, sample);

		// Set test_object so that it is validated by the caller
		/*
		REQUIRE(object != nullptr);
		test_object = std::unique_ptr<TestObject>(
			dynamic_cast<TestObject *>(object.release())
		);
		REQUIRE(test_object != nullptr);
		*/
	}
}

TEST_CASE("Serialization tests", "[serialization]")
{
	std::unique_ptr<TestObject> test_object = nullptr;
	define_types();

	SECTION("BinarySerializer")
	{
		SECTION("Regular format without compression")
		{
			serialization::BinarySerializer serializer(
				*g_type_system.get(), false,
				serialization::BinarySerializer::flags::NONE
			);
			test_serializer(test_object, serializer, "regular");
		}
		SECTION("File format without compression")
		{
			serialization::BinarySerializer serializer(
				*g_type_system.get(), true,
				serialization::BinarySerializer::flags::WRITE_SERIALIZER_FLAGS
			);
			test_serializer(test_object, serializer, "file");
		}
		SECTION("Regular format with compression")
		{
			serialization::BinarySerializer serializer(
				*g_type_system.get(), false,
				serialization::BinarySerializer::flags::COMPRESSED
			);
			test_serializer(test_object, serializer, "regular_compressed");
		}
		SECTION("File format with compression")
		{
			serialization::BinarySerializer serializer(
				*g_type_system.get(), true,
				serialization::BinarySerializer::flags::WRITE_SERIALIZER_FLAGS |
				serialization::BinarySerializer::flags::COMPRESSED
			);
			test_serializer(test_object, serializer, "file_compressed");
		}
	}

	SECTION("JsonSerializer")
	{
		SECTION("Regular format")
		{
			serialization::JsonSerializer serializer(*g_type_system, false);
			test_serializer(test_object, serializer, "regular");
		}

		SECTION("File format")
		{
			serialization::JsonSerializer serializer(*g_type_system, true);
			test_serializer(test_object, serializer, "file");
		}
	}

	// If one of the above sections leaves test_object set, then
	// we're expected to validate it
	if (test_object)
	{
		validate_test_object(*test_object);
		test_object = nullptr;
	}
}
