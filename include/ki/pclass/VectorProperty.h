#pragma once
#include <vector>
#include <memory>
#include "ki/pclass/Property.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	// Forward declare for our helpers
	template <typename ValueT>
	class VectorProperty;

	namespace detail
	{
		/**
		 * Provides default implementations for vector_object_helper.
		 */
		template <typename ValueT>
		struct default_vector_object_helper
		{
			static ValueT copy(VectorProperty<ValueT> &prop, const int index)
			{
				// In cases where ValueT is not a pointer, and does not derive from PropertyClass,
				// just call the copy constructor.
				return ValueT(prop.at(index));
			}

			static const PropertyClass *get_object(const VectorProperty<ValueT> &prop, const int index)
			{
				// ValueT does not derive from PropertyClass, and so, this property is not
				// storing an object.
				throw runtime_error(
					"Tried calling get_object() on a property that does not store an object."
				);
			}

			static void set_object(VectorProperty<ValueT> &prop,
				std::unique_ptr<PropertyClass> &object, const int index)
			{
				// ValueT does not derive from PropertyClass, and so, this property is not
				// storing an object.
				throw runtime_error(
					"Tried calling set_object() on a property that does not store an object."
				);
			}
		};

		/**
		 * Provides default implementations of copy()
		 * for static_object_helper where ValueT is a pointer type.
		 * @tparam ValueT The type of the value (as a non-pointer type).
		 */
		template <typename ValueT>
		struct pointer_vector_object_helper
		{
			static ValueT *copy(const VectorProperty<ValueT *> &prop)
			{
				// The copy constructor for all pointers is to copy the pointer
				// without creating a new copy of the object it's pointing to.
				return prop.m_value;
			}

			static const PropertyClass *get_object(
				const VectorProperty<ValueT *> &prop, const int index)
			{
				// By default, assume that ValueT is not a 
				throw runtime_error(
					"Tried calling get_object() on a property that does not store an object."
				);
			}

			static void set_object(VectorProperty<ValueT *> &prop,
				std::unique_ptr<PropertyClass> &object, const int index)
			{
				throw runtime_error(
					"Tried calling set_object() on a property that does not store an object."
				);
			}
		};

		/**
		 * A helper utility that provides the right implementation of  copy(),
		 * get_object() and set_object(), based on characteristics of type: ValueT.
		 */
		template <
			typename ValueT,
			typename Enable = void
		>
		struct vector_object_helper : default_vector_object_helper<ValueT>
		{
			using default_vector_object_helper<ValueT>::copy;
			using default_vector_object_helper<ValueT>::get_object;
			using default_vector_object_helper<ValueT>::set_object;
		};

		/**
		 * Specialization for when ValueT is:
		 * - Not a pointer; but
		 * - does derive from PropertyClass
		 */
		template <typename ValueT>
		struct vector_object_helper<
			ValueT,
			typename std::enable_if<
				std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
			: default_vector_object_helper<ValueT>
		{
			using default_vector_object_helper<ValueT>::copy;

			static const PropertyClass *get_object(
				const VectorProperty<ValueT> &prop, const int index)
			{
				// ValueT does derive from PropertyClass, and we have an instance of ValueT,
				// so we can cast down to a PropertyClass pointer.
				return dynamic_cast<const PropertyClass *>(&prop.at(index));
			}

			static void set_object(VectorProperty<ValueT> &prop,
				std::unique_ptr<PropertyClass> &object, const int index)
			{
				// Ensure that object is not nullptr
				if (!object)
					throw runtime_error("Value cannot be null.");

				// Ensure that object is exactly the type of the property.
				assert_type_match(prop.get_type(), object->get_type());

				// ValueT does derive from PropertyClass, but we don't store a pointer,
				// so we need to copy the value in.
				prop.at(index) = *dynamic_cast<ValueT *>(object.get());
			}
		};

		/**
		 * Specialization for when ValueT is:
		 * - A pointer; but
		 * - does not derive from PropertyClass
		 */
		template <typename ValueT>
		struct vector_object_helper<
			ValueT *,
			typename std::enable_if<
				!std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
			: pointer_vector_object_helper<ValueT>
		{
			using pointer_vector_object_helper<ValueT>::copy;
			using pointer_vector_object_helper<ValueT>::get_object;
			using pointer_vector_object_helper<ValueT>::set_object;
		};

		/**
		 * Specialization for when ValueT is:
		 * - A pointer; and
		 * - does derive from PropertyClass
		 */
		template <typename ValueT>
		struct vector_object_helper<
			ValueT *,
			typename std::enable_if<
				std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
			: pointer_vector_object_helper<ValueT>
		{
			using pointer_vector_object_helper<ValueT>::copy;

			static const PropertyClass *get_object(
				const VectorProperty<ValueT *> &prop, const int index)
			{
				// ValueT does derive from PropertyClass, and we have a pointer to an instance
				// of ValueT, so we can cast down to a PropertyClass pointer.
				return dynamic_cast<const PropertyClass *>(prop.at(index));
			}

			static void set_object(VectorProperty<ValueT *> &prop,
				std::unique_ptr<PropertyClass> &object, const int index)
			{
				// Ensure that object inherits the type of the property
				if (object)
					assert_type_match(prop.get_type(), object->get_type(), true);

				// ValueT does derive from PropertyClass, and we have a pointer to an instance
				// of PropertyClass, so cast the pointer up to a ValueT.
				prop.at(index) = dynamic_cast<ValueT *>(object.release());
			}
		};

		/**
		 * A helper utility that provides the right implementation of
		 * get_value() and set_value() based on whether ValueT is a pointer
		 * for vector properties.
		 */
		template <typename ValueT>
		struct vector_value_helper
		{
			static Value get_value(const VectorProperty<ValueT> &prop, const int index)
			{
				return Value::make_reference<ValueT>(prop.at(index));
			}

			static void set_value(VectorProperty<ValueT> &prop,
				Value value, const int index)
			{
				Value casted_value = value.as<ValueT>();
				prop.at(index) = casted_value.get<ValueT>();
			}
		};

		/**
		 * Specialization for when ValueT is a pointer.
		 *
		 * Dereference the pointer before creating Value instances.
		 * This is so that the Value stores a pointer to a ValueT instance,
		 * rather than storing a pointer to a pointer.
		 */
		template <typename ValueT>
		struct vector_value_helper<ValueT *>
		{
			static Value get_value(const VectorProperty<ValueT *> &prop, const int index)
			{
				ValueT *value_ptr = prop.at(index);
				if (value_ptr == nullptr)
					throw runtime_error("Called get_value() but value is nullptr.");
				return Value::make_reference<ValueT>(*value_ptr);
			}

			static void set_value(VectorProperty<ValueT *> &prop,
				Value value, const int index)
			{
				Value casted_value = value.as<ValueT>();
				prop.at(index) = casted_value.release<ValueT>();
			}
		};
	}

	/**
	 * A dynamically-sized array property.
	 */
	template <typename ValueT>
	class VectorProperty : public std::vector<ValueT>, public IProperty
	{
	public:
		// Do not allow copy assignment. Once a property has been constructed,
		// it shouldn't be able to change.
		VectorProperty<ValueT> &operator=(const VectorProperty<ValueT> &that) = delete;

		VectorProperty(PropertyClass &object,
			const std::string &name, const Type &type, IProperty::flags flags)
			: IProperty(object, name, type, flags)
		{}

		VectorProperty(PropertyClass &object,
			const VectorProperty<ValueT> &that)
			: IProperty(object, that)
		{
			// Copy vector values into this vector
			for (auto i = 0; i < this->size(); i++)
				this->push_back(detail::vector_value_helper<ValueT>::copy(*this, i));
		}

		constexpr bool is_pointer() const override
		{
			return std::is_pointer<ValueT>::value;
		}

		bool is_dynamic() const override
		{
			return true;
		}

		bool is_array() const override
		{
			return true;
		}

		std::size_t get_element_count() const override
		{
			return this->size();
		}

		void set_element_count(const std::size_t size) override
		{
			this->resize(size);
		}

		Value get_value(std::size_t index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return detail::vector_value_helper<ValueT>::get_value(*this, index);
		}

		void set_value(Value value, std::size_t index) override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return detail::vector_value_helper<ValueT>::set_value(*this, value, index);
		}

		const PropertyClass *get_object(const std::size_t index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return detail::vector_object_helper<ValueT>::get_object(*this, index);
		}

		void set_object(std::unique_ptr<PropertyClass> &object, std::size_t index) override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return detail::vector_object_helper<ValueT>::set_object(*this, object, index);
		}
	};
}
}
