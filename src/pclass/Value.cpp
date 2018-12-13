#include "ki/pclass/Value.h"

namespace ki
{
namespace pclass
{
	namespace detail
	{
		Value value_caster_base::cast(const Value &v) const
		{
			throw runtime_error("Unimplemented cast.");
		}

		ValueDeallocator::ValueDeallocator()
		{
			m_deallocator = nullptr;
		}

		ValueDeallocator::ValueDeallocator(value_deallocator_base *deallocator)
		{
			m_deallocator = deallocator;
		}

		ValueDeallocator::ValueDeallocator(ValueDeallocator &&that) noexcept
		{
			m_deallocator = that.m_deallocator;
			that.m_deallocator = nullptr;
		}

		ValueDeallocator& ValueDeallocator::operator=(ValueDeallocator &&that) noexcept
		{
			m_deallocator = that.m_deallocator;
			that.m_deallocator = nullptr;
			return *this;
		}

		ValueDeallocator::~ValueDeallocator()
		{
			delete m_deallocator;
		}

		void ValueDeallocator::deallocate(void *ptr) const
		{
			m_deallocator->deallocate(ptr);
		}
	}

	ValueCaster::ValueCaster()
	{
		m_src_type = nullptr;
	}

	ValueCaster::~ValueCaster()
	{
		for (auto it = m_casts.begin();
			it != m_casts.end(); ++it)
		{
			delete it->second;
		}
	}

	ValueCaster::ValueCaster(const std::type_info &src_type)
	{
		m_src_type = &src_type;
	}

	Value::Value(void *value_ptr, const bool owned)
	{
		m_value_ptr = value_ptr;
		m_ptr_is_owned = owned;
		m_type_hash = 0;
		m_caster = nullptr;
		m_deallocator = detail::ValueDeallocator();
	}

	Value::Value(Value &&that) noexcept
	{
		// Move pointer to this Value object, and take ownership if
		// the other Value previously owned it.
		m_value_ptr = that.m_value_ptr;
		m_ptr_is_owned = that.m_ptr_is_owned;
		that.m_ptr_is_owned = false;
		m_type_hash = that.m_type_hash;
		m_caster = that.m_caster;
		m_deallocator = std::move(that.m_deallocator);
	}

	Value &Value::operator=(Value &&that) noexcept
	{
		// If the current pointer is owned, deallocate it
		if (m_ptr_is_owned)
			m_deallocator.deallocate(m_value_ptr);

		// Move pointer to this Value object, and take ownership if
		// the other Value previously owned it.
		m_value_ptr = that.m_value_ptr;
		m_ptr_is_owned = that.m_ptr_is_owned;
		that.m_ptr_is_owned = false;
		m_type_hash = that.m_type_hash;
		m_caster = that.m_caster;
		m_deallocator = std::move(that.m_deallocator);

		return *this;
	}

	Value::~Value()
	{
		if (m_value_ptr && m_ptr_is_owned)
			m_deallocator.deallocate(m_value_ptr);
		m_value_ptr = nullptr;
	}

	// Initialize the static lookup map
	std::unordered_map<std::size_t, ValueCaster *> ValueCaster::s_caster_lookup = {};
}
}
