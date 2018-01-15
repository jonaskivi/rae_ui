#pragma once

#include <iostream>

#define RAE_LOG_TARGET std::cout
#define RAE_LOG_ERROR_TARGET std::cerr

#ifndef rae_log
#define rae_log(...) do_log(__VA_ARGS__)
// To disable log output on compile time:
//#define rae_log(...)
#endif

#ifndef rae_log_error
#define rae_log_error(...) do_log_error(__VA_ARGS__)
#endif

#if __cplusplus >= 201103L // C++11
#include <mutex>

#include <stdarg.h>

namespace rae
{

inline std::ostream& log_one(std::ostream& os)
{
	return os;
}

template <class A0, class ...Args>
inline std::ostream& log_one(std::ostream& os, const A0& a0, const Args& ...args)
{
	os << a0;
	return log_one(os, args...);
}

template <class ...Args>
inline std::ostream& do_log(std::ostream& os, const Args& ...args)
{
	return log_one(os, args...);
}

template <class ...Args>
inline std::ostream& do_log(const Args& ...args)
{
	static std::mutex m;
	std::lock_guard<std::mutex> _(m);
	return do_log(RAE_LOG_TARGET, args...);
}

template <class ...Args>
inline std::ostream& do_log_error(const Args& ...args)
{
	static std::mutex m;
	std::lock_guard<std::mutex> _(m);
	return do_log(RAE_LOG_ERROR_TARGET, "ERROR: ", args...);
}

} // end namespace rae

#else // C++98 version

namespace rae
{

void do_log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	while (*fmt != '\0')
	{
		if (*fmt == 'd')
		{
			int i = va_arg(args, int);
			std::cout << i << '\n';
		}
		else if (*fmt == 's')
		{
			char* s = va_arg(args, char*);
			RAE_LOG_TARGET << s << '\n';
		}
		++fmt;
	}

	va_end(args);
}

void do_log_error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	while (*fmt != '\0')
	{
		if (*fmt == 'd')
		{
			int i = va_arg(args, int);
			std::cout << i << '\n';
		}
		else if (*fmt == 's')
		{
			char* s = va_arg(args, char*);
			RAE_LOG_ERROR_TARGET << "ERROR: " << s << '\n';
		}
		++fmt;
	}

	va_end(args);
}

} // end namespace rae

#endif
