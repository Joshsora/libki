#include "ki/pclass/TypeSystem.h"
#include "ki/util/BitTypes.h"
#include "ki/util/exception.h"
#include <sstream>
#include <iomanip>

#define DEFINE_INTEGER_PRIMTIIVE(st, ut, n) \
	define_primitive<st>(n); \
	define_primitive<st>("signed " n); \
	define_primitive<ut>("unsigned " n)

namespace ki
{
namespace pclass
{
	template <int N>
	void define_bit_integer_primitive(pclass::TypeSystem &type_system)
	{
		define_bit_integer_primitive<N - 1>(type_system);

		// Define the signed bit integer
		std::ostringstream oss;
		oss << "bi" << N;
		type_system.define_primitive<bi<N>>(oss.str());

		// Define the unsigned bit integer
		oss = std::ostringstream();
		oss << "bui" << N;
		type_system.define_primitive<bui<N>>(oss.str());
	}

	template <>
	void define_bit_integer_primitive<0>(TypeSystem &type_system) {}

	TypeSystem::TypeSystem(std::unique_ptr<HashCalculator> &hash_calculator)
	{
		m_hash_calculator = std::move(hash_calculator);

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
		define_bit_integer_primitive<7>(*this);
		define_primitive<bi<24>>("s24");
		define_primitive<bui<24>>("u24");

		// Define floating point types
		define_primitive<float>("float");
		define_primitive<double>("double");

		// Define string types
		define_primitive<std::string>("std::string");
		define_primitive<std::u16string>("std::wstring");

		// Define the base class for all classes
		define_class<PropertyClass>("class PropertyClass");
	}

	const HashCalculator &TypeSystem::get_hash_calculator() const
	{
		// Make sure the hash calculator isn't null
		if (m_hash_calculator == nullptr)
			throw runtime_error("TypeSystem::get_hash_calculator() called but hash calculator is null.");
		return *m_hash_calculator;
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

	void TypeSystem::define_type(std::unique_ptr<Type> type)
	{
		// Does a type with this name already exist?
		if (m_type_name_lookup.find(type->get_name()) != m_type_name_lookup.end())
		{
			// Throw an error
			std::ostringstream oss;
			oss << "Type '" << type->get_name() << "' is already defined.";
			throw runtime_error(oss.str());
		}

		// Does a type with this hash already exist?
		if (m_type_name_lookup.find(type->get_name()) != m_type_name_lookup.end())
		{
			// Throw an error
			auto &other_type = get_type(type->get_hash());
			std::ostringstream oss;
			oss << "Type hash collision between '" << type->get_name()
				<< "' and '" << other_type.get_name() << "'.";
			throw runtime_error(oss.str());
		}

		// This type is safe to add to our lookups
		m_type_name_lookup[type->get_name()] = type.get();
		m_type_hash_lookup[type->get_hash()] = type.get();
		m_types.push_back(std::move(type));
	}
}
}
