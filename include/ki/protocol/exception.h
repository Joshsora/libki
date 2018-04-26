#pragma once
#include <stdexcept>

namespace ki
{
namespace protocol
{
	class runtime_error : public std::runtime_error
	{
	public:
		explicit runtime_error(std::string message) : std::runtime_error(message) {}
	};

	class parse_error : public runtime_error
	{
	public:
		enum code
		{
			NONE,
			INVALID_XML_DATA,
			INVALID_HEADER_DATA,
			INSUFFICIENT_MESSAGE_DATA,
			INVALID_MESSAGE_DATA
		};

		explicit parse_error(std::string message, code error = code::NONE)
			: runtime_error(message)
		{
			m_code = error;
		}

		code get_error_code() const { return m_code; }
	private:
		code m_code;
	};

	class value_error : public runtime_error
	{
	public:
		enum code
		{
			NONE,
			MISSING_FILE,
			OVERWRITES_LOOKUP,
			EXCEEDS_LIMIT,

			DML_INVALID_SERVICE,
			DML_INVALID_PROTOCOL_TYPE,
			DML_INVALID_MESSAGE_TYPE,
			DML_INVALID_MESSAGE_NAME
		};

		explicit value_error(std::string message, code error = code::NONE)
			: runtime_error(message)
		{
			m_code = error;
		}

		code get_error_code() const { return m_code; }
	private:
		code m_code;
	};
}
}