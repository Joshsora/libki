#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ki/pclass/HashCalculator.h"
#include "ki/pclass/Value.h"
#include "ki/util/BitStream.h"

namespace ki
{
namespace pclass
{
	class TypeSystem;
	class PropertyClass;

	/**
	 * A base class for classes that represent a Type.
	 */
	class Type
	{
	public:
		// Do not allow copy construction or movement of types
		Type(const Type &that) = delete;
		Type &operator=(const Type &that) = delete;
		Type(Type &&that) noexcept = delete;
		Type &operator=(Type &&that) noexcept = delete;

		/**
		 * An enum of Type kinds.
		 */
		enum class Kind
		{
			NONE,
			
			/**
			 * A Type that contains pure, simple values.
			 */
			PRIMITIVE,

			/**
			 * A user-defined Type.
			 */
			CLASS,

			/**
			 * A data type consisting of a set of named values.
			 */
			ENUM
		};

		Type(const std::string &name, const TypeSystem &type_system);
		virtual ~Type() = default;

		const std::string &get_name() const;
		hash_t get_hash() const;
		Kind get_kind() const;

		/**
		 * The TypeSystem used to define this Type instance.
		 */
		const TypeSystem &get_type_system() const;

		/**
		 * Create an instance of the type being represented.
		 * @returns A pointer to a new PropertyClass instance.
		 */
		virtual std::unique_ptr<PropertyClass> instantiate() const;

		/**
		 * Write a value of this type to a BitStream.
		 * @param[in] stream The stream to write to.
		 * @param[in] value The value to write to the stream.
		 */
		virtual void write_to(BitStream &stream, Value &value) const;

		/**
		 * Read a value of this type from a BitStream.
		 * @param stream[in] The stream to read from.
		 * @returns The value read from the stream.
		 */
		virtual Value read_from(BitStream &stream) const;

	protected:
		Kind m_kind;

	private:
		std::string m_name;
		hash_t m_hash;
		const TypeSystem &m_type_system;
	};

	/**
	 * @param[in] expected The Type that is expected to match with the actual Type.
	 * @param[in] actual The Type that is being checked for a match with the expected Type.
	 * @param[in] allow_inheritance If false, then the expected and actual types must match exactly.
	 * @throws ki::runtime_error If the expected and actual types do not match.
	 */
	void assert_type_match(
		const Type &expected,
		const Type &actual,
		bool allow_inheritance = false
	);
}
}
