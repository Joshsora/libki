#pragma once
#include "ki/pclass/EnumType.h"
#include "ki/pclass/PrimitiveType.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class Enum
	{
	public:
		explicit Enum(const Type &type, enum_value_t value = 0);
		explicit Enum(const Type &type, const std::string &element_name);
		Enum &operator=(const Enum &that);

		const EnumType &get_type() const;

		enum_value_t get_value() const;
		void set_value(enum_value_t value);
		void set_value(const std::string &element_name);

		void write_to(BitStream &stream, const bool is_file) const;
		void read_from(BitStream &stream, const bool is_file);

		operator enum_value_t() const;
		void operator=(enum_value_t value);
		bool operator==(const enum_value_t &rhs) const;

		operator std::string() const;
		void operator=(const std::string &element_name);
		bool operator==(const std::string &rhs) const;

		bool operator==(const Enum &rhs) const;
		bool operator!=(const Enum &rhs) const;

	private:
		enum_value_t m_value;
		const EnumType *m_type;
	};
}
}