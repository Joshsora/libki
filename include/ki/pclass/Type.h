#pragma once
#include <string>
#include <vector>
#include <map>
#include "ki/pclass/HashCalculator.h"
#include "ki/pclass/Value.h"
#include "ki/pclass/PropertyClass.h"

namespace ki
{
	class BitStream;

namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	class Type
	{
		friend class TypeSystem;

	public:
		/**
		 * TODO: Documentation
		 */
		enum class kind
		{
			NONE,
			PRIMITIVE,
			CLASS,
			ENUM
		};

		Type(std::string name, hash_t hash);
		virtual ~Type() {};

		std::string get_name() const;
		hash_t get_hash() const;
		kind get_kind() const;

		virtual PropertyClass *instantiate() const;
		virtual void write_to(BitStream &stream, const Value &value) const = 0;
		virtual void read_from(BitStream &stream, Value &value) const = 0;

	protected:
		kind m_kind;

	private:
		std::string m_name;
		hash_t m_hash;

		void set_hash(hash_t hash);
	};

	typedef std::vector<Type *> TypeList;
	typedef std::map<std::string, Type *> TypeNameMap;
	typedef std::map<hash_t, Type *> TypeHashMap;
}
}
