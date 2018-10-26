#pragma once
#include <cstdint>
#include <type_traits>

namespace ki
{
	/**
	 * A helper utility that provides the most suitable primitive type
	 * to store an N bit integer with the appropriate signedness.
	 */
	template <uint8_t N>
	struct bits
	{
		// Do not allow 0 bits.
		static_assert(N > 0, "N must be greater than 0.");

		// Do not allow a bit length > 64 as there is no
		// type to adequately represent these values.
		static_assert(N <= 64, "N must be less than or equal to 64.");

		/**
		 * The integer type that can most efficiently store N bits
		 * with signedness.
		 */
		using int_type = typename std::conditional<
			N <= 8,
			int8_t,
			typename std::conditional<
				N <= 16,
				int16_t,
				typename std::conditional<
					N <= 32,
					int32_t,
					int64_t
				>::type
			>::type
		>::type;

		/**
		* The integer type that can most efficiently store N bits
		* without signedness.
		*/
		using uint_type = typename std::conditional<
			N <= 8,
			uint8_t,
			typename std::conditional<
				N <= 16,
				uint16_t,
				typename std::conditional<
					N <= 32,
					uint32_t,
					uint64_t
				>::type
			>::type
		>::type;
	};

	/**
	 * Represents an integer of N bits.
	 */
	template <uint8_t N, bool Unsigned>
	struct BitInteger
	{
		/**
		 * The type used to internally store the N-bit integer.
		 */
		using type = typename std::conditional<
			Unsigned,
			typename bits<N>::uint_type,
			typename bits<N>::int_type
		>::type;

		BitInteger()
		{
			m_value = 0;
		}

		BitInteger(const BitInteger<N, Unsigned> &cp)
		{
			m_value = cp.m_value;
		}

		BitInteger(const type value)
		{
			m_value = value;
		}

		BitInteger<N, Unsigned> &operator =(const type rhs)
		{
			m_value = rhs;
			return *this;
		}

		BitInteger<N, Unsigned> &operator +=(const type rhs)
		{
			m_value += rhs;
			return *this;
		}

		BitInteger<N, Unsigned> &operator -=(const type rhs)
		{
			m_value -= rhs;
			return *this;
		}

		BitInteger<N, Unsigned> &operator *=(const type rhs)
		{
			m_value *= rhs;
			return *this;
		}

		BitInteger<N, Unsigned> &operator /=(const type rhs)
		{
			m_value /= rhs;
			return *this;
		}

		BitInteger<N, Unsigned> &operator |=(const type rhs)
		{
			m_value |= rhs;
			return *this;
		}

		BitInteger<N, Unsigned> &operator &=(const type rhs)
		{
			m_value &= rhs;
			return *this;
		}

		BitInteger<N, Unsigned> &operator ++()
		{
			m_value += 1;
			return *this;
		}

		BitInteger<N, Unsigned> &operator --()
		{
			m_value -= 1;
			return *this;
		}

		BitInteger<N, Unsigned> operator ++(int increment)
		{
			auto copy(*this);
			++(*this);
			return copy;
		}

		BitInteger<N, Unsigned> operator --(int increment)
		{
			auto copy(*this);
			--(*this);
			return copy;
		}

		operator type() const
		{
			return m_value;
		}
	private:
		type m_value : N;
	};

	/**
	* Represents a signed integer of N bits.
	*/
	template <uint8_t N>
	using bi = BitInteger<N, false>;

	/**
	* Represents an unsigned integer of N bits.
	*/
	template <uint8_t N>
	using bui = BitInteger<N, true>;

	/**
	* A utility to calculate the bitsize of a type.
	*/
	template <typename T, typename Enable = void>
	struct bitsizeof
	{
		/**
		* The number of bits a type will occupy if written to a BitStream.
		* This does not reflect the size of the type in memory.
		*/
		static constexpr std::size_t value = sizeof(T) * 8;
	};

	template <uint8_t N, bool Unsigned>
	struct bitsizeof<BitInteger<N, Unsigned>>
	{
		static constexpr std::size_t value = N;
	};

	/**
	 * Determines whether a given type is integral.
	 */
	template <typename T, typename Enable = void>
	struct is_integral : std::false_type {};

	template <typename T>
	struct is_integral<
		T,
		typename std::enable_if<std::is_integral<T>::value>::type
	> : std::true_type {};

	template <uint8_t N, bool Unsigned>
	struct is_integral<BitInteger<N, Unsigned>> : std::true_type {};
}