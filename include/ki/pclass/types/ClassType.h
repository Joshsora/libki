#pragma once
#include <type_traits>
#include <string>
#include "ki/pclass/Property.h"
#include "ki/pclass/types/Type.h"
#include "ki/pclass/PropertyClass.h"
#include "ki/util/exception.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class ClassTypeBase : public Type
	{
	public:
		ClassTypeBase(const std::string &name,
			const Type *base_class, const TypeSystem &type_system);
		virtual ~ClassTypeBase() {}

		bool inherits(const Type &type) const;

	private:
		const ClassTypeBase *m_base_class;
	};

	/**
	* TODO: Documentation
	*/
	template <class ClassT>
	class ClassType : public ClassTypeBase
	{
		// Ensure that ClassT inherits PropertyClass
		static_assert(std::is_base_of<PropertyClass, ClassT>::value, "ClassT must inherit PropertyClass!");

	public:
		ClassType(const std::string &name,
			const Type *base_class, const TypeSystem &type_system)
			: ClassTypeBase(name, base_class, type_system) {}

		PropertyClass *instantiate() const override
		{
			return new ClassT(*this, get_type_system());
		}

		void write_to(BitStream &stream, const Value &value) const override
		{
			const auto &object = dynamic_cast<const PropertyClass &>(value.get<ClassT>());
			const auto &properties = object.get_properties();
			for (auto it = properties.begin(); it != properties.end(); ++it)
				(*it)->write_value_to(stream);
		}

		void read_from(BitStream &stream, Value &value) const override
		{
			auto &object = dynamic_cast<PropertyClass &>(value.get<ClassT>());
			auto &properties = object.get_properties();
			for (auto it = properties.begin(); it != properties.end(); ++it)
				(*it)->read_value_from(stream);
		}
	};
}
}
