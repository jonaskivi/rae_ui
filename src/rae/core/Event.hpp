#pragma once

#include <functional>

#include "rae/core/Types.hpp"

namespace rae
{

template<typename ...Args>
class Event
{
public:
	void connect(std::function<void(const Args& ...)> handler)
	{
		m_handlers.emplace_back(handler);
	}

	void emit(const Args& ...args)
	{
		for (auto&& handler : m_handlers)
		{
			handler(args...);
		}
	}

protected:
	Array<std::function<void(const Args& ...)>> m_handlers;
};

}
