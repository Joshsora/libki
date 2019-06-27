#pragma once
#include <type_traits>

#define MAKE_FLAGS_ENUM(n) \
	namespace ki { \
		template <> \
		struct is_flags_enum<n> : std::true_type {}; }

#define SET_FLAG(v, f) v |= f
#define UNSET_FLAG(v, f) v &= ~f
#define FLAG_IS_SET(v, f) ((v & f) == f)

namespace ki
{
	template <typename EnumT>
	struct is_flags_enum : std::false_type {};

	template <
		typename EnumT,
		typename = typename std::enable_if<is_flags_enum<EnumT>::value>::type
	>
	constexpr EnumT operator|(EnumT lhs, EnumT rhs)
	{
		using type = typename std::underlying_type<EnumT>::type;
		return static_cast<EnumT>(
			static_cast<type>(lhs) | static_cast<type>(rhs)
		);
	}

	template <
		typename EnumT,
		typename = typename std::enable_if<is_flags_enum<EnumT>::value>::type
	>
	constexpr EnumT operator&(EnumT lhs, EnumT rhs)
	{
		using type = typename std::underlying_type<EnumT>::type;
		return static_cast<EnumT>(
			static_cast<type>(lhs) & static_cast<type>(rhs)
		);
	}

	template <
		typename EnumT,
		typename = typename std::enable_if<is_flags_enum<EnumT>::value>::type
	>
	constexpr EnumT operator~(EnumT lhs)
	{
		using type = typename std::underlying_type<EnumT>::type;
		return static_cast<EnumT>(~static_cast<type>(lhs));
	}

	template <
		typename EnumT,
		typename = typename std::enable_if<is_flags_enum<EnumT>::value>::type
	>
	EnumT &operator|=(EnumT &lhs, EnumT rhs)
	{
		return lhs = lhs | rhs;
	}

	template <
		typename EnumT,
		typename = typename std::enable_if<is_flags_enum<EnumT>::value>::type
	>
	EnumT &operator&=(EnumT &lhs, EnumT rhs)
	{
		return lhs = lhs & rhs;
	}
}
