#include "rae/scene/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "rae/core/Utils.hpp"

namespace rae
{

Transform::Transform(vec3 const& position) :
	position(position)
{
}

Transform::Transform(vec3 const& position, qua const& rotation) :
	position(position),
	rotation(rotation)
{
}

Transform::Transform(vec3 const& position, qua const& rotation, vec3 const& scale) :
	position(position),
	rotation(rotation),
	scale(scale)
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
