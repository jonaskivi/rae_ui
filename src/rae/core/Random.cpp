#include <random>
#include <algorithm>

#include "Random.hpp"

namespace rae
{

RandomGeneratorEngine g_randomEngine;

#ifdef _WIN32
double drand48()
{
	return getRandom(0.0, 1.0);
}
#endif

float getRandom()
{
#ifdef _WIN32
	return getRandom( 0.0f, 1.0f );
#else
	return drand48();
#endif
}

float getRandom( float from, float to )
{
	static std::uniform_real_distribution<float> uniform_dist;
	return uniform_dist( g_randomEngine, std::uniform_real_distribution<float>::param_type(from, to) );
}

float getRandomDistribution(float mean, float deviation)
{
	static std::normal_distribution<float> normal_dist(mean, deviation);
	return normal_dist(g_randomEngine, std::normal_distribution<float>::param_type(mean, deviation));
}

int getRandomInt( int from, int to )
{
	static std::uniform_int_distribution<int> uniform_dist;
	return uniform_dist( g_randomEngine, std::uniform_int_distribution<int>::param_type(from, to) );
}

} // end namespace rae
