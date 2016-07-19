#ifndef RAE_RANDOM_HPP
#define RAE_RANDOM_HPP

#include <random>
#include <algorithm>

namespace Rae
{

typedef std::mt19937 RandomGeneratorEngine;
extern RandomGeneratorEngine g_randomEngine;

#ifdef _WIN32
double drand48();
#endif

float getRandom();

float getRandom( float from, float to );
float getRandomDistribution(float mean, float deviation);
int getRandomInt( int from, int to );

} // end namespace Rae

#endif