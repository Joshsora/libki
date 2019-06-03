#define CATCH_CONFIG_MAIN
#include <cstdio>
#include <cstdint>
#include <string>
#include <catch.hpp>
#include <ki/util/unique.h>
#include <ki/pclass/TypeSystem.h>
#include <ki/pclass/PropertyClass.h>
#include <ki/pclass/StaticProperty.h>
#include <ki/pclass/VectorProperty.h>
#include <ki/serialization/BinarySerializer.h>
#include <ki/serialization/JsonSerializer.h>
#include "ki/pclass/Enum.h"
#include "ki/serialization/XmlSerializer.h"
#include "ki/serialization/FileSerializer.h"

using namespace ki;

/**
 * Structure used to test custom primitive serialization.
 */
struct Vector3D
{
	// Allow json caster to access private members
	friend pclass::detail::value_caster<Vector3D, nlohmann::json>;
	friend pclass::detail::value_caster<nlohmann::json, Vector3D>;

	// Allow string caster to access private members
	friend pclass::detail::value_caster<Vector3D, std::string>;
	friend pclass::detail::value_caster<std::string, Vector3D>;

	explicit Vector3D(
		const float x = 0.0f,
		const float y = 0.0f,
		const float z = 0.0f)
	{
		m_x = x;
		m_y = y;
		m_z = z;
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
		return m_x == that.m_x &&
			   m_y == that.m_y &&
			   m_z == that.m_z;
	}

	void write_to(BitStream &stream, const bool is_file) const
	{
		pclass::detail::primitive_type_helper<float>
			::write_to(stream, is_file, m_x);
		pclass::detail::primitive_type_helper<float>
			::write_to(stream, is_file, m_y);
		pclass::detail::primitive_type_helper<float>
			::write_to(stream, is_file, m_z);
	}

