#pragma once
#include <vector>
#include "ki/pclass/Property.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	// Forward declare for our helpers
	template <typename ValueT>
	class VectorProperty;

	/// @cond DOXYGEN_SKIP
	/**
	 * A helper utility that provides the right implementation of copy(),
	 * get_object() and set_object(), based on characteristics of type: ValueT.
	 */
	template <
		typename ValueT,
		typename IsPointerEnable = void,
		typename IsBaseEnable = void
	>
	struct vector_value_object_helper
	{
		static ValueT copy(VectorProperty<ValueT> &prop, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

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
	 *
	 */
	template <typename ValueT>
	struct vector_value_object_helper<
		ValueT,
		typename std::enable_if<
			std::is_pointer<ValueT>::value
		>::type,
		typename std::enable_if<
			!std::is_base_of<
				PropertyClass,
				typename std::remove_pointer<ValueT>::type
			>::value
		>::type
	>
	{
		static ValueT copy(VectorProperty<ValueT> &prop, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			// The copy constructor for all pointers is to copy the pointer
			// without creating a new copy of the object it's pointing to.
			return prop.at(index);
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
	 *
	 */
	template <typename ValueT>
	struct vector_value_object_helper<
		ValueT,
		typename std::enable_if<
			std::is_pointer<ValueT>::value
		>::type,
		typename std::enable_if<
			std::is_base_of<
				PropertyClass,
				typename std::remove_pointer<ValueT>::type
			>::value
		>::type
	>
	{
		static ValueT copy(VectorProperty<ValueT> &prop, const int index)
		{
			// The copy constructor for all pointers is to copy the pointer
			// without creating a new copy of the object it's pointing to.
			return prop.at(index);
		}

		static const PropertyClass *get_object(const VectorProperty<ValueT> &prop, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			// ValueT does derive from PropertyClass, and we have a pointer to an instance
			// of ValueT, so we can cast down to a PropertyClass pointer.
			return dynamic_cast<PropertyClass *>(prop.at(index));
		}

		static void set_object(VectorProperty<ValueT> &prop,
			std::unique_ptr<PropertyClass> &object, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			// Ensure that object inherits the type of the property
			if (object)
				assert_type_match(prop.get_type(), object->get_type(), true);

			// ValueT does derive from PropertyClass, and we have a pointer to an instance
			// of PropertyClass, so cast the pointer up to a ValueT.
			prop.at(index) = dynamic_cast<ValueT>(object.release());
		}
	};

	/**
	 *
	 */
	template <typename ValueT>
	struct vector_value_object_helper<
		ValueT,
		typename std::enable_if<
			!std::is_pointer<ValueT>::value
		>::type,
		typename std::enable_if<
			std::is_base_of<
				PropertyClass,
				typename std::remove_pointer<ValueT>::type
			>::value
		>::type
	>
	{
		static ValueT copy(VectorProperty<ValueT> &prop, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			// Derivitives of PropertyClass implement a clone method that returns
			// a pointer to a copy.
			ValueT *value_ptr = dynamic_cast<ValueT *>(prop.at(index).copy());
			ValueT value = *value_ptr;
			delete value_ptr;
			return value;
		}

		static const PropertyClass *get_object(const VectorProperty<ValueT> &prop, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			// ValueT does derive from PropertyClass, and we have an instance of ValueT,
			// so we can cast down to a PropertyClass pointer.
			return dynamic_cast<const PropertyClass *>(&prop.at(index));
		}

		static void set_object(VectorProperty<ValueT> &prop,
			std::unique_ptr<PropertyClass> &object, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			// Ensure that object is not nullptr
			if (!object)
				throw runtime_error("Value cannot be null.");

			// Ensure that object is exactly the type of the property
			assert_type_match(prop.get_type(), object->get_type());

			// ValueT does derive from PropertyClass, but we don't store a pointer,
			// so we need to copy the value in.
			prop.at(index) = *dynamic_cast<ValueT *>(object.get());
		}
	};

	/**
	 *
	 */
	template <
		typename ValueT,
		typename IsPointerEnable = void
	>
	struct vector_value_rw_helper
	{
		static void write_value_to(const VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			prop.get_type().write_to(
				stream,
				Value::make_reference<ValueT>(prop.at(index))
			);
		}

		static void read_value_from(VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			Value value = prop.get_type().read_from(stream);
			prop.at(index) = value.get<ValueT>();
		}
	};

	/**
	 *
	 */
	template <typename ValueT>
	struct vector_value_rw_helper<
		ValueT,
		typename std::enable_if<std::is_pointer<ValueT>::value>::type
	>
	{
		using type = typename std::remove_pointer<ValueT>::type;

		static void write_value_to(const VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			prop.get_type().write_to(
				stream,
				Value::make_reference<type>(*prop.at(index))
			);
		}

		static void read_value_from(VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			// Ensure index is within bounds
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");

			Value value = prop.get_type().read_from(stream);
			ValueT &value_ref = prop.at(index);
			value_ref = value.take<type>();
		}
	};

	/**
	 *
	 */
	template <typename ValueT>
	struct vector_value_helper
	{
		static ValueT copy(VectorProperty<ValueT> &prop, const int index)
		{
			return vector_value_object_helper<ValueT>::copy(prop, index);
		}

		static const PropertyClass *get_object(const VectorProperty<ValueT> &prop,
			const int index)
		{
			return vector_value_object_helper<ValueT>::get_object(prop, index);
		}

		static void set_object(VectorProperty<ValueT> &prop, 
			std::unique_ptr<PropertyClass> &object, const int index)
		{
			vector_value_object_helper<ValueT>::set_object(prop, object, index);
		}

		static void write_value_to(const VectorProperty<ValueT> &prop,
			BitStream &stream, const int index)
		{
			vector_value_rw_helper<ValueT>::write_value_to(prop, stream, index);
		}

		static void read_value_from(VectorProperty<ValueT> &prop,
			BitStream &stream, const int index)
		{
			vector_value_rw_helper<ValueT>::read_value_from(prop, stream, index);
		}
	};
	/// @endcond

	/**
	 *
	 */
	template <typename ValueT>
	class VectorProperty : public std::vector<ValueT>, public IDynamicProperty
	{
	public:
		// Do not allow copy assignment. Once a property has been constructed,
		// it shouldn't be able to change.
		VectorProperty<ValueT> &operator=(const VectorProperty<ValueT> &that) = delete;

		VectorProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: IDynamicProperty(object, name, type)
		{}

		VectorProperty(PropertyClass &object,
			const VectorProperty<ValueT> &that)
			: IDynamicProperty(object, that)
		{
			// Copy vector values into this vector
			for (auto i = 0; i < this->size(); i++)
				this->push_back(vector_value_helper<ValueT>::copy(*this, i));
		}

		constexpr bool is_pointer() const override
		{
			return std::is_pointer<ValueT>::value;
		}

		std::size_t get_element_count() const override
		{
			return this->size();
		}

		void set_element_count(const std::size_t size) override
		{
			this->resize(size);
		}

		Value get_value(int index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return Value::make_reference(this->at(index));
		}

		const PropertyClass *get_object(const int index) const override
		{
			return vector_value_helper<ValueT>::get_object(*this, index);
		}

		void set_object(std::unique_ptr<PropertyClass> &object, int index) override
		{
			return vector_value_helper<ValueT>::set_object(*this, object, index);
		}

		void write_value_to(BitStream &stream, const int index) const override
		{
			vector_value_helper<ValueT>::write_value_to(*this, stream, index);
		}

		void read_value_from(BitStream &stream, const int index) override
		{
			vector_value_helper<ValueT>::read_value_from(*this, stream, index);
		}
	};
}
}
