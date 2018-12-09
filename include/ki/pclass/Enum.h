#pragma once
#include "ki/pclass/types/EnumType.h"
#include "ki/pclass/types/PrimitiveType.h"
#include "ki/util/BitTypes.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class IEnum
	{
	public:
		explicit IEnum(const Type &type);
		virtual ~IEnum() {}

		const EnumType &get_type() const;

		virtual void write_to(BitStream &stream) const = 0;
		virtual void read_from(BitStream &stream) = 0;

	private:
		EnumType *m_type;
	};

	/**
	 * TODO: Documentation
	 */
	template <typename UnderlyingT>
	class Enum : public IEnum
	{
		// Make sure UnderlyingT is integral
		static_assert(
			is_integral<UnderlyingT>::value,
			"The underlying type of an enum must be integral."
		);

	public:
		explicit Enum(const Type &type, UnderlyingT value = 0)
			: IEnum(type)
		{
			set_value(value);
		}

		explicit Enum(const Type &type, const std::string &element_name)
			: IEnum(type)
		{
			set_value(element_name);
		}

		UnderlyingT get_value() const
		{
			return m_value;
		}

		virtual void set_value(UnderlyingT value)
		{
			if (value != 0 && !get_type().has_element(value))
			{
				std::ostringstream oss;
				oss << "Enum '" << get_type().get_name()
					<< "' has no element with value: " << value << ".";
				throw runtime_error(oss.str());
			}
			m_value = value;
		}

		void set_value(const std::string &element_name)
		{
			m_value = get_type().get_element(element_name).get_value();
		}

		void write_to(BitStream& stream) const override
		{
			PrimitiveTypeWriter<UnderlyingT>::write_to(stream, m_value);
		}

		void read_from(BitStream& stream) override
		{
			UnderlyingT value;
			PrimitiveTypeReader<UnderlyingT>::read_from(stream, value);
			set_value(value);
		}

		operator UnderlyingT() const
		{
			return get_value();
		}

		void operator=(UnderlyingT value)
		{
			set_value(value);
		}

		bool operator==(const UnderlyingT &rhs) const
		{
			return m_value == rhs;
		}

		operator std::string() const
		{
			return get_type().get_element(m_value);
		}

		void operator=(const std::string &element_name)
		{
			set_value(element_name);
		}

		bool operator==(const std::string &rhs) const
		{
			if (!get_type().has_element(rhs))
				return false;
			return m_value == get_type().get_element(rhs).get_value();
		}

		bool operator==(const Enum<UnderlyingT> &rhs) const
		{
			return get_type() == &rhs.get_type() &&
				m_value == rhs.m_value;
		}

		bool operator!=(const Enum<UnderlyingT> &rhs) const
		{
			return !(*this == rhs);
		}

	protected:
		UnderlyingT m_value;
	};
}
}