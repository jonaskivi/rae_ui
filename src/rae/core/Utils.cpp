#include "rae/core/Utils.hpp"

// For sleeping:
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace rae
{
namespace Utils
{

void sleep(int durationMilliSeconds)
{
	#ifdef _WIN32
	Sleep(durationMilliSeconds);
	#else
	usleep(durationMilliSeconds*1000);
	#endif
}

int wrapEnum(int enumValue, int enumCount)
{
	if (enumValue < 0)
		return enumCount-1;
	if (enumValue >= enumCount)
		return 0;
	return enumValue;
}

Color createColor8bit(float r, float g, float b, float a)
{
	return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

String toString(bool set)
{
	return set ? "true" : "false";
}

String toString(float value)
{
	String ret(16, '\0');
	auto written = std::snprintf(&ret[0], ret.size(), "%.2f", value);
	ret.resize(written);
	return ret;
}

String toString(int value)
{
	return std::to_string(value);
}

String toString(glm::vec3 const& position)
{
	String ret = "x: ";
	ret += Utils::toString(position.x);
	ret += ", y: ";
	ret += Utils::toString(position.y);
	ret += ", z: ";
	ret += Utils::toString(position.z);
	return ret;
}

}
}
