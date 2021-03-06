#pragma once

#include <math.h>
#include <glm/glm.hpp>

#include "core/Math.hpp"

namespace rae
{

enum class AnimatorType
{
	Smoothstep,
	SineIn,
	SineOut,
	SineInOut,
	CubicIn,
	CubicOut,
	CubicInOut
};

template<typename T> inline bool isCloseEnough(const T& target, const T& value)
{
	if (target - value == static_cast<T>(0.0)) // Maybe this doesn't work with doubles.
		return true;
	return false;
}

template<> inline bool isCloseEnough<float>(const float& target, const float& value)
{
	if (std::abs(target - value) < 0.001f)
		return true;
	return false;
}

template<> inline bool isCloseEnough<glm::vec3>(const glm::vec3& target, const glm::vec3& value)
{
	glm::vec3 temp = target - value;
	float lengthSqr = glm::dot(temp, temp);
	if (lengthSqr < 0.001f)
		return true;
	return false;
}

template <typename T>
class Animator
{
public:

	Animator() :
		m_value(),
		m_startValue(),
		m_valueChange(),
		m_startTime(0.0f),
		m_duration(0.0f),
		m_animatorType(AnimatorType::Smoothstep)
	{
	}

	Animator(
		T startValue,
		T targetValue,
		float duration,
		AnimatorType setType = AnimatorType::Smoothstep,
		bool isLooping = false)
	{
		init(startValue, targetValue, duration, setType, isLooping);
	}

	void init(
		T startValue,
		T targetValue,
		float startTime,
		float duration,
		AnimatorType setType = AnimatorType::Smoothstep,
		bool isLooping = false)
	{
		m_value = startValue;
		m_startValue = startValue;
		m_valueChange = targetValue - startValue;
		m_startTime = startTime;
		m_duration = duration;
		if (m_duration == 0.0f)
		{
			m_duration = 5.0f;
		}
		m_animatorType = setType;
		m_isLooping = isLooping;
		m_running = true;
	}

	// A deferred init, which can be used to init without knowing the startTime. update() will finish initing.
	void init(
		T startValue,
		T targetValue,
		float duration,
		AnimatorType setType = AnimatorType::Smoothstep,
		bool isLooping = false)
	{
		float startTime = -5.0f; // Unknown at this point, so we set it to a magic value -5.0f.

		init(startValue,
			targetValue,
			startTime,
			duration,
			setType,
			isLooping);
	}

	~Animator(){}

	// Value must be between 0.0 - 1.0
	float smoothstep(float value)
	{
		return value * value * (3.0f - 2.0f * (value));
	}

	T smoothstepEase(float time, T startValue, T valueChange, float duration)
	{
		float v = time / duration;
		v = smoothstep(v);
		return (startValue * v) + ((startValue + valueChange) * (1.0f - v));
	}

	T sineEaseIn(float time, T startValue, T valueChange, float duration)
	{
		return -valueChange * cosf(time / duration * Math::QuarterTau) + valueChange + startValue;
	}

	T sineEaseOut(float time, T startValue, T valueChange, float duration)
	{
		return valueChange * sinf(time / duration * Math::QuarterTau) + startValue;
	}

	T sineEaseInOut(float time, T startValue, T valueChange, float duration)
	{
		return -valueChange / 2.0f * (cosf(Math::Pi * time / duration) - 1.0f) + startValue;
	}

	T cubicEaseIn(float time, T startValue, T valueChange, float duration)
	{
		time /= duration;
		return valueChange * time * time * time + startValue;
	}

	T cubicEaseOut(float time, T startValue, T valueChange, float duration)
	{
		time = (time / duration) - 1.0f;
		return valueChange * (time * time * time + 1.0f) + startValue;
	}

	static T cubicEaseInOut(float time, T startValue, T valueChange, float duration)
	{
		time /= duration / 2.0f;
		if (time < 1.0f)
			return valueChange / 2.0f * time * time * time + startValue;
		time -= 2.0f;
		return valueChange / 2.0f * (time * time * time + 2.0f) + startValue;
	}

	void restart()
	{
		m_value = m_startValue;
		m_startTime = -5.0f;
		m_running = true;
	}

	// returns true if the Animator is still running.
	bool update(float currentTime)
	{
		if (m_startTime == -5.0f)
		{
			// Finalize the deferred init.
			m_startTime = currentTime;
		}

		if (m_running == false)
			return false;

		// Clamp to targetValue if we are finished
		if (isFinished(currentTime))
		{
			finish(targetValue());

			if (m_isLooping)
			{
				// This means that when looping, the target value is never reached 100%, only 99.9% (or the
				// amount of ellipsis used in isCloseEnough) as restart will just jump back to the start.
				restart();
			}

			return true; // Return true one more time, so that the value is updated to target.
		}

		switch (m_animatorType)
		{
			case AnimatorType::Smoothstep:
				m_value = smoothstepEase(currentTime - m_startTime, m_startValue, m_valueChange, m_duration);
				break;
			case AnimatorType::SineIn:
				m_value = sineEaseIn(currentTime - m_startTime, m_startValue, m_valueChange, m_duration);
				break;
			case AnimatorType::SineOut:
				m_value = sineEaseOut(currentTime - m_startTime, m_startValue, m_valueChange, m_duration);
				break;
			default:
			case AnimatorType::SineInOut:
				m_value = sineEaseInOut(currentTime - m_startTime, m_startValue, m_valueChange, m_duration);
				break;
			case AnimatorType::CubicIn:
				m_value = cubicEaseIn(currentTime - m_startTime, m_startValue, m_valueChange, m_duration);
				break;
			case AnimatorType::CubicOut:
				m_value = cubicEaseOut(currentTime - m_startTime, m_startValue, m_valueChange, m_duration);
				break;
			case AnimatorType::CubicInOut:
				m_value = cubicEaseInOut(currentTime - m_startTime, m_startValue, m_valueChange, m_duration);
				break;
		}

		return true;
	}

	void finish(T set)
	{
		m_value = set;
		m_running = false;
	}

	bool isFinished() // Could also be called isTargetReached
	{
		if (m_running == false || isCloseEnough<T>(targetValue(), m_value))
			return true;
		return false;
	}

	bool isFinished(float currentTime)
	{
		// First check if target is reached, then check if time has ended.
		if (isCloseEnough<T>(targetValue(), m_value))
			return true;
		return hasTimeEnded(currentTime);
	}

	bool hasTimeEnded(float currentTime)
	{
		if (currentTime > endTime())
			return true;
		return false;
	}

	const T& value() const { return m_value; }
	const T& startValue() const { return m_startValue; }
	T targetValue() const { return m_startValue + m_valueChange; }
	void  setTargetValue(T targetValue) { m_valueChange = targetValue - m_startValue; }
	const T& valueChange() const { return m_valueChange; }
	float duration() const { return m_duration; }
	float endTime() const { return m_startTime + m_duration; }
	bool isLooping() const { return m_isLooping; }

private:
	T m_value;

	T m_startValue;
	T m_valueChange; // targetValue - startValue
	float m_startTime;
	float m_duration;
	bool m_isLooping = false;
	// We need to mark when it is finished, because the update() needs to run one more time
	// after it has actually finished, so that who ever is calling update, can update the final value.
	bool m_running = false;

	AnimatorType m_animatorType;
};

}
