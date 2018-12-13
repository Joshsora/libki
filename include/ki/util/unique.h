#pragma once
#include <utility>
#include <memory>

#if __cplusplus <= 201103L
namespace ki
{
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(
			new T(std::forward<Args>(args)...)
		);
	}
}
#endif
