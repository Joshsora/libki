#pragma once
#include <utility>
#include <typeinfo>
#include <sstream>
#include <unordered_map>
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	class Value;
	class PropertyClass;
	class IEnum;

	namespace detail
	{
		template <
			typename SrcT, typename DestT,
			typename SrcEnable = void, typename DestEnable = void
		>
		struct value_caster;

		/**
		 * A base class for Value casters.
		 * Provides a common interface for cast()
		 */
		struct value_caster_base
		{
			virtual ~value_caster_base() {}

			/**
			 * @param[in] value The value being casted.
			 * @returns A new Value holding a casted value.
			 * @throws ki::cast_error If the cast was unsuccessful.
			 */
			virtual Value cast(const Value &value) const;
		};

		/**
		 * A base class for Value caster implementations.
		 * Provides a shortcut for throwing a cast_error.
		 */
		template <typename SrcT, typename DestT>
		struct value_caster_impl : value_caster_base
		{
		protected:
			static Value bad_cast();
		};

		/**
		 * A base class for Value deallocators.
		 * Provides a common interface for deallocate()
		 */
		struct value_deallocator_base
		{
			virtual ~value_deallocator_base() {}

			/**
			 * Deallocate a Value-owned pointer.
			 * @param[in] ptr The pointer to deallocate.
			 */
			virtual void deallocate(void *ptr) const = 0;
		};

		/**
		 * TODO: Documentation
		 */
		template <typename T>
		struct value_deallocator : value_deallocator_base
		{
			void deallocate(void *ptr) const override
			{
				// By default, now that we have the proper type, just delete it.
				delete static_cast<T *>(ptr);
			}
		};

		/**
		* TODO: Documentation
		*/
		class ValueDeallocator
		{
			// Allow Value to call make<T>() and the default constructor
			friend Value;

		public:
			ValueDeallocator(ValueDeallocator &&that) noexcept;
			ValueDeallocator &operator=(ValueDeallocator &&that) noexcept;
			~ValueDeallocator();

			/**
			 * Deallocate a Value-owned pointer.
			 * @param[in] ptr The pointer to deallocate.
			 */
			void deallocate(void *ptr) const;

		private:
			value_deallocator_base *m_deallocator;

			ValueDeallocator();
			explicit ValueDeallocator(value_deallocator_base *deallocator);

			template <typename T>
			static ValueDeallocator make()
			{
				return ValueDeallocator(new value_deallocator<T>());
			}
		};
	}

	/**
	* TODO: Documentation
	*/
	class ValueCaster
	{
		// Allow Value to call the default constructor and get<SrcT>()
		friend Value;

	public:
		~ValueCaster();

		/**
		* Declare a run-time casting interface from SrcT to DestT.
		* @tparam SrcT The cast source type.
		* @tparam DestT the cast destination type.
		*/
		template <typename SrcT, typename DestT>
		static void declare()
		{
			ValueCaster &caster = ValueCaster::get<SrcT>();
			caster.add_caster<SrcT, DestT>();
		}

		/**
		 * @tparam SrcT The cast source type.
		 * @tparam DestT The cast destination type.
		 * @param[in] value The Value that is being casted to the destination type.
		 * @returns A Value with a reference to a DestT value/instance.
		 */
		template <typename SrcT, typename DestT>
		static Value cast(const Value &value);

		/**
		* @tparam DestT The cast destination type.
		* @param[in] src_type The std::type_info object of the source type.
		* @param[in] value The Value that is being casted to the destination type.
		* @returns A Value with a reference to a DestT value/instance.
		*/
		template <typename DestT>
		static Value cast(const std::type_info &src_type, const Value &value);

		/**
		 * @tparam DestT The cast destination type.
		 * @param[in] value The Value that is being casted to the destination type.
		 * @returns A Value with a reference to a DestT value/instance.
		 */
		template <typename DestT>
		Value cast_value(const Value &value) const;

	private:
		/**
		 * A static lookup used to find appropriate casters at runtime.
		 * Contains SrcT -> Caster elements.
		 */
		static std::unordered_map<std::size_t, ValueCaster *> *s_caster_lookup;

		const std::type_info *m_src_type;
		std::unordered_map<std::size_t, detail::value_caster_base *> m_casts;

		ValueCaster();
		explicit ValueCaster(const std::type_info &src_type);

		/**
		 * @tparam SrcT The cast source type.
		 * @returns The ValueCaster instance responsible for casting values of type SrcT.
		 */
		template <typename SrcT>
		static ValueCaster &get()
		{
			if (!s_caster_lookup)
				s_caster_lookup = new std::unordered_map<std::size_t, ValueCaster *>();

			const auto &src_type = typeid(SrcT);
			const auto src_type_hash = src_type.hash_code();
			if (s_caster_lookup->find(src_type_hash) == s_caster_lookup->end())
				(*s_caster_lookup)[src_type_hash] = new ValueCaster(src_type);
			return *s_caster_lookup->at(src_type_hash);
		}

		template <typename SrcT, typename DestT>
		void add_caster()
		{
			const auto dest_type_hash = typeid(DestT).hash_code();
			m_casts[dest_type_hash] = new detail::value_caster<SrcT, DestT>();
		}
	};

	/**
	 * A wrapper around a void pointer that ensures type safety.
	 */
	class Value
	{
	public:
		Value(Value &&that) noexcept;
		Value &operator=(Value &&that) noexcept;
		~Value();

		/**
		 * @returns Whether or the not the value is holding a reference or a value.
		 */
		bool is_reference() const
		{
			// If the pointer isn't owned, then it isn't this Value's responsibility
			// to clean it up, so we say it's referencing something.
			return !m_ptr_is_owned;
		}

		/**
		 * @returns Whether or not the value being held is of type T.
		 */
		template <typename T>
		bool is() const
		{
			// Do the type hashes match?
			return m_type_hash == typeid(T).hash_code();
		}
		
		/**
		 * @tparam T 
		 * @returns A new Value instance that owns it's value.
		 */
		template <typename T>
		Value dereference() const
		{
			// Do we need to attempt casting?
			if (!is<T>())
				return m_caster->cast_value<T>(*this);
			return Value::make_value<T>(*static_cast<T *>(m_value_ptr));
		}

		/**
		 * @tparam T The expected type.
		 * @returns A reference to the value being held.
		 * @throws ki::runtime_error The expected type and the type of the value being held are not the same.
		 */
		template <typename T>
		const T &get() const
		{
			// Make sure they requested the correct type
			if (!is<T>())
				throw runtime_error("Invalid call to Value::get<T>.");

			// Return a reference to the value being held
			return *static_cast<T *>(m_value_ptr);
		}

		/**
		 * @tparam T The expected type.
		 * @returns A reference to the value being held.
		 * @throws ki::runtime_error If the expected type and the type of the value being held are not the same.
		 */
		template <typename T>
		T &get()
		{
			// Make sure they requested the correct type
			if (!is<T>())
				throw runtime_error("Invalid call to Value::get<T>.");

			// Return a reference to the value being held
			return *static_cast<T *>(m_value_ptr);
		}

		/**
		 * @tparam T The expected type.
		 * @returns A pointer to the value being held (that the caller takes ownership of).
	 	 * @throws ki::runtime_error If the Value is a reference.
		 * @throws ki::runtime_error If the expected type and the type of the value being held are not the same.
		 */
		template <typename T>
		T *take()
		{
			// Make sure this Value is not a reference
			if (is_reference())
				throw runtime_error("Cannot take ownership from a reference Value.");

			// Make sure they requested the correct type
			if (!is<T>())
				throw runtime_error("Invalid call to Value::get<T>.");

			// Give up the pointer (this Value becomes a reference)
			m_ptr_is_owned = false;
			return static_cast<T *>(m_value_ptr);
		}

		/**
		 * @tparam T The type of value to hold.
		 * @param[in] value The initial value.
		 * @returns A new Value instance that owns it's value.
		 */
		template <typename T>
		static Value make_value(T value)
		{
			auto *ptr = static_cast<void *>(new T(value));
			auto val = Value(ptr, true);
			val.construct<T>();
			return val;
		}

		/**
		 * @tparam T The type of value to hold.
		 * @param[in] value The initial value.
		 * @returns A new Value instance that refers to a value it doesn't own.
		 */
		template <typename T>
		static Value make_reference(T &value)
		{
			auto *ptr = static_cast<void *>(&value);
			auto val = Value(ptr);
			val.construct<T>();
			return val;
		}

		/**
		 * @tparam T The type of value to hold.
		 * @param[in] value The initial value.
		 * @returns A new Value instance that refers to a value it doesn't own.
		 */
		template <typename T>
		static Value make_reference(const T &value)
		{
			auto *ptr = const_cast<void *>(static_cast<const void *>(&value));
			auto val = Value(ptr);
			val.construct<T>();
			return val;
		}

	private:
		void *m_value_ptr;
		bool m_ptr_is_owned;
		std::size_t m_type_hash;
		ValueCaster *m_caster;
		detail::ValueDeallocator m_deallocator;

		explicit Value(void *value_ptr, bool owned = false);

		/**
		 * Set values that depend on knowing T.
		 * @tparam T The type that is being held.
		 */
		template <typename T>
		void construct()
		{
			m_type_hash = typeid(T).hash_code();
			m_caster = &ValueCaster::get<T>();
			m_deallocator = detail::ValueDeallocator::make<T>();
		}
	};

	template <typename DestT>
	Value ValueCaster::cast(const std::type_info &src_type, const Value& value)
	{
		const auto it = s_caster_lookup->find(src_type.hash_code());
		if (it != s_caster_lookup->end())
			return it->second->cast_value<DestT>(value);
		throw cast_error(src_type, typeid(DestT));
	}

	template <typename SrcT, typename DestT>
	Value ValueCaster::cast(const Value& value)
	{
		ValueCaster::declare<SrcT, DestT>();
		ValueCaster &caster = ValueCaster::get<SrcT>();
		return caster.cast_value<DestT>(value);
	}

	template <typename DestT>
	Value ValueCaster::cast_value(const Value& value) const
	{
		const auto it = m_casts.find(typeid(DestT).hash_code());
		if (it != m_casts.end())
			return it->second->cast(value);
		throw cast_error(*m_src_type, typeid(DestT));
	}

	namespace detail
	{
		template <typename SrcT, typename DestT>
		Value value_caster_impl<SrcT, DestT>::bad_cast()
		{
			throw cast_error(typeid(SrcT), typeid(DestT));
		}

		/**
		 * TODO: Documentation
		 */
		template <
			typename SrcT, typename DestT,
			typename SrcEnable, typename DestEnable
		>
		struct value_caster : value_caster_impl<SrcT, DestT>
		{
			Value cast(const Value &value) const override
			{
				// By default, just attempt to static_cast from SrcT to DestT.
				return Value::make_value<DestT>(
					static_cast<DestT>(value.get<SrcT>())
				);
			}
		};
	}
}
}
