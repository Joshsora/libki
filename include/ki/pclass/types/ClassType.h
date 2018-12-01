#pragma once
#include <type_traits>
#include <string>
#include "ki/pclass/Property.h"
#include "ki/pclass/types/Type.h"
#include "ki/pclass/PropertyClass.h"

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

		void write_to(BitStream &stream, const Value &value) const override;
		void read_from(BitStream &stream, Value &value) const override;

		bool inherits(const Type &type) const;

	protected:
		virtual const PropertyClass &get_object_from_value(const Value &value) const = 0;
		virtual PropertyClass &get_object_from_value(Value &value) const = 0;

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
			: IClassType(name, base_class, type_system) {}

		PropertyClass *instantiate() const override
		{
			return new ClassT(*this, get_type_system());
		}

		const PropertyClass& get_object_from_value(const Value& value) const override
		{
			return dynamic_cast<const PropertyClass &>(value.get<ClassT>());
		}

		PropertyClass& get_object_from_value(Value& value) const override
		{
			return dynamic_cast<PropertyClass &>(value.get<ClassT>());
		}
	};
}
}
