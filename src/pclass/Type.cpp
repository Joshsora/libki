#include "ki/pclass/Type.h"
#include "ki/pclass/ClassType.h"
#include "ki/pclass/TypeSystem.h"
#include "ki/util/exception.h"
#include <sstream>

namespace ki
{
namespace pclass
{
	Type::Type(const std::string &name, const TypeSystem &type_system)
		: m_type_system(type_system)
	{
		m_name = name;
		m_hash = m_type_system
			.get_hash_calculator()
			.calculate_type_hash(name);
		m_kind = Kind::NONE;
	}

	const std::string &Type::get_name() const
	{
		return m_name;
	}

	hash_t Type::get_hash() const
	{
		return m_hash;
	}

	Type::Kind Type::get_kind() const
	{
		return m_kind;
	}

	const TypeSystem &Type::get_type_system() const
	{
		return m_type_system;
	}

	bool Type::is_byte_aligned() const
	{
		return true;
	}

	Value Type::cast(Value &value) const
	{
		std::ostringstream oss;
		oss << "Type '" << m_name << "' does not implement Type::cast.";
		throw runtime_error(oss.str());
	}

	void Type::write_to(BitStream &stream, const bool is_file, Value &value) const
	{
		std::ostringstream oss;
		oss << "Type '" << m_name << "' does not implement Type::write_to.";
		throw runtime_error(oss.str());
	}

	Value Type::read_from(BitStream &stream, const bool is_file) const
	{
		std::ostringstream oss;
		oss << "Type '" << m_name << "' does not implement Type::read_from.";
		throw runtime_error(oss.str());
	}

	std::unique_ptr<PropertyClass> Type::instantiate() const
	{
		std::ostringstream oss;
		oss << "Type '" << m_name << "' does not implement Type::instantiate.";
		throw runtime_error(oss.str());
	}

	void assert_type_match(
		const Type &expected,
		const Type &actual,
		const bool allow_inheritance
	)
	{
		// Do the types match via inheritance?
		if (allow_inheritance &&
			expected.get_kind() == Type::Kind::CLASS)
		{
			const auto &actual_class = dynamic_cast<const IClassType &>(actual);
			if (actual_class.inherits(expected))
				return;
		}

		// Do the types match exactly?
		if (&expected == &actual)
			return;

		// The types do not match
		std::ostringstream oss;
		oss << "Type mismatch. ("
			<< "expected=" << expected.get_name() << ", "
			<< "actual=" << actual.get_name() << ", "
			<< "allow_inheritance=" << allow_inheritance
			<< ")";
		throw runtime_error(oss.str());
	}
}
}
