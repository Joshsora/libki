#include "ki/pclass/Type.h"
#include <stdexcept>
#include <sstream>

namespace ki
{
namespace pclass
{
	Type::Type(const std::string name, const hash_t hash)
	{
		m_name = name;
		m_hash = hash;
		m_kind = kind::NONE;
	}

	std::string Type::get_name() const
	{
		return m_name;
	}

	void Type::set_hash(const hash_t hash)
	{
		m_hash = hash;
	}

	hash_t Type::get_hash() const
	{
		return m_hash;
	}

	Type::kind Type::get_kind() const
	{
		return m_kind;
	}

	PropertyClass *Type::instantiate() const
	{
		std::ostringstream oss;
		oss << "Type '" << m_name << "' cannot be instantiated.";
		throw std::runtime_error(oss.str());
	}
}
}