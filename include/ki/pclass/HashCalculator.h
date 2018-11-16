#pragma once
#include <string>

namespace ki
{
	namespace pclass
	{
		/**
		 * The type used to store a type/property hash.
		 */
		typedef uint32_t hash_t;

		/**
		 * A base class for type/property hash calculators.
		 */
		class HashCalculator
		{
		public:
			virtual ~HashCalculator() {};

			/**
			 * Calculate a type hash from the type's name.
			 * @param name The name of the type.
			 */
			virtual hash_t calculate_type_hash(const std::string &name) const = 0;

			/**
			* Calculate a property hash from the property's name.
			* @param name The name of the property.
			*/
			virtual hash_t calculate_property_hash(const std::string& name) const = 0;
		};

		/**
		 * A hash calculator that uses the algorithms found and used in
		 * Wizard101.
		 */
		class WizardHashCalculator : public HashCalculator
		{
		public:
			hash_t calculate_type_hash(const std::string& name) const override;
			hash_t calculate_property_hash(const std::string& name) const override;
		};
	}
}
