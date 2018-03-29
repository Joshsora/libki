#include "ki/dml/FieldBase.h"

namespace ki
{
namespace dml
{
	FieldBase::FieldBase(std::string name, const Record& record)
		: m_record(record)
	{
		m_name = name;
		m_transferable = true;
		m_type_hash = 0;
	}

	const Record &FieldBase::get_record() const
	{
		return m_record;
	}

	std::string FieldBase::get_name() const
	{
		return m_name;
	}

	bool FieldBase::is_transferable() const
	{
		return m_transferable;
	}
}
}
