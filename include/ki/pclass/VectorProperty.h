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
	 *
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

		static void set_object(VectorProperty<ValueT> &prop, PropertyClass *object, const int index)
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

		static void set_object(VectorProperty<ValueT> &prop, PropertyClass *object, const int index)
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
			ValueT *value_ptr = dynamic_cast<ValueT *>(prop.at(index).copy());
			ValueT value = *value_ptr;
			delete value_ptr;
			return value;
		}

		static const PropertyClass *get_object(const VectorProperty<ValueT> &prop, const int index)
		{
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			return dynamic_cast<PropertyClass *>(&prop.at(index));
		}

		static void set_object(VectorProperty<ValueT> &prop, PropertyClass *object, const int index)
		{
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			prop.at(index) = dynamic_cast<ValueT>(*object);
			delete object;
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
			return prop.at(index);
		}

		static const PropertyClass *get_object(const VectorProperty<ValueT> &prop, const int index)
		{
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			return dynamic_cast<PropertyClass *>(prop.at(index));
		}

		static void set_object(VectorProperty<ValueT> &prop, PropertyClass *object, const int index)
		{
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			prop.at(index) = dynamic_cast<ValueT>(object);
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
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			prop.get_type().write_to(stream, prop.at(index));
		}

		static void read_value_from(VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			prop.get_type().read_from(stream, Value(prop.at(index)));
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
		static void write_value_to(const VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			prop.get_type().write_to(stream, *prop.at(index));
		}

		static void read_value_from(VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			if (index < 0 || index >= prop.size())
				throw runtime_error("Index out of bounds.");
			prop.get_type().read_from(stream, Value(*prop.at(index)));
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

		static const PropertyClass *get_object(const VectorProperty<ValueT> &prop, const int index)
		{
			return vector_value_object_helper<ValueT>::get_object(prop, index);
		}

		static void set_object(VectorProperty<ValueT> &prop, PropertyClass *object, const int index)
		{
			vector_value_object_helper<ValueT>::set_object(prop, object, index);
		}

		static void write_value_to(const VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			vector_value_rw_helper<ValueT>::write_value_to(prop, stream, index);
		}

		static void read_value_from(VectorProperty<ValueT> &prop, BitStream &stream, const int index)
		{
			vector_value_rw_helper<ValueT>::read_value_from(prop, stream, index);
		}
	};
	/// @endcond

	template <typename ValueT>
	class VectorProperty : public std::vector<ValueT>, public DynamicPropertyBase
	{
	public:
		// Do not allow copy assignment. Once a property has been constructed,
		// it shouldn't be able to change.
		VectorProperty<ValueT> &operator=(const VectorProperty<ValueT> &that) = delete;

		VectorProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: DynamicPropertyBase(object, name, type)
		{}

		VectorProperty(PropertyClass &object,
			const VectorProperty<ValueT> &that)
			: DynamicPropertyBase(object, that)
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
			return this->at(index);
		}

		const PropertyClass *get_object(const int index) const override
		{
			return vector_value_helper<ValueT>::get_object(*this, index);
		}

		void set_object(PropertyClass *object, int index) override
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
