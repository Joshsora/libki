#pragma once
#include <string>
#include "ki/pclass/Type.h"
#include "ki/pclass/HashCalculator.h"
#include "ki/pclass/Value.h"
#include "ki/util/BitStream.h"

namespace ki
{
namespace pclass
{
	class PropertyClass;

	/**
	 * A base class for properties.
	 * Provides access to meta information such as name, and type
	 * of instance members, and a mechanism to get/set their values.
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

		virtual ~IProperty() = default;

		const std::string &get_name() const;
		hash_t get_name_hash() const;
		hash_t get_full_hash() const;
		const Type &get_type() const;

		/**
		 * @returns A reference to the instance of PropertyClass that this property
		 *          belongs to.
		 */
		const PropertyClass &get_instance() const;

		/**
		 * @returns Whether or not the property's value type is a pointer.
		 */
		virtual bool is_pointer() const;

		/**
		 * @returns Whether or not the property's size is dynamic.
		 */
		virtual bool is_dynamic() const;

		/**
		 * @returns Whether or not the property's value type is capable of
		 *          holding more than one value.
		 */
		virtual bool is_array() const;

		/**
		 * @returns The number of values that the property is holding.
		 */
		virtual std::size_t get_element_count() const;

		/**
		 * @param[in] size The new number of elements.
		 * @throws ki::runtime_error If the property is not dynamically sized.
		 */
		virtual void set_element_count(std::size_t size) = 0;

		/**
		 * @param[in] index The index of the element to retrieve the value from.
		 * @returns A reference to the value at the specified index, as a Value instance.
		 */
		virtual Value get_value(std::size_t index = 0) const = 0;

		/**
		 * @param[in] value The new value.
		 * @param[in] index The index of the element to modify.
		 */
		virtual void set_value(Value value, std::size_t index = 0) = 0;

		/**
		 * @param[in] index The index of the object element to retrieve.
		 * @returns A pointer to the instance of PropertyClass at the specified index.
		 * @throws ki::runtime_error If the property's value type is not an object,
		 *                           as in, it does not inherit PropertyClass.
		 */
		virtual const PropertyClass *get_object(std::size_t index = 0) const = 0;

		/**
		 * @param[in] object A pointer to the new object value.
		 * @param[in] index The index of the object element to modify.
		 * @throws ki::runtime_error If the property's value type is not an object,
		 *                           as in, it does not inherit PropertyClass.
		 */
		virtual void set_object(std::unique_ptr<PropertyClass> &object, std::size_t index = 0) = 0;

		/**
		 * Write the value of this property's specified element to a BitStream.
		 * @param[in] stream The stream to write to.
		 * @param[in] index The index of the element to retrieve the value from
		 */
		virtual void write_value_to(BitStream &stream, std::size_t index = 0) const;

		/**
		 * Read a value from a BitStream into the specified element of this property.
		 * @param[in] stream The stream to read from.
		 * @param[in] index The index of the element to read a value into.
		 */
		virtual void read_value_from(BitStream &stream, std::size_t index = 0);

	private:
		const PropertyClass *m_instance;
		std::string m_name;
		hash_t m_name_hash;
		hash_t m_full_hash;
		const Type *m_type;
	};
}
}
