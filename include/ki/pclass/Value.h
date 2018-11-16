#pragma once
#include <utility>
#include <stdexcept>
#include <typeinfo>

namespace ki
{
namespace pclass
{
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
		}

		template <typename T>
		Value(const T &value)
		{
			m_value_ptr = const_cast<void *>(static_cast<const void *>(&value));
			m_type_hash = typeid(value).hash_code();
		}

		Value(Value &&o) noexcept
			: m_value_ptr(std::move(m_value_ptr))
			, m_type_hash(std::move(o.m_type_hash))
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
			// Make sure that this is allowed
			if (!is<T>())
				throw std::runtime_error("Type mismatch in Value::get<T>() call.");
			return *static_cast<T *>(m_value_ptr);
		}

		/**
		* @return A reference to the value being held as the specified type.
		*/
		template <typename T>
		T &get()
		{
			// Make sure that this is allowed
			if (!is<T>())
				throw std::runtime_error("Type mismatch in Value::get<T>() call.");
			return *static_cast<T *>(m_value_ptr);
		}

	private:
		void *m_value_ptr;
		std::size_t m_type_hash;
	};
}
}