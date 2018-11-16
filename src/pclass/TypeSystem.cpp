#include "ki/pclass/TypeSystem.h"
#include "ki/util/BitTypes.h"
#include "ki/util/exception.h"
#include <sstream>
#include <iomanip>

#define DEFINE_INTEGER_PRIMTIIVE(st, ut, n) \
	define_primitive<st>(n); \
	define_primitive<st>("signed " n); \
	define_primitive<ut>("unsigned " n)

#define DEFINE_BI_PRIMITIVE(n) \
	define_primitive< bi<n> >("bi" #n)

#define DEFINE_BUI_PRIMITIVE(n) \
	define_primitive< bui<n> >("bui" #n)

#define DEFINE_BIT_INTEGER_PRIMITIVE(n) \
	DEFINE_BI_PRIMITIVE(n); \
	DEFINE_BUI_PRIMITIVE(n)

namespace ki
{
namespace pclass
{
	TypeSystem::TypeSystem(HashCalculator *hash_calculator)
	{
		m_hash_calculator = hash_calculator;

		// Pre-define C++ primitive types
		// Define integer types
		define_primitive<bool>("bool");
		DEFINE_INTEGER_PRIMTIIVE(int8_t, uint8_t, "char");
		DEFINE_INTEGER_PRIMTIIVE(int8_t, uint8_t, "__int8");
		define_primitive<int8_t>("int8_t");
		define_primitive<uint8_t>("uint8_t");
		DEFINE_INTEGER_PRIMTIIVE(int16_t, uint16_t, "short");
		DEFINE_INTEGER_PRIMTIIVE(int16_t, uint16_t, "__int16");
		define_primitive<int16_t>("int16_t");
		define_primitive<uint16_t>("uint16_t");
		DEFINE_INTEGER_PRIMTIIVE(int32_t, uint32_t, "int");
		DEFINE_INTEGER_PRIMTIIVE(int32_t, uint32_t, "__int32");
		define_primitive<int32_t>("int32_t");
		define_primitive<uint32_t>("uint32_t");
		DEFINE_INTEGER_PRIMTIIVE(int64_t, uint64_t, "long");
		DEFINE_INTEGER_PRIMTIIVE(int64_t, uint64_t, "__int64");
		define_primitive<int64_t>("int64_t");
		define_primitive<uint64_t>("uint64_t");
		define_primitive<uint64_t>("gid");

		// Define bit-integer types
		DEFINE_BIT_INTEGER_PRIMITIVE(1);
		DEFINE_BIT_INTEGER_PRIMITIVE(2);
		DEFINE_BIT_INTEGER_PRIMITIVE(3);
		DEFINE_BIT_INTEGER_PRIMITIVE(4);
		DEFINE_BIT_INTEGER_PRIMITIVE(5);
		DEFINE_BIT_INTEGER_PRIMITIVE(6);
		DEFINE_BIT_INTEGER_PRIMITIVE(7);
		define_primitive<bi<24>>("s24");
		define_primitive<bui<24>>("u24");

		// Define floating point types
		define_primitive<float>("float");
		define_primitive<double>("double");

		// Define string types
		define_primitive<std::string>("std::string");
		define_primitive<std::wstring>("std::wstring");

		// Define the base class for all classes
		define_class<PropertyClass>("class PropertyClass");
	}

	TypeSystem::~TypeSystem()
	{
		// Delete all type declarations
		for (auto it = m_types.begin(); it != m_types.end(); ++it)
			delete *it;

		// Clear lookups
		m_type_name_lookup.clear();
		m_type_hash_lookup.clear();

		// Delete the hash calculator
		delete m_hash_calculator;
	}

	const HashCalculator &TypeSystem::get_hash_calculator() const
	{
		// Make sure the hash calculator isn't null
		if (m_hash_calculator == nullptr)
			throw runtime_error("TypeSystem::get_hash_calculator() called but hash calculator is null.");

		return *m_hash_calculator;
	}

	void TypeSystem::set_hash_calculator(HashCalculator* hash_calculator)
	{
		// Update the hash calculator
		m_hash_calculator = hash_calculator;

		// Iterate through all types and recalculate their hash
		m_type_hash_lookup.clear();
		for (auto it = m_types.begin(); it != m_types.end(); ++it)
		{
			// Add the new hash to lookup
			auto *type = *it;
			type->update_hash();
			m_type_hash_lookup[type->get_hash()] = type;
		}
	}

	bool TypeSystem::has_type(const std::string &name) const
	{
		return m_type_name_lookup.find(name) != m_type_name_lookup.end();
	}

	bool TypeSystem::has_type(const hash_t hash) const
	{
		return m_type_hash_lookup.find(hash) != m_type_hash_lookup.end();
	}

	const Type &TypeSystem::get_type(const std::string &name) const
	{
		const auto it = m_type_name_lookup.find(name);
		if (it == m_type_name_lookup.end())
		{
			std::ostringstream oss;
			oss << "Could not find type with name '" << name << "'.";
			throw runtime_error(oss.str());
		}
		return *it->second;
	}

	const Type &TypeSystem::get_type(const hash_t hash) const
	{
		const auto it = m_type_hash_lookup.find(hash);
		if (it == m_type_hash_lookup.end())
		{
			std::ostringstream oss;
			oss << "Could not find type with hash: 0x" <<
				std::hex << std::setw(8) << std::setfill('0') <<
				std::uppercase << hash << ".";
			throw runtime_error(oss.str());
		}
		return *it->second;
	}

	void TypeSystem::define_type(Type *type)
	{
		// Does a type with this name already exist?
		if (m_type_name_lookup.find(type->get_name()) != m_type_name_lookup.end())
		{
			// This pointer will become lost since it isn't being added to the lookups.
			delete type;

			// Throw an error
			std::ostringstream oss;
			oss << "Type '" << type->get_name() << "' is already defined.";
			throw runtime_error(oss.str());
		}

		// Does a type with this hash already exist?
		if (m_type_name_lookup.find(type->get_name()) != m_type_name_lookup.end())
		{
			// This pointer will become lost since it isn't being added to the lookups.
			delete type;

			// Throw an error
			auto &other_type = get_type(type->get_hash());
			std::ostringstream oss;
			oss << "Type hash collision between '" << type->get_name()
				<< "' and '" << other_type.get_name() << "'.";
			throw runtime_error(oss.str());
		}

		// This type is safe to add to our lookups
		m_types.push_back(type);
		m_type_name_lookup[type->get_name()] = type;
		m_type_hash_lookup[type->get_hash()] = type;
	}
}
}
