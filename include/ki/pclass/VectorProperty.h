#pragma once
#include <vector>
#include "ki/pclass/Property.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	template <typename ValueT>
	class VectorPropertyBase : public std::vector<ValueT>, public DynamicPropertyBase
	{
	public:
		VectorPropertyBase(PropertyClass &object,
			const std::string &name, const Type &type)
			: DynamicPropertyBase(object, name, type) { }

		std::size_t get_element_count() const override
		{
			return this->size();
		}

		Value get_value(int index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return this->at(index);
		}
	};

	template <typename ValueT, typename Enable = void>
	class VectorPropertyBase2 : public VectorPropertyBase<ValueT>
	{
		VectorPropertyBase2(PropertyClass &object,
			const std::string &name, const Type &type)
			: VectorPropertyBase<ValueT>(object, name, type) { }

		constexpr bool is_pointer() const override
		{
			return false;
		}

		void write_value_to(BitStream &stream, const int index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			this->get_type().write_to(stream, this->at(index));
		}

		void read_value_from(BitStream &stream, const int index) override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			this->get_type().read_from(stream, this->at(index));
		}
	};

	template <typename ValueT>
	class VectorPropertyBase2<
		ValueT,
		typename std::enable_if<
			std::is_pointer<ValueT>::value
		>::type
	> : public VectorPropertyBase<ValueT>
	{
	public:
		VectorPropertyBase2(PropertyClass &object,
			const std::string &name, const Type &type)
			: VectorPropertyBase<ValueT>(object, name, type) { }

		constexpr bool is_pointer() const override
		{
			return true;
		}

		void write_value_to(BitStream &stream, const int index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			this->get_type().write_to(stream, *this->at(index));
		}

		void read_value_from(BitStream &stream, const int index) override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			this->get_type().read_from(stream, Value(*this->at(index)));
		}
	};

	template <
		typename ValueT,
		typename _IsBaseEnable = void,
		typename _IsPointerEnable = void
	>
	class VectorProperty : public VectorPropertyBase2<ValueT>
	{
	public:
		VectorProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: VectorPropertyBase2<ValueT>(object, name, type) { }

		const PropertyClass *get_object(const int index) const override
		{
			// We aren't holding an object at all, whoever called this is mistaken.
			throw runtime_error(
				"Tried calling get_object(index) on a property that does not store an object."
			);
		}
	};

	template <typename ValueT>
	class VectorProperty<
		ValueT,
		typename std::enable_if<
			std::is_base_of<
				PropertyClass,
				typename std::remove_pointer<ValueT>::type
			>::value
		>::type,
		typename std::enable_if<
			!std::is_pointer<ValueT>::value
		>::type
	> : public VectorPropertyBase2<ValueT>
	{
	public:
		VectorProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: VectorPropertyBase2<ValueT>(object, name, type) { }

		const PropertyClass *get_object(const int index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return dynamic_cast<PropertyClass *>(&this->at(index));
		}
	};

	template <typename ValueT>
	class VectorProperty<
		ValueT,
		typename std::enable_if<
			std::is_base_of<
				PropertyClass,
				typename std::remove_pointer<ValueT>::type
			>::value
		>::type,
		typename std::enable_if<
			std::is_pointer<ValueT>::value
		>::type
	> : public VectorPropertyBase2<ValueT>
	{
	public:
		VectorProperty(PropertyClass &object,
			const std::string &name, const Type &type)
			: VectorPropertyBase2<ValueT>(object, name, type) { }

		const PropertyClass *get_object(const int index) const override
		{
			if (index < 0 || index >= this->size())
				throw runtime_error("Index out of bounds.");
			return dynamic_cast<PropertyClass *>(this->at(index));
		}
	};
}
}
