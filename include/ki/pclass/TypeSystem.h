#pragma once
#include <vector>
#include <unordered_map>
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
		explicit TypeSystem(std::unique_ptr<HashCalculator> &hash_calculator);

		const HashCalculator &get_hash_calculator() const;

		bool has_type(const std::string &name) const;
		bool has_type(hash_t hash) const;

		const Type &get_type(const std::string &name) const;
		const Type &get_type(hash_t hash) const;

		template <typename ValueT>
		PrimitiveType<ValueT> &define_primitive(const std::string &name)
		{
			auto *type = new PrimitiveType<ValueT>(name, *this);
			define_type(std::unique_ptr<Type>(
				dynamic_cast<Type *>(type)
			));
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
		CppEnumType<EnumT> &define_enum(const std::string &name)
		{
			auto *type = new CppEnumType<EnumT>(name, *this);
			define_type(std::unique_ptr<Type>(
				dynamic_cast<Type *>(type)
			));
			return *type;
		}

		template <typename ClassT>
		std::unique_ptr<ClassT> instantiate(const std::string &name) const
		{
			const auto &type = get_type(name);
			auto object = type.instantiate();
			return std::unique_ptr<ClassT>(
				dynamic_cast<ClassT *>(object.release())
			);
		}

	protected:
		void define_type(std::unique_ptr<Type> type);

	private:
		std::vector<std::unique_ptr<Type>> m_types;
		std::unordered_map<std::string, Type *> m_type_name_lookup;
		std::unordered_map<hash_t, Type *> m_type_hash_lookup;
		std::unique_ptr<HashCalculator> m_hash_calculator;

		template <class ClassT>
		ClassType<ClassT> &define_class(
			const std::string &name, const Type *base_class)
		{
			// If the caller does not specify a base class, automatically make
			// ki::pclass::PropertyClass the base class (if it has been defined)
			if (base_class == nullptr && has_type("class PropertyClass"))
				base_class = &get_type("class PropertyClass");

			auto *type = new ClassType<ClassT>(name, base_class, *this);
			define_type(std::unique_ptr<Type>(
				dynamic_cast<Type *>(type)
			));
			return *type;
		}
	};
}
}
