#include "ki/dml/Field.h"
#include "ki/util/ValueBytes.h"

namespace ki
{
namespace dml
{
	template <>
	void BytField::write_to(std::ostream &ostream) const
	{
		ValueBytes<BYT> data;
		data.value = m_value;
		ostream.write(data.buff, sizeof(BYT));
	}

	template <>
	void BytField::read_from(std::istream &istream)
	{
		ValueBytes<BYT> data;
		istream.read(data.buff, sizeof(BYT));
		m_value = data.value;
	}

	template <>
	size_t BytField::get_size() const
	{
		return sizeof(BYT);
	}
}
}