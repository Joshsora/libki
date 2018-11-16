#pragma once
#include "ki/pclass/types/Type.h"

namespace ki
{
namespace pclass
{
	class PropertyClass;

	/**
 	 * TODO: Documentation
	 */
	class PropertyBase
	{
	public:
		PropertyBase(PropertyClass &object,
			const std::string &name, const Type &type);
		virtual ~PropertyBase() { }

		std::string get_name() const;
		hash_t get_name_hash() const;
		hash_t get_full_hash() const;
		const Type &get_type() const;

		virtual bool is_pointer() const;
		virtual bool is_dynamic() const;

		virtual Value get_value() const = 0;
		virtual const PropertyClass *get_object() const = 0;

		virtual void write_value_to(BitStream &stream) const = 0;
		virtual void read_value_from(BitStream &stream) = 0;

	private:
		std::string m_name;
		hash_t m_name_hash;
		hash_t m_full_hash;
		const Type *m_type;
	};

	/**
	 * TODO: Documentation
	 */
	class DynamicPropertyBase : public PropertyBase
	{
	public:
		DynamicPropertyBase(PropertyClass &object,
			const std::string &name, const Type &type);
		virtual ~DynamicPropertyBase() {}

		bool is_dynamic() const override;
		virtual std::size_t get_element_count() const = 0;

		Value get_value() const final override;
		const PropertyClass *get_object() const final override;
		void write_value_to(BitStream &stream) const final override;
		void read_value_from(BitStream &stream) final override;

		virtual Value get_value(int index) const = 0;
		virtual const PropertyClass *get_object(int index) const = 0;
		virtual void write_value_to(BitStream &stream, int index) const = 0;
		virtual void read_value_from(BitStream &stream, int index) = 0;
	};
}
}
