#pragma once
#include "ki/pclass/Property.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	// Forward declare StaticProperty<ValueT> for our helpers
	template <typename ValueT>
	class StaticProperty;

	/**
	 * A helper utility that provides the right implementation of construct()
	 * and get_object() based on characteristics of type: ValueT.
	 */
	template <
		typename ValueT,
		typename IsPointerEnable = void,
		typename IsBaseEnable = void
	>
	struct value_object_helper
	{
		static ValueT construct(const Type &type)
		{
			// In cases where ValueT is not a pointer, and does not derive from PropertyClass,
			// just call the default constructor.
			return ValueT();
		}

		static const PropertyClass *get_object(const StaticProperty<ValueT> &prop)
		{
			// ValueT does not derive from PropertyClass, and so, this property is not
			// storing an object.
			throw runtime_error(
				"Tried calling get_object() on a property that does not store an object."
			);
		}
	};

	/**
	 * Specialization for when ValueT is:
	 * - A pointer; but
	 * - does not derive from PropertyClass
	 * 
	 * This should construct to a nullptr, and throw an exception
	 * when get_object()is called.
	 */
	template <typename ValueT>
	struct value_object_helper<
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
		static ValueT construct(const Type &type)
		{
			// The default value of pointers is null
			return nullptr;
		}

		static const PropertyClass *get_object(const StaticProperty<ValueT> &prop)
		{
			// ValueT does not derive from PropertyClass, and so, this property is not
			// storing an object.
			throw runtime_error(
				"Tried calling get_object() on a property that does not store an object."
			);
		}
	};

	/**
	 * Specialization for when ValueT is:
	 * - A pointer; and
	 * - does derive from PropertyClass
	 * 
	 * This should construct to a nullptr, and return a pointer to
	 * a ValueT instance (as a PropertyClass *) when get_object() is called.
	 */
	template <typename ValueT>
	struct value_object_helper<
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
		static ValueT construct(const Type &type)
		{
			// The default value of pointers is null
			return nullptr;
		}

		static const PropertyClass *get_object(const StaticProperty<ValueT> &prop)
		{
			// ValueT does derive from PropertyClass, and we have a pointer to an instance
			// of ValueT, so we can cast down to a PropertyClass pointer.
			return dynamic_cast<const PropertyClass *>(prop.m_value);
		}
	};

	/**
	 * Specialization for when ValueT is:
	 * - Not a pointer; and
	 * - does derive from PropertyClass
	 * 
	 * This should construct an instance of ValueT by passing the property
	 * type and the type's type system, and return a pointer to a ValueT
	 * instance (as a PropertyClass *) when get_object() is called.
	 */
	template <typename ValueT>
	struct value_object_helper<
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
		static ValueT construct(const Type &type)
		{
			// Derivitives of PropertyClass cannot have a default constructor since
			// they require their Type and TypeSystem, so we need to pass these
			// along from what the StaticProperty constructor has been given.
			return ValueT(type, type.get_type_system());
		}

		static const PropertyClass *get_object(const StaticProperty<ValueT> &prop)
		{
			// ValueT does derive from PropertyClass, and we have an instance of ValueT,
			// so we can cast down to a PropertyClass pointer.
			return dynamic_cast<const PropertyClass *>(&prop.m_value);
		}
	};
	
	/**
	 * A helper utility that provides the right implementation of write() and read()
	 * based on whether ValueT is a pointer.
	 */
	template <
		typename ValueT,
		typename IsPointerEnable = void
	>
	struct value_rw_helper
	{
		static void write(const StaticProperty<ValueT> &prop, BitStreamBase &stream)
		{
			prop.get_type().write_to(stream, prop.m_value);
		}

		static void read(StaticProperty<ValueT> &prop, BitStreamBase &stream)
		{
			prop.get_type().read_from(stream, Value(prop.m_value));
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
	struct value_rw_helper<
		ValueT,
		typename std::enable_if<
			std::is_pointer<ValueT>::value
		>::type
	>
	{
		static void write(const StaticProperty<ValueT> &prop, BitStreamBase &stream)
		{
			prop.get_type().write_to(stream, *prop.m_value);
		}

		static void read(StaticProperty<ValueT> &prop, BitStreamBase &stream)
		{
			prop.get_type().read_from(stream, Value(*prop.m_value));
		}
	};

	/**
	 * A helper utility that combines functions provided by value_object_helper
	 * and value_rw_helper into a single type.
	 */
	template <typename ValueT>
	struct value_helper
	{
		static ValueT construct(const Type &type)
		{
			return value_object_helper<ValueT>::construct(type);
		}

		static const PropertyClass *get_object(const StaticProperty<ValueT> &prop)
		{
			return value_object_helper<ValueT>::get_object(prop);
		}

		static void write(const StaticProperty<ValueT> &prop, BitStreamBase &stream)
		{
			value_rw_helper<ValueT>::write(prop, stream);
		}

		static void read(StaticProperty<ValueT> &prop, BitStreamBase &stream)
		{
			value_rw_helper<ValueT>::read(prop, stream);
		}
	};

	/**
	 * TODO: Documentation
	 */
	template <typename ValueT>
	class StaticProperty : public PropertyBase
	{
		// Allow helper utilities access to m_value
		friend value_object_helper<ValueT>;
		friend value_rw_helper<ValueT>;

	public:
		StaticProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: PropertyBase(object, name, type)
			, m_value(value_helper<ValueT>::construct(type))
		{}

		StaticProperty(PropertyClass &object,
			const std::string &name, const Type &type, ValueT value)
			: PropertyBase(object, name, type)
		{
			m_value = value;
		}

		constexpr bool is_dynamic() const override
		{
			return false;
		}

		constexpr bool is_pointer() const override
		{
			return std::is_pointer<ValueT>::value;
		}

		void write_value_to(BitStreamBase &stream) const override
		{
			value_helper<ValueT>::write(*this, stream);
		}

		void read_value_from(BitStreamBase &stream) override
		{
			value_helper<ValueT>::read(*this, stream);
		}

		const PropertyClass *get_object() const override
		{
			return value_helper<ValueT>::get_object(*this);
		}

		ValueT &get()
		{
			return m_value;
		}

		Value get_value() const override
		{
			return m_value;
		}

		operator ValueT &() const
		{
			return m_value;
		}

		ValueT *operator&() const
		{
			return &m_value;
		}

		void operator=(const ValueT &value)
		{
			m_value = value;
		}

	protected:
		ValueT m_value;
	};
}
}
