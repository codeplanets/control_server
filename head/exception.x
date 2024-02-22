#pragma once

#include <string>
#include <exception>

namespace core {
    class exception_format : public std::exception
	{
	private:
		std::string		m_strWhat;

	public:
		exception_format(const char* pszFormat, ...);
		~exception_format(void) noexcept {}

		const char*	what(void) const throw() {
            return m_strWhat.c_str();
        }
	};
}