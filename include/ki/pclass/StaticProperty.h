#pragma once
#include "ki/pclass/Property.h"
#include "ki/pclass/PropertyClass.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	// Forward declare StaticProperty<ValueT> for our helpers
	template <typename ValueT>
	class StaticProperty;

	namespace detail
	{
		/**
		 * Provides default implementations for static_object_helper.
		 */
		template <typename ValueT>
		struct default_static_object_helper
		{
			static ValueT construct(const Type &type)
			{
				// In cases where ValueT is not a pointer, and does not derive from PropertyClass,
				// just call the default constructor.
				return ValueT();
			}

			static ValueT copy(const StaticProperty<ValueT> &prop)
			{
				// In cases where ValueT is not a pointer, and does not derive from PropertyClass,
				// just call the copy constructor.
				return ValueT(prop.m_value);
			}

			static const PropertyClass *get_object(
				const StaticProperty<ValueT> &prop, int index)
			{
				// By default, assume that ValueT is not a 
				throw runtime_error(
					"Tried calling get_object() on a property that does not store an object."
				);
			}

			static void set_object(StaticProperty<ValueT> &prop,
				std::unique_ptr<PropertyClass> &object, int index)
			{
				throw runtime_error(
					"Tried calling set_object() on a property that does not store an object."
				);
			}
		};

		/**
		 * Specialization for when ValueT is an explicitly-sized array.
		 * Removes construct() and copy() to stop the compiler complaining about
		 * a function returning an array.
		 */
		template <typename ValueT, int N>
		struct default_static_object_helper<ValueT[N]>
		{
			static const PropertyClass *get_object(
				const StaticProperty<ValueT[N]> &prop, int index)
			{
				// By default, assume that ValueT is not a 
				throw runtime_error(
					"Tried calling get_object() on a property that does not store an object."
				);
			}

			static void set_object(StaticProperty<ValueT[N]> &prop,
				std::unique_ptr<PropertyClass> &object, int index)
			{
				throw runtime_error(
					"Tried calling set_object() on a property that does not store an object."
				);
			}
		};

		/**
		 * Provides default implementations of construct(), copy()
		 * get_object(), and set_object() for static_object_helper where
		 * ValueT is a pointer type.
		 * @tparam ValueT The type of the value (as a non-pointer type).
		 */
		template <typename ValueT>
		struct pointer_static_object_helper
		{
			static ValueT *construct(const Type &type)
			{
				// The default value of pointers is null
				return nullptr;
			}

			static ValueT *copy(const StaticProperty<ValueT *> &prop)
			{
				// The copy constructor for all pointers is to copy the pointer
				// without creating a new copy of the object it's pointing to.
				return prop.m_value;
			}

			static const PropertyClass *get_object(
				const StaticProperty<ValueT *> &prop, int index)
			{
				// By default, assume that ValueT is not a 
				throw runtime_error(
					"Tried calling get_object() on a property that does not store an object."
				);
			}

			static void set_object(StaticProperty<ValueT *> &prop,
				std::unique_ptr<PropertyClass> &object, int index)
			{
				throw runtime_error(
					"Tried calling set_object() on a property that does not store an object."
				);
			}
		};

		/**
		 * A helper utility that provides the right implementation of construct(),
		 * copy(), get_object() and set_object(), based on characteristics of type: ValueT.
		 */
		template <
			typename ValueT,
			typename Enable = void
		>
		struct static_object_helper : default_static_object_helper<ValueT>
		{
			using default_static_object_helper<ValueT>::construct;
			using default_static_object_helper<ValueT>::copy;
			using default_static_object_helper<ValueT>::get_object;
			using default_static_object_helper<ValueT>::set_object;
		};

		/**
		 * Specialization for when ValueT is:
		 * - Not a pointer; but
		 * - does derive from PropertyClass
		 */
		template <typename ValueT>
		struct static_object_helper<
			ValueT,
			typename std::enable_if<
				std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
			: default_static_object_helper<ValueT>
		{
			using default_static_object_helper<ValueT>::copy;

			static ValueT construct(const Type &type)
			{
				// Derivitives of PropertyClass cannot have a default constructor since
				// they require their Type and TypeSystem, so we need to pass these
				// along from what the StaticProperty constructor has been given.
				return ValueT(type, type.get_type_system());
			}

			static const PropertyClass *get_object(
				const StaticProperty<ValueT> &prop, int index)
			{
				// ValueT does derive from PropertyClass, and we have an instance of ValueT,
				// so we can cast down to a PropertyClass pointer.
				return dynamic_cast<const PropertyClass *>(&prop.m_value);
			}

			static void set_object(StaticProperty<ValueT> &prop,
				std::unique_ptr<PropertyClass> &object, int index)
			{
				// Ensure that object is not nullptr
				if (!object)
					throw runtime_error("Value cannot be null.");

				// Ensure that object is exactly the type of the property.
				assert_type_match(prop.get_type(), object->get_type());

				// ValueT does derive from PropertyClass, but we don't store a pointer,
				// so we need to copy the value in.
				prop.m_value = *dynamic_cast<ValueT *>(object.get());
			}
		};

		/**
		 * Specialization for when ValueT is:
		 * - A pointer; but
		 * - does not derive from PropertyClass
		 */
		template <typename ValueT>
		struct static_object_helper<
			ValueT *,
			typename std::enable_if<
				!std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
			: pointer_static_object_helper<ValueT>
		{
			using pointer_static_object_helper<ValueT>::construct;
			using pointer_static_object_helper<ValueT>::copy;
			using pointer_static_object_helper<ValueT>::get_object;
			using pointer_static_object_helper<ValueT>::set_object;
		};

		/**
		 * Specialization for when ValueT is:
		 * - A pointer; and
		 * - does derive from PropertyClass
		 */
		template <typename ValueT>
		struct static_object_helper<
			ValueT *,
			typename std::enable_if<
				std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
			: pointer_static_object_helper<ValueT>
		{
			using pointer_static_object_helper<ValueT>::construct;
			using pointer_static_object_helper<ValueT>::copy;

			static const PropertyClass *get_object(
				const StaticProperty<ValueT *> &prop, int index)
			{
				// ValueT does derive from PropertyClass, and we have a pointer to an instance
				// of ValueT, so we can cast down to a PropertyClass pointer.
				return dynamic_cast<const PropertyClass *>(prop.m_value);
			}

			static void set_object(StaticProperty<ValueT *> &prop,
				std::unique_ptr<PropertyClass> &object, int index)
			{
				// Ensure that object inherits the type of the property
				if (object)
					assert_type_match(prop.get_type(), object->get_type(), true);

				// ValueT does derive from PropertyClass, and we have a pointer to an instance
				// of PropertyClass, so cast the pointer up to a ValueT.
				prop.m_value = dynamic_cast<ValueT *>(object.release());
			}
		};

		/**
		 * Specialization for when ValueT is:
		 * - An explicitly-sized array; and
		 * - does not derive from PropertyClass
		 */
		template <typename ValueT, int N>
		struct static_object_helper<
			ValueT[N],
			typename std::enable_if<
				!std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
			: default_static_object_helper<ValueT[N]>
		{
			using default_static_object_helper<ValueT[N]>::get_object;
			using default_static_object_helper<ValueT[N]>::set_object;
		};

		/**
		 * Specialization for when ValueT is:
		 * - An explicitly-sized array of non-pointer values; and
		 * - does derive from PropertyClass
		 */
		template <typename ValueT, int N>
		struct static_object_helper<
			ValueT[N],
			typename std::enable_if<
				std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
		{
			static const PropertyClass *get_object(
				const StaticProperty<ValueT[N]> &prop, const int index)
			{
				// ValueT does derive from PropertyClass, and we have an instance of ValueT,
				// so we can cast down to a PropertyClass pointer.
				return dynamic_cast<const PropertyClass *>(&prop.m_value[index]);
			}

			static void set_object(StaticProperty<ValueT[N]> &prop,
				std::unique_ptr<PropertyClass> &object, const int index)
			{
				// Ensure that object is not nullptr
				if (!object)
					throw runtime_error("Value cannot be null.");

				// Ensure that object is exactly the type of the property.
				assert_type_match(prop.get_type(), object->get_type());

				// ValueT does derive from PropertyClass, but we don't store a pointer,
				// so we need to copy the value in.
				prop.m_value[index] = *dynamic_cast<ValueT *>(object.get());
			}
		};

		/**
		 * Specialization for when ValueT is:
		 * - An explicitly-sized array of pointer values; and
		 * - does derive from PropertyClass
		 */
		template <typename ValueT, int N>
		struct static_object_helper<
			ValueT *[N],
			typename std::enable_if<
				std::is_base_of<PropertyClass, ValueT>::value
			>::type
		>
		{
			static const PropertyClass *get_object(
				const StaticProperty<ValueT *[N]> &prop, const int index)
			{
				// ValueT does derive from PropertyClass, and we have an instance of ValueT,
				// so we can cast down to a PropertyClass pointer.
				return dynamic_cast<const PropertyClass *>(prop.m_value[index]);
			}

			static void set_object(StaticProperty<ValueT *[N]> &prop,
				std::unique_ptr<PropertyClass> &object, const int index)
			{
				// Ensure that object inherits the type of the property
				if (object)
					assert_type_match(prop.get_type(), object->get_type(), true);

				// ValueT does derive from PropertyClass, and we have a pointer to an instance
				// of PropertyClass, so cast the pointer up to a ValueT.
				prop.m_value[index] = dynamic_cast<ValueT *>(object.release());
			}
		};
	
		/**
		 * A helper utility that provides the right implementation of
		 * get_value() and set_value() based on whether ValueT is a pointer.
		 */
		template <typename ValueT>
		struct static_value_helper
		{
			static Value get_value(const StaticProperty<ValueT> &prop, int index)
			{
				return Value::make_reference<ValueT>(prop.m_value);
			}

			static void set_value(StaticProperty<ValueT> &prop, Value value, int index)
			{
				Value casted_value = value.as<ValueT>();
				prop.m_value = casted_value.get<ValueT>();
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
		struct static_value_helper<ValueT *>
		{
			static Value get_value(const StaticProperty<ValueT *> &prop, int index)
			{
				if (prop.m_value == nullptr)
					throw runtime_error("Called get_value() but value is nullptr.");
				return Value::make_reference<ValueT>(*prop.m_value);
			}

			static void set_value(StaticProperty<ValueT *> &prop, Value value, int index)
			{
				Value casted_value = value.as<ValueT>();
				prop.m_value = casted_value.release<ValueT>();
			}
		};

		/**
		* Specialization for when ValueT is an explicitly-sized array
		* of non-pointer values.
		*/
		template <typename ValueT, int N>
		struct static_value_helper<ValueT[N]>
		{
			static Value get_value(const StaticProperty<ValueT[N]> &prop, const int index)
			{
				return Value::make_reference<ValueT>(prop.m_value[index]);
			}

			static void set_value(StaticProperty<ValueT[N]> &prop, Value value, const int index)
			{
				Value casted_value = value.as<ValueT>();
				prop.m_value[index] = casted_value.get<ValueT>();
			}
		};

		/**
		 * Specialization for when ValueT is an explicitly-sized array
		 * of pointer values.
		 */
		template <typename ValueT, int N>
		struct static_value_helper<ValueT *[N]>
		{
			static Value get_value(const StaticProperty<ValueT *[N]> &prop, const int index)
			{
				return Value::make_reference<ValueT>(*prop.m_value[index]);
			}

			static void set_value(StaticProperty<ValueT *[N]> &prop, Value value, const int index)
			{
				Value casted_value = value.as<ValueT>();
				prop.m_value[index] = casted_value.release<ValueT>();
			}
		};
	}

	/**
	 * Base type for StaticProperty.
	 * This is used to remove the amount of repeated code in specializations.
	 */
	template <typename ValueT>
	class IStaticProperty : public IProperty
	{
	public:
		using IProperty::IProperty;

		// Do not allow copy assignment. Once a property has been constructed,
		// it shouldn't be able to change.
		virtual IStaticProperty<ValueT> &operator=(const IStaticProperty<ValueT> &that) = delete;

		constexpr bool is_pointer() const override
		{
			return std::is_pointer<ValueT>::value;
		}

		constexpr bool is_dynamic() const override
		{
			return false;
		}

		void set_element_count(std::size_t size) override
		{
			throw runtime_error("Tried to call set_element_count() on a property that is not dynamic.");
		}
	};

	/**
	 * A statically-sized property.
	 */
	template <typename ValueT>
	class StaticProperty : public IStaticProperty<ValueT>
	{
		// Allow helper utilities access to m_value
		friend detail::default_static_object_helper<ValueT>;
		friend detail::pointer_static_object_helper<ValueT>;
		friend detail::static_object_helper<ValueT>;
		friend detail::static_value_helper<ValueT>;

	public:
		// Do not allow copy assignment. Once a property has been constructed,
		// it shouldn't be able to change.
		StaticProperty<ValueT> &operator=(const StaticProperty<ValueT> &that) = delete;

		StaticProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: IStaticProperty<ValueT>(object, name, type)
			, m_value(detail::static_object_helper<ValueT>::construct(type))
		{}

		StaticProperty(PropertyClass &object,
			const std::string &name, const Type &type, ValueT value)
			: IStaticProperty<ValueT>(object, name, type)
		{
			m_value = value;
		}

		StaticProperty(PropertyClass &object, const StaticProperty<ValueT> &that)
			: IStaticProperty<ValueT>(object, that)
			, m_value(detail::static_object_helper<ValueT>::copy(that))
		{}

		std::size_t get_element_count() const override
		{
			return 1;
		}

		Value get_value(std::size_t index) const override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			return detail::static_value_helper<ValueT>::get_value(*this, index);
		}

		void set_value(Value value, const std::size_t index) override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			detail::static_value_helper<ValueT>::set_value(*this, value, index);
		}

		const PropertyClass *get_object(const std::size_t index) const override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			return detail::static_object_helper<ValueT>::get_object(*this, index);
		}

		void set_object(std::unique_ptr<PropertyClass> &object, const std::size_t index) override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			return detail::static_object_helper<ValueT>::set_object(*this, object, index);
		}

		ValueT &get()
		{
			return m_value;
		}

		const ValueT &get() const
		{
			return m_value;
		}

		operator ValueT &()
		{
			return m_value;
		}

		operator const ValueT &() const
		{
			return m_value;
		}

		ValueT *operator&()
		{
			return &m_value;
		}

		const ValueT *operator&() const
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

	/**
	 * Specialization of StaticProperty to support explicitly-sized arrays.
	 */
	template <typename ValueT, int N>
	class StaticProperty<ValueT[N]> : public IStaticProperty<ValueT[N]>
	{
		// Allow helper utilities access to m_value
		friend detail::static_object_helper<ValueT[N]>;
		friend detail::static_value_helper<ValueT[N]>;

	public:
		// Do not allow copy assignment. Once a property has been constructed,
		// it shouldn't be able to change.
		StaticProperty<ValueT[N]> &operator=(const StaticProperty<ValueT[N]> &that) = delete;

		StaticProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: IStaticProperty<ValueT[N]>(object, name, type)
		{
			for (auto i = 0; i < N; ++i)
				m_value[i] = detail::static_object_helper<ValueT>::construct(type);
		}

		StaticProperty(PropertyClass &object, const StaticProperty<ValueT[N]> &that)
			: IStaticProperty<ValueT[N]>(object, that)
		{
			for (auto i = 0; i < N; ++i)
				m_value[i] = that.m_value[i];
		}

		bool is_array() const override
		{
			return true;
		}

		std::size_t get_element_count() const override
		{
			return N;
		}

		Value get_value(std::size_t index) const override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			return detail::static_value_helper<ValueT[N]>::get_value(*this, index);
		}

		void set_value(Value value, const std::size_t index) override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			detail::static_value_helper<ValueT[N]>::set_value(*this, value, index);
		}

		const PropertyClass *get_object(const std::size_t index) const override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			return detail::static_object_helper<ValueT[N]>::get_object(*this, index);
		}

		void set_object(std::unique_ptr<PropertyClass> &object, const std::size_t index) override
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			return detail::static_object_helper<ValueT[N]>::set_object(*this, object, index);
		}

		ValueT &operator[](const int index)
		{
			if (index < 0 || index >= get_element_count())
				throw runtime_error("Index out of bounds.");
			return m_value[index];
		}

	protected:
		ValueT m_value[N];
	};
}
}
