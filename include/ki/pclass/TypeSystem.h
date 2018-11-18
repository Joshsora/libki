#pragma once
#include <vector>
#include <map>
#include "ki/pclass/HashCalculator.h"
#include "ki/pclass/types/Type.h"
#include "ki/pclass/types/PrimitiveType.h"
#include "ki/pclass/types/ClassType.h"
#include "ki/pclass/types/EnumType.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class TypeSystem
	{
	public:
		explicit TypeSystem(HashCalculator *hash_calculator);
		~TypeSystem();

		const HashCalculator &get_hash_calculator() const;
		void set_hash_calculator(HashCalculator *hash_calculator);

		bool has_type(const std::string &name) const;
		bool has_type(hash_t hash) const;

		const Type &get_type(const std::string &name) const;
		const Type &get_type(hash_t hash) const;

		template <typename ValueT>
		PrimitiveType<ValueT> &define_primitive(const std::string &name)
		{
			auto *type = new PrimitiveType<ValueT>(name, *this);
			define_type(type);
			return *type;
		}

		template <class ClassT>
		ClassType<ClassT> &define_class(const std::string &name)
		{
			return define_class<ClassT>(name, nullptr);
		}

		template <class ClassT>
		ClassType<ClassT> &define_class(
			const std::string &name, const Type &base_class)
		{
			return define_class<ClassT>(name, &base_class);
		}

		template <typename EnumT>
		EnumType<EnumT> *define_enum(const std::string &name)
		{
			/*
			auto *type = new EnumType<EnumT>(name, this);
			define_type(type);
			return type;
			*/
		}

		template <typename ClassT>
		ClassT *instantiate(const std::string &name) const
		{
			const auto &type = get_type(name);
			return dynamic_cast<ClassT *>(type.instantiate());
		}

	protected:
		void define_type(Type *type);

		template <class ClassT>
		ClassType<ClassT> &define_class(
			const std::string &name, const Type *base_class)
		{
			// If the caller does not specify a base class, automatically make
			// ki::pclass::PropertyClass the base class (if it has been defined)
			if (base_class == nullptr && has_type("class PropertyClass"))
				base_class = &get_type("class PropertyClass");

			auto *type = new ClassType<ClassT>(name, base_class, *this);
			define_type(type);
			return *type;
		}

	private:
		TypeList m_types;
		TypeNameMap m_type_name_lookup;
		TypeHashMap m_type_hash_lookup;
		HashCalculator *m_hash_calculator;
	};
}
}
