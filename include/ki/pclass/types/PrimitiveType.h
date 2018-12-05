#pragma once
#include "ki/pclass/types/Type.h"

namespace ki
{
namespace pclass
{
	/**
	 * TODO: Documentation
	 */
	template <typename ValueT, typename Enable = void>
	struct PrimitiveTypeWriter
	{
		static void write_to(BitStream &stream, const ValueT &value)
		{
			// Provide a compiler error if this is not specialized
			static_assert(
				sizeof(ValueT) == 0,
				"Missing specialization of PrimitiveTypeWriter::write_to"
			);
		}
	};

	/**
	 * TODO: Documentation
	 */
	template <typename ValueT, typename Enable = void>
	struct PrimitiveTypeReader
	{
		static void read_from(BitStream &stream, ValueT &value)
		{
			// Provide a compiler error if this is not specialized
			static_assert(
				sizeof(ValueT) == 0,
				"Missing specialization of PrimitiveTypeReader::read_from"
			);
		}
	};
	
	/**
	 * TODO: Documentation
	 */
	template <typename ValueT>
	class PrimitiveType : public Type
	{
	public:
		PrimitiveType(const std::string name, const TypeSystem &type_system)
			: Type(name, type_system)
		{
			m_kind = kind::PRIMITIVE;
		}

		void write_to(BitStream &stream, const Value &value) const override
		{
			try
			{
				PrimitiveTypeWriter<ValueT>::write_to(stream, value.get<ValueT>());
			}
			catch (runtime_error &e)
			{
				std::ostringstream oss;
				oss << "Invalid call to Type::write_to -- " << e.what();
				throw runtime_error(oss.str());
			}
		}

		void read_from(BitStream &stream, Value &value) const override
		{
			try
			{
				PrimitiveTypeReader<ValueT>::read_from(stream, value.get<ValueT>());
			}
			catch (runtime_error &e)
			{
				std::ostringstream oss;
				oss << "Invalid call to Type::read_from -- " << e.what();
				throw runtime_error(oss.str());
			}
		}
	};
}
}

// Include all template specializations
#include "ki/pclass/types/IntegralPrimitiveType.h"
#include "ki/pclass/types/FloatingPointPrimitiveType.h"
#include "ki/pclass/types/StringPrimitiveType.h"
