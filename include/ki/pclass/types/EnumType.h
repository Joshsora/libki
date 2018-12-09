#pragma once
#include <type_traits>
#include "ki/pclass/types/Type.h"

namespace ki
{
namespace pclass
{
	typedef uint64_t enum_value_t;

	/**
	 * TODO: Documentation
	 */
	class EnumType : public Type
	{
		/**
		 * TODO: Documentation
		 */
		class Element
		{
		public:
			explicit Element(const std::string &name, enum_value_t value);

			std::string get_name() const;
			enum_value_t get_value() const;

		private:
			std::string m_name;
			enum_value_t m_value;
		};

	public:
		EnumType(const std::string &name, const TypeSystem &type_system);
		~EnumType();

		bool has_element(const std::string &name) const;
		bool has_element(enum_value_t value) const;

		const Element &get_element(const std::string &name) const;
		const Element &get_element(enum_value_t value) const;

		EnumType &add_element(const std::string &name, enum_value_t value);

		void write_to(BitStream& stream, const Value& value) const override;
		void read_from(BitStream& stream, Value& value) const override;

	private:
		std::vector<Element *> m_elements;
		std::map<std::string, Element *> m_element_name_lookup;
		std::map<enum_value_t, Element *> m_element_value_lookup;
	};

	/**
	 * TODO: Documentation
	 */
	template <typename EnumT>
	class CppEnumType : public Type
	{
		// Ensure that EnumT is an enum
		static_assert(std::is_enum<EnumT>::value, "EnumT must be an enum!");
		using underlying_type = typename std::underlying_type<EnumT>::type;

	public:
		CppEnumType(const std::string &name, const TypeSystem &type_system)
			: Type(name, type_system)
		{
			m_kind = kind::ENUM;
		}

		void write_to(BitStream& stream, const Value &value) const override
		{
			PrimitiveTypeWriter<underlying_type>::write_to(
				stream, reinterpret_cast<const underlying_type &>(value.get<EnumT>())
			);
		}

		void read_from(BitStream& stream, Value &value) const override
		{
			PrimitiveTypeReader<underlying_type>::read_from(
				stream, reinterpret_cast<underlying_type &>(value.get<EnumT>())
			);
		}
	};
}
}
