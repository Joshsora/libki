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
		enum class kind
		{
			NONE,
			
			/**
			 * A Type that contain pure, simple values.
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
		virtual ~Type() {}

		std::string get_name() const;
		hash_t get_hash() const;
		kind get_kind() const;
		const TypeSystem &get_type_system() const;

		virtual std::unique_ptr<PropertyClass> instantiate() const;
		virtual void write_to(BitStream &stream, Value value) const;
		virtual Value read_from(BitStream &stream) const;

	protected:
		kind m_kind;

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
