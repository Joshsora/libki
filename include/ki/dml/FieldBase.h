#pragma once
#include <string>
#include <vector>
#include <map>
#include "../util/Serializable.h"

namespace ki
{
namespace dml
{
	class Record;

	class FieldBase : public util::Serializable
	{
		friend Record;
	public:
		const Record &get_record() const;
		std::string get_name() const;
		bool is_transferable() const;

		template <typename ValueT>
		bool is_type() const
		{
			return (typeid(ValueT).hash_code() == m_type_hash);
		}
	protected:
		size_t m_type_hash;

		FieldBase(std::string name, const Record& record);

		void set_name(std::string name);
		void set_transferable(bool transferable);

		virtual FieldBase *clone(const Record &record) const = 0;
	private:
		const Record &m_record;
		std::string m_name;
		bool m_transferable;
	};

	typedef std::vector<FieldBase *> FieldList;
	typedef std::map<std::string, FieldBase *> FieldNameMap;
}
}
