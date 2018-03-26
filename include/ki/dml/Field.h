#pragma once
#include "FieldBase.h"
#include "types.h"

namespace ki
{
namespace dml
{
	template <typename ValueT>
	class Field : public FieldBase
	{
		friend Record;
	public:
		ValueT get_value() const;
		void set_value(ValueT value);

		Field<ValueT> *clone(const Record &record) const;

		void write_to(std::ostream &ostream) const;
		void read_from(std::istream &istream);
		size_t get_size() const;
	protected:
		Field(std::string name, const Record &record)
			: FieldBase(name, record)
		{
			m_type_hash = typeid(ValueT).hash_code();
			m_value = ValueT();
		}
	private:
		ValueT m_value;
	};

	typedef Field<BYT> BytField;
	typedef Field<UBYT> UBytField;
	typedef Field<SHRT> ShrtField;
	typedef Field<USHRT> UShrtField;
	typedef Field<INT> IntField;
	typedef Field<UINT> UIntField;
	typedef Field<STR> StrField;
	typedef Field<WSTR> WStrField;
	typedef Field<FLT> FltField;
	typedef Field<DBL> DblField;
	typedef Field<GID> GidField;
}
}
