#pragma once

#include <functional>

#include "rae/core/Types.hpp"

namespace rae
{

template<typename T>
class Event
{
public:
	void connect(std::function<void(const T&)> handler)
	{
		m_handlers.emplace_back(handler);
	}

	void emit(const T& value)
	{
		for (auto&& handler : m_handlers)
		{
			handler(value);
		}
	}

protected:
	Array<std::function<void(const T&)>> m_handlers;
};

}
