#pragma once

namespace rae
{

class Engine;

class Time
{
	friend class Engine;

public:
	double time() const { return m_currentTime; }
	double deltaTime() const { return m_deltaTime; }

protected:
	void initTime(double value)
	{
		m_currentTime = value;
		m_previousTime = value;
		m_deltaTime = 0.0;
	}

	void setTime(double value)
	{
		m_currentTime = value;
		m_deltaTime = m_currentTime - m_previousTime;
	}

	void setPreviousTime()
	{
		 m_previousTime = m_currentTime;
	}

	double m_previousTime = 0.0;
	double m_currentTime = 0.0;
	double m_deltaTime = 0.0;
};

}
