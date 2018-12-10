#pragma once
#include <type_traits>
#include <string>
#include "ki/pclass/types/Type.h"
#include "ki/pclass/PropertyClass.h"
#include "ki/pclass/Property.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class IClassType : public Type
	{
	public:
		IClassType(const std::string &name,
			const Type *base_class, const TypeSystem &type_system);
		virtual ~IClassType() {}

		bool inherits(const Type &type) const;

		void write_to(BitStream &stream, Value value) const override = 0;
		Value read_from(BitStream &stream) const override = 0;

	private:
		const IClassType *m_base_class;
	};

	/**
	 * TODO: Documentation
	 */
	template <class ClassT>
	class ClassType : public IClassType
	{
		// Ensure that ClassT inherits PropertyClass
		static_assert(std::is_base_of<PropertyClass, ClassT>::value, "ClassT must inherit PropertyClass!");

	public:
		ClassType(const std::string &name,
			const Type *base_class, const TypeSystem &type_system)
			: IClassType(name, base_class, type_system)
		{}

		PropertyClass *instantiate() const override
		{
			return new ClassT(*this, get_type_system());
		}

		void write_to(BitStream &stream, Value value) const override
		{
			const auto &object = value.get<ClassT>();
			const auto &properties = object.get_properties();
			for (auto it = properties.begin(); it != properties.end(); ++it)
				it->write_value_to(stream);
		}

		Value read_from(BitStream &stream) const override
		{
			auto object = ClassT(*this, get_type_system());
			auto &properties = object.get_properties();
			for (auto it = properties.begin(); it != properties.end(); ++it)
				it->read_value_from(stream);
			return Value::make_value<ClassT>(object);
		}
	};
}
}
