#include "ki/pclass/TypeSystem.h"
#include <sstream>
#include <iomanip>

#define DEFINE_INTEGER_PRIMTIIVE(st, ut, n) \
	s_instance->define_primitive<st>(n); \
	s_instance->define_primitive<st>("signed " n); \
	s_instance->define_primitive<ut>("unsigned " n)

namespace ki
{
namespace pclass
{
	TypeSystem& TypeSystem::get_singleton()
	{
		if (s_instance == nullptr)
		{
			// Create the static instance with the default hash calculator
			s_instance = new TypeSystem(new WizardHashCalculator());

			// Define integer types
			s_instance->define_primitive<bool>("bool");
			DEFINE_INTEGER_PRIMTIIVE(int8_t, uint8_t, "char");
			DEFINE_INTEGER_PRIMTIIVE(int8_t, uint8_t, "__int8");
			s_instance->define_primitive<int8_t>("int8_t");
			s_instance->define_primitive<uint8_t>("uint8_t");

			DEFINE_INTEGER_PRIMTIIVE(int16_t, uint16_t, "short");
			DEFINE_INTEGER_PRIMTIIVE(int16_t, uint16_t, "__int16");
			s_instance->define_primitive<int16_t>("int16_t");
			s_instance->define_primitive<uint16_t>("uint16_t");

			DEFINE_INTEGER_PRIMTIIVE(int32_t, uint32_t, "int");
			DEFINE_INTEGER_PRIMTIIVE(int32_t, uint32_t, "__int32");
			s_instance->define_primitive<int32_t>("int32_t");
			s_instance->define_primitive<uint32_t>("uint32_t");

			DEFINE_INTEGER_PRIMTIIVE(int64_t, uint64_t, "long");
			DEFINE_INTEGER_PRIMTIIVE(int64_t, uint64_t, "__int64");
			s_instance->define_primitive<int64_t>("int64_t");
			s_instance->define_primitive<uint64_t>("uint64_t");
			s_instance->define_primitive<uint64_t>("gid");

			// TODO: Define bit integer types

			// Define floating point types
			s_instance->define_primitive<float>("float");
			s_instance->define_primitive<double>("double");

			// TODO: Define bit floating point types

			// Define string types
			s_instance->define_primitive<std::string>("std::string");
			s_instance->define_primitive<std::wstring>("std::wstring");
		}

		return *s_instance;
	}

	TypeSystem::TypeSystem(HashCalculator* hash_calculator)
	{
		m_hash_calculator = hash_calculator;
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

	void TypeSystem::set_hash_calculator(HashCalculator* hash_calculator)
	{
		// Update the hash calculator
		m_hash_calculator = hash_calculator;

		// Iterate through all types and recalculate their hash
		m_type_hash_lookup.clear();
		for (auto it = m_types.begin(); it != m_types.end(); ++it)
		{
			// Calculate the new hash and update the type
			auto *type = *it;
			const auto new_hash = m_hash_calculator->calculate_type_hash(type->get_name());
			type->set_hash(new_hash);

			// Add the new hash to lookup
			m_type_hash_lookup[new_hash] = type;

			// Is this type a class?
			if (type->get_kind() == Type::kind::CLASS)
			{
				// TODO: Recalculate property hashes
			}
		}
	}

	Type& TypeSystem::get_type(const std::string &name) const
	{
		const auto it = m_type_name_lookup.find(name);
		if (it == m_type_name_lookup.end())
		{
			std::ostringstream oss;
			oss << "Could not find type with name '" << name << "'.";
			throw std::runtime_error(oss.str());
		}
		return *(it->second);
	}

	Type& TypeSystem::get_type(const hash_t hash) const
	{
		const auto it = m_type_hash_lookup.find(hash);
		if (it == m_type_hash_lookup.end())
		{
			std::ostringstream oss;
			oss << "Could not find type with hash: " <<
				std::hex << std::setw(8) << std::setfill('0') <<
				std::uppercase << hash << ".";
			throw std::runtime_error(oss.str());
		}
		return *(it->second);
	}
}
}