	void read_from(BitStream &stream, const bool is_file)
	{
		m_x = pclass::detail::primitive_type_helper<float>
			::read_from(stream, is_file).get<float>();
		m_y = pclass::detail::primitive_type_helper<float>
			::read_from(stream, is_file).get<float>();
		m_z = pclass::detail::primitive_type_helper<float>
			::read_from(stream, is_file).get<float>();
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
		static bool is_byte_based()
		{
			return true;
		}

		static void write_to(BitStream &stream, const bool is_file, const Vector3D &value)
		{
			value.write_to(stream, is_file);
		}

		static Value read_from(BitStream &stream, const bool is_file)
		{
			Vector3D value;
			value.read_from(stream, is_file);
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
		nlohmann::json cast_value(const Vector3D &value) const override
		{
			return {
				{ "x", value.m_x },
				{ "y", value.m_y },
				{ "z", value.m_z }
			};
		}
	};

	/**
	 * value_caster specialization for casting json object to Vector3D.
	 */
	template <>
	struct value_caster<nlohmann::json, Vector3D>
		: value_caster_impl<nlohmann::json, Vector3D>
	{
		Vector3D cast_value(const nlohmann::json &value) const override
		{
			return Vector3D(
				value["x"].get<float>(),
				value["y"].get<float>(),
				value["z"].get<float>()
			);
		}
	};

	/**
	 * value_caster specialization for casting Vector3D to std::string.
	 */
	template <>
	struct value_caster<Vector3D, std::string>
		: value_caster_impl<Vector3D, std::string>
	{
		std::string cast_value(const Vector3D &value) const override
		{
			std::ostringstream oss;
			oss << value.m_x << " "
				<< value.m_y << " "
				<< value.m_z;
			return oss.str();
		}
	};

	/**
	 * value_caster specialization for casting std::string to Vector3D.
	 */
	template <>
	struct value_caster<std::string, Vector3D>
		: value_caster_impl<std::string, Vector3D>
	{
		Vector3D cast_value(const std::string &value) const override
		{
			Vector3D result;
			std::istringstream iss(value);
			iss >> result.m_x;
			iss >> result.m_y;
			iss >> result.m_z;
			return result;
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
		INIT_PROPERTY(int_array, "int")
		INIT_PROPERTY(int_ptr_array, "int")
		INIT_PROPERTY(object, "class NestedTestObjectA")
		INIT_PROPERTY(object_ptr, "class NestedTestObject")
		INIT_PROPERTY(null_object_ptr, "class NestedTestObject")
		INIT_PROPERTY(int_vector, "int")
		INIT_PROPERTY(int_ptr_vector, "int")
		INIT_PROPERTY(object_ptr_vector, "class NestedTestObject")
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

	// Test explicitly-sized array of primitives and pointers
	pclass::StaticProperty<int[5]> int_array;
	pclass::StaticProperty<int *[5]> int_ptr_array;

	// Test writing a class type as a value
	pclass::StaticProperty<NestedTestObjectA> object;

	// Test writing a class type as a pointer
	pclass::StaticProperty<NestedTestObject *> object_ptr;
	pclass::StaticProperty<NestedTestObject *> null_object_ptr;

	// Test writing collections of primitives
	pclass::VectorProperty<int> int_vector;
	pclass::VectorProperty<int *> int_ptr_vector;

	// Test writing multiple instances of another object
	pclass::VectorProperty<NestedTestObject *> object_ptr_vector;
};

// Setup a global TypeSystem instance
std::unique_ptr<pclass::IHashCalculator> g_hash_calculator
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
#define EXPECTED_wstring u"\u1d57\u02b0\u2071\u02e2\u0020\u2071\u02e2\u0020\u1d43\u0020\u1d57\u1d49\u02e2\u1d57\u0020\u1d5b\u1d43\u02e1\u1d58\u1d49"
#define EXPECTED_float32 3.1415927410125732421875f
#define EXPECTED_float64 3.141592653589793115997963468544185161590576171875
#define EXPECTED_vector3d Vector3D(24.0f, 61.0f, 3.62f)
#define EXPECTED_int_ptr 52
#define EXPECTED_value_object_extra_value 20
#define EXPECTED_int_array_size 5
#define EXPECTED_int_vector_size 100
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

	// Configure the int array
	for (auto i = 0; i < EXPECTED_int_array_size; ++i)
	{
		object.int_array[i] = i;
		object.int_ptr_array[i] = new int(i);
	}

	// Configure the int vector
	for (auto i = 0; i < EXPECTED_int_vector_size; ++i)
	{
		object.int_vector.push_back(i);
		object.int_ptr_vector.push_back(new int(i));
	}

	// Configure nested objects
	object.object.get().extra_value = EXPECTED_value_object_extra_value;
	object.object_ptr = g_type_system->instantiate<NestedTestObject>("class NestedTestObject").release();
	object.null_object_ptr = nullptr;
	object.object_ptr_vector.push_back(
		g_type_system->instantiate<NestedTestObjectA>("class NestedTestObjectA").release()
	);
	object.object_ptr_vector.push_back(
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

	// Validate the int array
	REQUIRE(object.int_array.get_element_count() == EXPECTED_int_array_size);
	REQUIRE(object.int_ptr_array.get_element_count() == EXPECTED_int_array_size);
	for (auto i = 0; i < EXPECTED_int_array_size; i++)
	{
		REQUIRE(object.int_array[i] == i);
		REQUIRE(*object.int_ptr_array[i] == i);
	}

	// Validate the int vector
	REQUIRE(object.int_vector.size() == EXPECTED_int_vector_size);
	REQUIRE(object.int_ptr_vector.size() == EXPECTED_int_vector_size);
	for (auto i = 0; i < EXPECTED_int_vector_size; i++)
	{
		REQUIRE(object.int_vector[i] == i);
		REQUIRE(*object.int_ptr_vector[i] == i);
	}

	// Validate nested objects
	REQUIRE(object.object.get().extra_value == EXPECTED_value_object_extra_value);
	REQUIRE(object.object_ptr.get() != nullptr);
	REQUIRE(object.object_ptr.get()->get_kind() == NestedObjectKind::OBJECT);
	REQUIRE(object.null_object_ptr.get() == nullptr);
	REQUIRE(object.object_ptr_vector.size() == 2);
	REQUIRE(object.object_ptr_vector[0]->get_kind() == NestedObjectKind::OBJECT_A);
	REQUIRE(object.object_ptr_vector[1]->get_kind() == NestedObjectKind::OBJECT_B);
}

/**
 * Conduct save/load tests with a BinarySerializer instance.
 */
void test_serializer(
	std::unique_ptr<TestObject> &test_object,
	serialization::BinarySerializer::flags flags,
	const std::string &file_suffix)
{
	BitBuffer buffer;
	BitStream stream(buffer);
	const auto start_pos = stream.tell();

	// Open the sample data
	std::ifstream sample(
		"samples/serialization/" + file_suffix + ".bin",
		std::ios::binary | std::ios::ate
	);
	REQUIRE(sample.is_open());

	// Load the sample data
	const size_t sample_size = sample.tellg();
	sample.seekg(std::ios::beg);
	auto *sample_data = new char[sample_size];
	sample.read(sample_data, sample_size);
	sample.close();

	serialization::BinarySerializer serializer(*g_type_system, false, flags);
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
 * 
 */
void test_file_serializer_load(
	std::unique_ptr<TestObject> &test_object,
	const std::string &filename)
{
	serialization::FileSerializer serializer(*g_type_system);

	// Load an object from a pre-made sample
	const auto sample_filepath = "samples/serialization/" + filename;
	std::unique_ptr<pclass::PropertyClass> object = nullptr;
	serializer.load(object, sample_filepath);

	// Set test_object so that it is validated by the caller
	REQUIRE(object != nullptr);
	test_object = std::unique_ptr<TestObject>(
		dynamic_cast<TestObject *>(object.release())
	);
	REQUIRE(test_object != nullptr);
}

/**
 *
 */
void test_file_serializer_save(
	const std::string &out_filepath,
	const std::string &sample_filepath)
{
	// Load sample data
	std::ifstream sample_ifs(
		sample_filepath,
		std::ios::binary | std::ios::ate
	);
	REQUIRE(sample_ifs.is_open());
	const auto sample_size = sample_ifs.tellg();
	sample_ifs.seekg(std::ios::beg);
	auto *sample_data = new char[sample_size];
	sample_ifs.read(sample_data, sample_size);
	sample_ifs.close();

	// Load output file that was just created
	std::ifstream output_ifs(
		out_filepath,
		std::ios::binary | std::ios::ate
	);
	REQUIRE(output_ifs.is_open());
	const auto output_size = output_ifs.tellg();
	output_ifs.seekg(std::ios::beg);
	auto *output_data = new char[output_size];
	output_ifs.read(output_data, output_size);
	output_ifs.close();

	// Validate the output
	REQUIRE(sample_size == output_size);
	REQUIRE(strncmp(sample_data, output_data, sample_size) == 0);

	// Cleanup
	delete[] sample_data;
	delete[] output_data;
}

/**
 * 
 */
void test_binary_file_serializer(
	std::unique_ptr<TestObject> &test_object,
	const serialization::BinarySerializer::flags flags,
	const std::string &filename)
{
	SECTION("Saving objects")
	{
		// Create a test object, configure it, and write it a file
		serialization::FileSerializer serializer(*g_type_system);
		test_object = g_type_system->instantiate<TestObject>("class TestObject");
		configure_test_object(*test_object);
		const auto out_filepath = "out_" + filename + ".bin";
		serializer.save_binary(test_object.get(), flags, out_filepath);
		test_file_serializer_save(
			out_filepath,
			"samples/serialization/" + filename + ".bin"
		);

		// Delete the test object here so that it is not
		// unnecessarily validated by the caller
		test_object = nullptr;
	}
	SECTION("Loading objects")
	{
		test_file_serializer_load(
			test_object, filename + ".bin");
	}
}

TEST_CASE("Serialization tests", "[serialization]")
{
	std::unique_ptr<TestObject> test_object = nullptr;
	define_types();

	SECTION("BinarySerializer")
	{
		SECTION("Without compression")
		{
			test_serializer(
				test_object,
				serialization::BinarySerializer::flags::NONE,
				"regular"
			);
		}
		SECTION("With compression")
		{
			test_serializer(
				test_object,
				serialization::BinarySerializer::flags::COMPRESSED,
				"regular_compressed"
			);
		}
	}

	SECTION("JsonSerializer")
	{
		// Open the sample data
		std::ifstream sample_file(
			"samples/serialization/regular.json",
			std::ios::binary | std::ios::ate
		);
		REQUIRE(sample_file.is_open());

		// Load the sample data into a buffer
		const size_t sample_size = sample_file.tellg();
		sample_file.seekg(std::ios::beg);
		auto *sample_data = new char[sample_size];
		sample_file.read(sample_data, sample_size);
		sample_file.close();

		// Load the sample data into a string
		const auto sample = std::string(sample_data, sample_size);
		delete[] sample_data;

		serialization::JsonSerializer serializer(*g_type_system, false);
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
			REQUIRE(object != nullptr);
			test_object = std::unique_ptr<TestObject>(
				dynamic_cast<TestObject *>(object.release())
			);
			REQUIRE(test_object != nullptr);
		}
	}

	SECTION("XmlSerializer")
	{
		// Open the sample data
		std::ifstream sample_file(
			"samples/serialization/regular.xml",
			std::ios::binary | std::ios::ate
		);
		REQUIRE(sample_file.is_open());

		// Load the sample data into a buffer
		const size_t sample_size = sample_file.tellg();
		sample_file.seekg(std::ios::beg);
		auto *sample_data = new char[sample_size];
		sample_file.read(sample_data, sample_size);
		sample_file.close();

		// Load the sample data into a string
		auto sample = std::string(sample_data, sample_size);
		delete[] sample_data;

		serialization::XmlSerializer serializer(*g_type_system);
		SECTION("Saving objects")
		{
			// Create a test object, configure it, and write it to our stream
			test_object = g_type_system->instantiate<TestObject>("class TestObject");
			configure_test_object(*test_object);
			auto xml_string = serializer.save(test_object.get());

			// Delete the test object here so that it is not
			// unnecessarily validated by the caller
			test_object = nullptr;

			// Validate the JSON string
			REQUIRE(xml_string == sample);
		}
		SECTION("Loading objects")
		{
			// Load an object from the sample
			std::unique_ptr<pclass::PropertyClass> object = nullptr;
			serializer.load(object, sample);

			// Set test_object so that it is validated by the caller
			REQUIRE(object != nullptr);
			test_object = std::unique_ptr<TestObject>(
				dynamic_cast<TestObject *>(object.release())
			);
			REQUIRE(test_object != nullptr);
		}
	}

	SECTION("FileSerializer")
	{
		SECTION("Binary")
		{
			test_binary_file_serializer(
				test_object,
				serialization::BinarySerializer::flags::NONE,
				"file"
			);
		}
		SECTION("Compressed Binary")
		{
			test_binary_file_serializer(
				test_object,
				serialization::BinarySerializer::flags::COMPRESSED,
				"file_compressed"
			);
		}
		SECTION("JSON")
		{
			SECTION("Saving objects")
			{
				// Create a test object, configure it, and write it a file
				serialization::FileSerializer serializer(*g_type_system);
				test_object = g_type_system->instantiate<TestObject>("class TestObject");
				configure_test_object(*test_object);
				const auto out_filepath = "out_file.json";
				serializer.save_json(test_object.get(), out_filepath);
				test_file_serializer_save(
					out_filepath,
					"samples/serialization/file.json"
				);

				// Delete the test object here so that it is not
				// unnecessarily validated by the caller
				test_object = nullptr;
			}
			SECTION("Loading objects")
			{
				test_file_serializer_load(test_object, "file.json");
			}
		}
		SECTION("XML")
		{
			SECTION("Saving objects")
			{
				// Create a test object, configure it, and write it a file
				serialization::FileSerializer serializer(*g_type_system);
				test_object = g_type_system->instantiate<TestObject>("class TestObject");
				configure_test_object(*test_object);
				const auto out_filepath = "out_file.xml";
				serializer.save_xml(test_object.get(), out_filepath);
				test_file_serializer_save(
					out_filepath,
					"samples/serialization/file.xml"					
				);

				// Delete the test object here so that it is not
				// unnecessarily validated by the caller
				test_object = nullptr;
			}
			SECTION("Loading objects")
			{
				test_file_serializer_load(test_object, "file.xml");
			}
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
