#pragma once
#include <utility>
#include <typeinfo>
#include <sstream>
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	class Value;

	namespace detail
	{
		template <typename SrcT>
		struct value_caster_helper;

		/**
		 * TODO: Documentation
		 */
		struct value_caster_base
		{
			virtual ~value_caster_base() {}
			virtual Value cast(const std::type_info &dst_type, Value &v) const = 0;

		protected:
			/**
			 * Provides a nice way for specialized casters to throw with a
			 * consistent error when casting is not possible.
			 */
			Value bad_cast(const std::type_info &src_type, const std::type_info &dst_type) const;
		};

		/**
		 * TODO: Documentation
		 */
		struct value_caster
		{
			// Allow Value to call the default constructor and make
			friend Value;

			/**
			 * @tparam DstT The cast destination type.
			 * @param[in] value The Value that is being casted to the destination type.
			 * @returns A Value with a reference that has been casted to the destination type.
			 */
			template <typename DstT>
			Value cast(const Value &value) const;
			
		private:
			value_caster_base *m_caster;

			explicit value_caster(value_caster_base *caster = nullptr)
			{
				m_caster = caster;
			}

			/**
			 * @tparam SrcT
 			 */
			template <typename SrcT>
			static value_caster make()
			{
				return value_caster(new value_caster_helper<SrcT>());
			}
		};
	}

	/**
	 * A wrapper around a void pointer that ensures type safety.
	 */
	class Value
	{
	public:
		template <typename T>
		Value(T &value)
		{
			m_value_ptr = static_cast<void *>(&value);
			m_type_hash = typeid(value).hash_code();
			m_caster = detail::value_caster::make<T>();
		}

		template <typename T>
		Value(const T &value)
		{
			m_value_ptr = const_cast<void *>(static_cast<const void *>(&value));
			m_type_hash = typeid(value).hash_code();
			m_caster = detail::value_caster::make<T>();
		}

		Value(Value &&o) noexcept
			: m_value_ptr(std::move(o.m_value_ptr))
			, m_type_hash(std::move(o.m_type_hash))
			, m_caster(std::move(o.m_caster))
		{}

		/**
		 * @return Whether or not the value being held is of type T.
		 */
		template <typename T>
		bool is() const
		{
			// Do the type hashes match?
			return m_type_hash == typeid(T).hash_code();
		}

		/**
		 * @return A reference to the value being held as the specified type.
		 */
		template <typename T>
		const T &get() const
		{
			// Do we need to attempt casting?
			if (!is<T>())
				return m_caster.cast<T>(*this).get<T>();
			return *static_cast<T *>(m_value_ptr);
		}

		/**
		 * @return A reference to the value being held as the specified type.
		 */
		template <typename T>
		T &get()
		{
			// Do we need to attempt casting?
			if (!is<T>())
				return m_caster.cast<T>(*this).get<T>();
			return *static_cast<T *>(m_value_ptr);
		}

	private:
		void *m_value_ptr;
		std::size_t m_type_hash;
		detail::value_caster m_caster;
	};

	namespace detail
	{
		inline Value value_caster_base::bad_cast(
			const std::type_info& src_type, const std::type_info& dst_type) const
		{
			std::ostringstream oss;
			oss << "Cannot cast Value from " << src_type.name()
				<< " to " << dst_type.name() << ".";
			throw runtime_error(oss.str());
		}

		template <typename DstT>
		Value value_caster::cast(const Value &value) const
		{
			return m_caster->cast(typeid(DstT), const_cast<Value &>(value));
		}

		/**
		 * TODO: Documentation
		 */
		template <typename SrcT>
		struct value_caster_helper : value_caster_base
		{
			Value cast(const std::type_info &dst_type, Value& value) const override
			{
				return bad_cast(typeid(SrcT), dst_type);
			}
		};
	}
}
}
