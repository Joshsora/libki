#pragma once
#include <string>
#include <vector>
#include <map>
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
		friend class TypeSystem;

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
		virtual ~Type() { }

		std::string get_name() const;
		hash_t get_hash() const;
		kind get_kind() const;
		const TypeSystem &get_type_system() const;

		virtual PropertyClass *instantiate() const;
		virtual void write_to(BitStream &stream, const Value &value) const;
		virtual void read_from(BitStream &stream, Value &value) const;

	protected:
		kind m_kind;

	private:
		std::string m_name;
		hash_t m_hash;
		const TypeSystem &m_type_system;

		/**
		 * Called by a TypeSystem instance when it's HashCalculator
		 * is changed.
		 */
		virtual void update_hash();
	};

	typedef std::vector<Type *> TypeList;
	typedef std::map<std::string, Type *> TypeNameMap;
	typedef std::map<hash_t, Type *> TypeHashMap;

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
