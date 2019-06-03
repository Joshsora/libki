#pragma once
#include <type_traits>
#include <unordered_map>
#include "ki/pclass/Type.h"
#include "ki/pclass/PrimitiveType.h"

namespace ki
{
namespace pclass
{
	typedef uint32_t enum_value_t;

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

			const std::string &get_name() const;
			enum_value_t get_value() const;

		private:
			std::string m_name;
			enum_value_t m_value;
		};

	public:
		// Do not allow copy construction or movement of types
		EnumType(const EnumType &that) = delete;
		EnumType &operator=(const EnumType &that) = delete;
		EnumType(EnumType &&that) noexcept = delete;
		EnumType &operator=(EnumType &&that) noexcept = delete;

		EnumType(const std::string &name, const TypeSystem &type_system);
		~EnumType();

		bool has_element(const std::string &name) const;
		bool has_element(enum_value_t value) const;

		const Element &get_element(const std::string &name) const;
		const Element &get_element(enum_value_t value) const;

		EnumType &add_element(const std::string &name, enum_value_t value);

		void write_to(BitStream &stream, const bool is_file, Value &value) const override;
		Value read_from(BitStream &stream, const bool is_file) const override;

	private:
		std::vector<Element *> m_elements;
		std::unordered_map<std::string, Element *> m_element_name_lookup;
		std::unordered_map<enum_value_t, Element *> m_element_value_lookup;
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
			m_kind = Kind::ENUM;
		}

		void write_to(BitStream &stream, const bool is_file, Value &value) const override
		{
			auto &enum_reference = value.get<EnumT>();
			auto &underlying_reference = reinterpret_cast<const underlying_type &>(enum_reference);
			detail::primitive_type_helper<underlying_type>::write_to(stream, is_file, underlying_reference);
		}

		Value read_from(BitStream &stream, const bool is_file) const override
		{
			Value read_result =
				detail::primitive_type_helper<underlying_type>::read_from(stream, is_file);
			auto underlying_value = read_result.get<underlying_type>();
			return Value::make_value<EnumT>(
				static_cast<EnumT>(underlying_value)
			);
		}
	};
}
}
