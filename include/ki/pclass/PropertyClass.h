#pragma once

namespace ki
{
namespace pclass
{
	class Type;

	/**
	 * TODO: Documentation
	 */
	class PropertyClass
	{
		explicit PropertyClass(const Type &type) : m_type(type) {}
		virtual ~PropertyClass() {};

		const Type &get_type();

		virtual void on_created();
	private:
		const Type &m_type;
	};
}
}
