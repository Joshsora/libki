#pragma once
#include "ki/pclass/types/Type.h"
#include "ki/pclass/PropertyList.h"

#define _KI_TYPE ki::pclass::Type
#define _KI_TYPE_SYSTEM ki::pclass::TypeSystem
#define _KI_PCLASS ki::pclass::PropertyClass

#define _KI_PCLASS_CONSTRUCTOR(derived) \
	explicit derived(const _KI_TYPE &type, const _KI_TYPE_SYSTEM &type_system)

#define _KI_PCLASS_COPY_CONSTRUCTOR(derived) \
	derived(const derived &that)

#define _KI_PCLASS_CONSTRUCT_BASE(base) \
	: base(type, type_system)

#define _KI_PCLASS_COPY_CONSTRUCT_BASE(base) \
	: base(that)

#define DERIVED_PCLASS(derived, base) class derived : public base
#define PCLASS(derived) DERIVED_PCLASS(derived, _KI_PCLASS)

#define PCLASS_CONSTRUCTOR(derived) \
_KI_PCLASS_CONSTRUCTOR(derived) \
	_KI_PCLASS_CONSTRUCT_BASE(_KI_PCLASS)

#define DERIVED_PCLASS_CONSTRUCTOR(derived, base) \
_KI_PCLASS_CONSTRUCTOR(derived) \
	_KI_PCLASS_CONSTRUCT_BASE(base)

#define PCLASS_COPY_CONSTRUCTOR(derived) \
_KI_PCLASS_COPY_CONSTRUCTOR(derived) \
	_KI_PCLASS_COPY_CONSTRUCT_BASE(_KI_PCLASS)

#define DERIVED_PCLASS_COPY_CONSTRUCTOR(derived, base) \
_KI_PCLASS_COPY_CONSTRUCTOR(derived) \
	_KI_PCLASS_COPY_CONSTRUCT_BASE(base)

#define TYPE(n) type_system.get_type(n)

#define INIT_PROPERTY(identifier, type) \
	, identifier(*this, #identifier, TYPE(type))

#define INIT_PROPERTY_COPY(identifier) \
	, identifier(*this, that.identifier)

#define INIT_PROPERTY_VALUE(identifier, type, value) \
	, identifier(*this, #identifier, TYPE(type), value)

namespace ki
{
namespace pclass
{
	template <typename ValueT>
	class ClassType;

	/**
	 * TODO: Documentation
	 */
	class PropertyClass
	{
		friend IProperty;

	public:
		explicit PropertyClass(const Type &type, const TypeSystem &type_system);
		virtual ~PropertyClass() {}

		PropertyClass(const PropertyClass &that);
		PropertyClass &operator=(const PropertyClass &that);

		const Type &get_type() const;
		PropertyList &get_properties();
		const PropertyList &get_properties() const;

		virtual PropertyClass *copy() const;
		virtual void on_created() const {}

	protected:
		void add_property(IProperty &prop);

	private:
		const Type *m_type;
		PropertyList m_properties;
	};
}
}
