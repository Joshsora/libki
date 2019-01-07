#pragma once
#include <vector>
#include <unordered_map>
#include <json.hpp>
#include "ki/pclass/HashCalculator.h"
#include "ki/pclass/Casters.h"
#include "ki/pclass/Type.h"
#include "ki/pclass/PrimitiveType.h"
#include "ki/pclass/ClassType.h"
#include "ki/pclass/EnumType.h"
#include "ki/util/unique.h"

namespace ki
{
namespace pclass
{
	/**
	 * A class that provides run-time type definition and lookup.
	 */
	class TypeSystem
	{
	public:
		explicit TypeSystem(std::unique_ptr<IHashCalculator> &hash_calculator);
		virtual ~TypeSystem() = default;

		/**
		 * @returns The IHashCalculator instance this TypeSystem uses to calculate
		 *          type and property hashes.
		 */
		const IHashCalculator &get_hash_calculator() const;

		/**
		 * @param[in] name The name of the type to search for.
		 * @returns Whether a type has been defined with the specified name.
		 */
		bool has_type(const std::string &name) const;

		/**
		 * @param[in] hash The hash of the type to search for.
		 * @returns Whether a type exists with the specified hash.
		 */
		bool has_type(hash_t hash) const;

		/**
		 * @param[in] name The name of the type to search for.
		 * @returns The Type instance defined with the specified name.
		 * @throws ki::runtime_error If a Type with the specified name could not be found.
		 */
		const Type &get_type(const std::string &name) const;
		
		/**
		 * @param[in] hash The hash of the type to search for.
		 * @returns The Type instance with the specified type hash.
		 * @throws ki::runtime_error If a Type with the specified hash could not be found.
		 */
		const Type &get_type(hash_t hash) const;

		/**
		 * Define a new primitive type.
		 * @tparam ValueT the compile-time primitive type to represent.
		 * @param[in] name The name of the primitive to define.
		 * @returns A reference to the newly defined PrimitiveType<ValueT>.
		 */
		template <typename ValueT>
		PrimitiveType<ValueT> &define_primitive(const std::string &name)
		{
			detail::caster_declarer<ValueT>::declare();
			auto *type = new PrimitiveType<ValueT>(name, *this);
			define_type(std::unique_ptr<Type>(
				dynamic_cast<Type *>(type)
			));
			return *type;
		}

		/**
		 * Define a new class type.
		 * @tparam ClassT The compile-time class type to represent. Must inherit PropertyClass.
		 * @param[in] name The name of the class to define.
		 * @returns A reference to the newly defined ClassType<ClassT>.
		 */
		template <class ClassT>
		ClassType<ClassT> &define_class(const std::string &name)
		{
			return define_class<ClassT>(name, nullptr);
		}

		/**
		 * Define a new derived class type.
		 * @tparam ClassT The compile-time class type to represent.
		 * @param[in] name The name of the class to define.
		 * @param[in] base_class A reference to the Type instance of the base class. Must be an IClassType.
		 * @returns A reference to the newly defined ClassType<ClassT>.
		 */
		template <typename ClassT>
		ClassType<ClassT> &define_class(
			const std::string &name, const Type &base_class)
		{
			return define_class<ClassT>(name, &base_class);
		}

		/**
		 * Define a new dynamic enum type.
		 * @param[in] name The name of the enum to define.
		 * @returns A reference to the newly defined EnumType.
		 */
		EnumType &define_enum(const std::string &name);

		/**
		 * Define a new static enum type.
		 * @tparam EnumT the compile-time enum type to represent.
		 * @param[in] name The name of the enum to define.
		 * @returns A reference to the newly defined CppEnumType<EnumT>.
		 */
		template <typename EnumT>
		CppEnumType<EnumT> &define_enum(const std::string &name)
		{
			detail::caster_declarer<EnumT>::declare();
			auto *type = new CppEnumType<EnumT>(name, *this);
			define_type(std::unique_ptr<Type>(
				dynamic_cast<Type *>(type)
			));
			return *type;
		}

		/**
		 * Create a new instance of a PropertyClass-derived class.
		 * @param[in] name The name of the class type to instantiate.
		 */
		virtual std::unique_ptr<PropertyClass> instantiate(const std::string &name) const;

		/**
		 * Create a new instance of a PropertyClass-derived class.
		 * @tparam ClassT The expected compile-time class.
		 * @param[in] name The name of the class type to instantiate.
		 */
		template <typename ClassT>
		std::unique_ptr<ClassT> instantiate(const std::string &name) const
		{
			auto object = instantiate(name);
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
		std::unique_ptr<IHashCalculator> m_hash_calculator;

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
