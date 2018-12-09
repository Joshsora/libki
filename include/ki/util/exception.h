#pragma once
#include <stdexcept>
#include <typeinfo>
#include <sstream>

namespace ki
{
	class runtime_error : public std::runtime_error
	{
	public:
		explicit runtime_error(const std::string &message)
			: std::runtime_error(message) {}
	};

	class cast_error : public std::exception
	{
	public:
		cast_error(const std::type_info &src_type, const std::type_info &dest_type)
		{
			std::ostringstream oss;
			oss << "Cannot cast from " << src_type.name()
				<< " to " << dest_type.name() << ".";
			m_what = oss.str();
		}

		char const* what() const override
		{
			return m_what.c_str();
		}

	private:
		std::string m_what;
	};
}
