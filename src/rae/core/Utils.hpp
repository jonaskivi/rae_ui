#pragma once

#include <string>
#include <functional>
#include <algorithm>
#include <thread>

#include <glm/glm.hpp>

#include "rae/core/Types.hpp"

#include "rae/core/make_unique.hpp"

namespace rae
{

namespace Utils
{

void sleep(int durationMilliSeconds);

// Pass the enum parameter as cast to int
// Usage: m_renderMode = (RenderMode) Utils::wrapEnum(((int)m_renderMode) + 1, (int)RenderMode::Count);
int wrapEnum(int enumValue, int enumCount);

// Give input as 255 "8-bit" floats and the output will be a 1.0f float color.
Color createColor8bit(float r, float g, float b, float a = 255.0f);

String toString(bool set);
String toString(float value); // A string with four decimal places
String toString(int value);
String toString(glm::vec3 const& position);

}

/* A simple parallel for loop.
// Usage example:
parallel_for(0, array.size(), [&](int i)
{
	array[i] = computeSomeResult();
});
*/
template<typename Callable>
static void parallel_for(int start, int end, Callable func)
{
	const static int threadCountHint = (int)std::thread::hardware_concurrency();
	const static int threadCount = (threadCountHint == 0 ? 8 : threadCountHint);

	std::vector<std::thread> threads(threadCount);

	for (int t = 0; t < threadCount; ++t)
	{
		threads[t] = std::thread(std::bind(
		[&](const int beginIndex, const int endIndex, const int t)
		{
			for (int i = beginIndex; i < endIndex; ++i)
			{
				func(i);
			}
		},
		t * end / threadCount,
		(t+1) == threadCount ? end : (t+1) * end / threadCount,
		t));
	}

	std::for_each(threads.begin(),threads.end(), [](std::thread& x)
	{
		x.join();
	});
}

} // namespace rae
