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
	class IProperty
	{
	public:
		// Do not allow copy assignment. Once a property has been constructed,
		// it shouldn't be able to change.
		virtual IProperty &operator=(const IProperty &that) = delete;

		IProperty(PropertyClass &object,
			const std::string &name, const Type &type);
		IProperty(PropertyClass &object,
			const IProperty &that);

		virtual ~IProperty() {}

		const PropertyClass &get_instance() const;
		std::string get_name() const;
		hash_t get_name_hash() const;
		hash_t get_full_hash() const;
		const Type &get_type() const;

		virtual bool is_pointer() const;
		virtual bool is_dynamic() const;
		virtual bool is_array() const;
		virtual std::size_t get_element_count() const;
		virtual void set_element_count(std::size_t size) = 0;

		virtual Value get_value(std::size_t index = 0) const = 0;
		virtual void set_value(Value value, std::size_t index = 0) = 0;

		virtual const PropertyClass *get_object(std::size_t index = 0) const = 0;
		virtual void set_object(std::unique_ptr<PropertyClass> &object, std::size_t index = 0) = 0;

		virtual void write_value_to(BitStream &stream, std::size_t index = 0) const = 0;
		virtual void read_value_from(BitStream &stream, std::size_t index = 0) = 0;

	private:
		const PropertyClass *m_instance;
		std::string m_name;
		hash_t m_name_hash;
		hash_t m_full_hash;
		const Type *m_type;
	};
}
}
