#pragma once
#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include "../util/Serializable.h"

namespace ki
{
namespace dml
{
	class Record;

	/**
	 * An abstract base class for DML fields.
	 */
	class FieldBase : public util::Serializable
	{
		friend Record;
	public:
		virtual ~FieldBase() = default;

		const Record &get_record() const;
		std::string get_name() const;
		bool is_transferable() const;

		template <typename ValueT>
		bool is_type() const
		{
			return (typeid(ValueT).hash_code() == m_type_hash);
		}
	protected:
		std::string m_name;
		bool m_transferable;
		size_t m_type_hash;

		FieldBase(std::string name, const Record& record);

		/**
		 * Returns a new Field with the same name, transferability
		 * and value but with a different owner Record.
		 */
		virtual FieldBase *clone(const Record &record) const = 0;
	private:
		const Record &m_record;
	};

	typedef std::vector<FieldBase *> FieldList;
	typedef std::map<std::string, FieldBase *> FieldNameMap;
}
}
