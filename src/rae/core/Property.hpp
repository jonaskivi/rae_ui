#pragma once

#include "rae/core/Event.hpp"

namespace rae
{

template<typename T>
class Property
{
public:
	Property() :
		m_value(static_cast<T>(0))
	{
	}

	Property(T value) :
		m_value(value)
	{
	}

	Property& operator=(const Property& other)
	{
		m_value = other.m_value;
		onChanged.emit(m_value);
		return *this;
	}

	T& operator=(const T& other)
	{
		m_value = other;
		onChanged.emit(m_value);
		return m_value;
	}

	operator T&() { return m_value; }
	operator T() const { return m_value; }

	Event<T> onChanged;

protected:
	T m_value;
};

using Bool = Property<bool>;
using Float = Property<float>;

}
