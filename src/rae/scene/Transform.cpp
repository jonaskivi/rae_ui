#include "rae/scene/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "rae/core/Utils.hpp"

namespace rae
{

Transform::Transform(const vec3& position) :
	position(position)
{
}

String Transform::toString() const
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
