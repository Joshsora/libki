#pragma once
#include "ki/pclass/types/Type.h"

namespace ki
{
namespace pclass
{
	namespace detail
	{
		/**
		 * TODO: Documentation
		 */
		template <typename ValueT, typename Enable = void>
		struct primitive_type_helper
		{
			/**
			 * TODO: Documentation
			 */
			static void write_to(BitStream &stream, const ValueT &value)
			{
				// Provide a compiler error if this is not specialized
				static_assert(
					sizeof(ValueT) == 0,
					"Missing specialization of primitive_type_writer::write_to"
				);
			}

			/**
			 * TODO: Documentation
			 */
			static Value read_from(BitStream &stream)
			{
				// Provide a compiler error if this is not specialized
				static_assert(
					sizeof(ValueT) == 0,
					"Missing specialization of PrimitiveTypeReader::read_from"
				);

				// This should be impossible to reach.
				throw runtime_error("Missing specialization of PrimitiveTypeReader::read_from");
			}
		};
	}
	
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

		void write_to(BitStream &stream, const Value value) const override
		{
			try
			{
				// Dereference the value to the correct type
				Value deref_value = value.dereference<ValueT>();
				detail::primitive_type_helper<ValueT>::write_to(
					stream,
					deref_value.get<ValueT>()
				);
			}
			catch (runtime_error &e)
			{
				std::ostringstream oss;
				oss << "Invalid call to Type::write_to -- " << e.what();
				throw runtime_error(oss.str());
			}
		}

		Value read_from(BitStream &stream) const override
		{
			try
			{
				return detail::primitive_type_helper<ValueT>::read_from(stream);
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
