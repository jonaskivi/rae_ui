#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::dot;

#include "Random.hpp"

namespace Rae
{

vec3 randomInUnitDisk()
{
	vec3 point;
	do
	{
		point = 2.0f * vec3(getRandom(), getRandom(), 0) - vec3(1,1,0);
	} while(dot(point, point) >= 1.0f);
	return point;
}

} // end namespace Rae

