#pragma once
#include <memory>
#include <type_traits>
#include <string>
#include "ki/pclass/Property.h"
#include "ki/pclass/Type.h"
#include "ki/pclass/PropertyClass.h"

namespace ki
{
namespace pclass
{
	/**
	 * Base type for classes. Adds inheritance to Type.
	 */
	class IClassType : public Type
	{
	public:
		IClassType(const std::string &name,
			const Type *base_class, const TypeSystem &type_system);
		virtual ~IClassType() = default;

		/**
		 * @param[in] type The ancestor type to check against.
		 * @returns True if this Type is a descendant of the given type. False otherwise.
		 */
		bool inherits(const Type &type) const;

		void write_to(BitStream &stream, const bool is_file, Value &value) const override = 0;
		Value read_from(BitStream &stream, const bool is_file) const override = 0;

	private:
		const IClassType *m_base_class;
	};

	/**
	 * A user-defined structure.
	 * @tparam ClassT The compile-time user-defined class that the class represents.
	 */
	template <typename ClassT>
	class ClassType : public IClassType
	{
		// Ensure that ClassT inherits PropertyClass
		static_assert(std::is_base_of<PropertyClass, ClassT>::value, "ClassT must inherit PropertyClass!");

	public:
		// Do not allow copy construction or movement of types
		ClassType(const ClassType<ClassT> &that) = delete;
		ClassType &operator=(const ClassType<ClassT> &that) = delete;
		ClassType(ClassType<ClassT> &&that) noexcept = delete;
		ClassType &operator=(ClassType<ClassT> &&that) noexcept = delete;

		ClassType(const std::string &name,
			const Type *base_class, const TypeSystem &type_system)
			: IClassType(name, base_class, type_system)
		{}
		~ClassType() = default;

		std::unique_ptr<PropertyClass> instantiate() const override
		{
			auto *instance = new ClassT(*this, get_type_system());
			return std::unique_ptr<PropertyClass>(
				dynamic_cast<PropertyClass *>(instance)
			);
		}

		void write_to(BitStream &stream, const bool is_file, Value &value) const override
		{
			const auto &object = value.get<ClassT>();
			const auto &properties = object.get_properties();
			for (auto it = properties.begin(); it != properties.end(); ++it)
				it->write_value_to(stream, is_file);
		}

		Value read_from(BitStream &stream, const bool is_file) const override
		{
			auto object = ClassT(*this, get_type_system());
			auto &properties = object.get_properties();
			for (auto it = properties.begin(); it != properties.end(); ++it)
				it->read_value_from(stream, is_file);
			return Value::make_value<ClassT>(object);
		}
	};
}
}
