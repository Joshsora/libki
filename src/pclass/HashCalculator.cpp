#include "ki/pclass/HashCalculator.h"
#include <string>

namespace ki
{
namespace pclass
{
	hash_t WizardHashCalculator::calculate_type_hash(const std::string& name) const
	{
		// Declare variables
		auto result = 0;
		auto a = 0;
		auto b = 32;

		// Iterate through the characters in the string
		for (auto it = name.begin(); it != name.end(); ++it)
		{
			result ^= (*it - 32) << a;
			if (a > 24)
			{
				result ^= (*it - 32) >> b;
				if (a >= 27)
				{
					a -= 32;
					b += 32;
				}
			}

			a += 5;
			b -= 5;
		}

		// Make the result an absolute value
		if (result < 0)
			result = -result;
		return result;
	}

	hash_t WizardHashCalculator::calculate_property_hash(const std::string& name) const
	{
		// Find the hash of the property name
		hash_t result = 0x1505;
		for (auto it = name.begin(); it != name.end(); ++it)
			result = (0x21 * result) + *it;
		return result & 0x7FFFFFFF;
	}
}
}
