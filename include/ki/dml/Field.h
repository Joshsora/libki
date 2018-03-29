#pragma once
#include "FieldBase.h"
#include "types.h"

namespace ki
{
namespace dml
{
	template <typename ValueT>
	class Field final : public FieldBase
	{
		friend Record;
	public:
		virtual ~Field() = default;

		ValueT get_value() const
		{
			return m_value;
		}

		void set_value(ValueT value)
		{
			m_value = value;
		}

		void write_to(std::ostream &ostream) const final;
		void read_from(std::istream &istream) final;
		size_t get_size() const final;
	protected:
		Field(std::string name, const Record &record)
			: FieldBase(name, record)
		{
			m_type_hash = typeid(ValueT).hash_code();
			m_value = ValueT();
		}
	private:
		ValueT m_value;

		Field<ValueT> *clone(const Record &record) const final
		{
			auto *clone = new Field<ValueT>(m_name, record);
			clone->m_transferable = true;
			clone->m_value = m_value;
			return clone;
		}
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
