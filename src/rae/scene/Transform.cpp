#include "rae/scene/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "rae/core/Utils.hpp"
#include "rae/core/Math.hpp"

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
	ret += ", scale: ";
	ret += Utils::toString(scale);

	vec3 angles = Math::quaToEuler(rotation);
	ret += ", yaw: ";
	ret += Utils::toString(Math::toDegrees(angles.x));
	ret += ", pitch: ";
	ret += Utils::toString(Math::toDegrees(angles.y));
	ret += ", roll: ";
	ret += Utils::toString(Math::toDegrees(angles.z));

	return ret;
}

}
