#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::dot;

#include <iostream>

#include "Random.hpp"
#include "Camera.hpp"

namespace rae
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

Camera::Camera(float fieldOfViewRadians, float setAspectRatio, float aperture, float focusDistance)
	: m_fieldOfView(fieldOfViewRadians),
	m_aspectRatio(setAspectRatio),
	m_aperture(aperture),
	m_focusDistance(focusDistance),
	m_yawAngle(0.0f),
	m_pitchAngle(0.0f),
	m_cameraSpeed(2.0f),
	m_rotateSpeed(1.0f)
{
	calculateFrustum();
}

Ray Camera::getRay(float s, float t)
{
	//return Ray(origin, lowerLeftCorner + (s * m_horizontal) + (t * m_vertical) - origin);
	// Normal:
	//return Ray(m_position, m_topLeftCorner + (s * m_horizontal) - (t * m_vertical) - m_position);
	vec3 rd = m_lensRadius * randomInUnitDisk();
	vec3 offset = m_right * rd.x + m_up * rd.y;
	//return Ray(m_position + offset, m_lowerLeftCorner + (s * m_horizontal) + (t * m_vertical) - m_position - offset);
	return Ray(m_position + offset, m_topLeftCorner + (s * m_horizontal) - (t * m_vertical) - m_position - offset);
}

Ray Camera::getExactRay(float s, float t)
{
	//return Ray(origin, lowerLeftCorner + (s * m_horizontal) + (t * m_vertical) - origin);
	// Normal:
	//return Ray(m_position, m_lowerLeftCorner + (s * m_horizontal) + (t * m_vertical) - m_position);
	return Ray(m_position, m_topLeftCorner + (s * m_horizontal) - (t * m_vertical) - m_position);
}

void Camera::calculateFrustum()
{
	m_lensRadius = m_aperture / 2.0f;

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	m_direction = vec3(
		cos(m_pitchAngle) * sin(m_yawAngle), 
		sin(m_pitchAngle),
		cos(m_pitchAngle) * cos(m_yawAngle)
	);
	
	// Right vector
	m_right = glm::vec3(
		sin(m_yawAngle - 3.14f/2.0f),
		0,
		cos(m_yawAngle - 3.14f/2.0f)
	);

	// Up vector
	m_up = glm::cross( m_right, m_direction );

	m_projectionMatrix = glm::perspective(Math::toDegrees(m_fieldOfView), m_aspectRatio, 0.1f, 500.0f);
	m_viewMatrix = glm::lookAt( m_position, m_position + m_direction, m_up );

	//

	vec3 w;
	float halfHeight = tan(m_fieldOfView / 2.0f);

	float halfWidth = m_aspectRatio * halfHeight;
	w = glm::normalize(m_position - (m_position + m_direction));
	
	// Normal way: m_topLeftCorner = m_position - (halfWidth * m_right) + (halfHeight * m_up) - w;
	m_topLeftCorner =
		m_position
		- (halfWidth * m_focusDistance * m_right)
		+ (halfHeight * m_focusDistance * m_up)
		- (m_focusDistance * w);
	/*
	m_lowerLeftCorner =
		m_position
		- (halfWidth * m_focusDistance * m_right)
		- (halfHeight * m_focusDistance * m_up)
		- (m_focusDistance * w);
	*/
	
	m_horizontal = 2.0f * halfWidth * m_focusDistance * m_right;
	m_vertical = 2.0f * halfHeight * m_focusDistance * m_up;

	m_needsUpdate = false;
}

bool Camera::update(double time, double deltaTime)
{
	if( m_focusDistanceAnimator.update((float)time) )
	{
		m_focusDistance = m_focusDistanceAnimator.value();
		m_needsUpdate = true;
	}

	bool ret = m_needsUpdate;
	if (m_needsUpdate)
	{
		calculateFrustum();
	}
	return ret;
}

void Camera::setPosition(vec3 pos)
{
	if (pos != m_position)
	{
		m_needsUpdate = true;
		m_position = pos;
	}
}

void Camera::moveForward(float delta)
{
	if (delta > 0.0f)
	{
		m_needsUpdate = true;
		m_position += delta * m_direction * cameraSpeed();
	}	
}

void Camera::moveBackward(float delta)
{
	if (delta > 0.0f)
	{
		m_needsUpdate = true;
		m_position -= delta * m_direction * cameraSpeed();
	}	
}

void Camera::moveRight(float delta)
{
	if (delta > 0.0f)
	{
		m_needsUpdate = true;
		m_position += delta * m_right * cameraSpeed();
	}	
}

void Camera::moveLeft(float delta)
{
	if (delta > 0.0f)
	{
		m_needsUpdate = true;
		m_position -= delta * m_right * cameraSpeed();
	}	
}

void Camera::moveUp(float delta)
{
	if (delta > 0.0f)
	{
		m_needsUpdate = true;
		m_position += delta * m_up * cameraSpeed();
	}	
}

void Camera::moveDown(float delta)
{
	if (delta > 0.0f)
	{
		m_needsUpdate = true;
		m_position -= delta * m_up * cameraSpeed();
	}	
}

void Camera::rotateYaw(double delta_time, float dir)
{
	m_needsUpdate = true;
	m_yawAngle += float(delta_time) * m_rotateSpeed * dir;
}

void Camera::rotatePitch(float delta_time, float dir)
{
	m_needsUpdate = true;
	m_pitchAngle += float(delta_time) * m_rotateSpeed * dir;
}

void Camera::rotateYaw(float delta)
{
	m_needsUpdate = true;
	m_yawAngle += delta;
}

void Camera::rotatePitch(float delta)
{
	m_needsUpdate = true;
	m_pitchAngle += delta;
}

float Camera::cameraSpeed()
{
	if (m_cameraSpeedUp)
		return m_cameraSpeed * 10.0f;
	else if (m_cameraSpeedDown)
		return m_cameraSpeed * 0.1f;
	return m_cameraSpeed;
}

// also handles negative input unlike other similar funcs
void Camera::plusFieldOfView(float delta/* = 0.1f*/)
{
	if ((m_fieldOfView + delta) >= Math::toRadians(1.0f))
	{
		if ((m_fieldOfView + delta) <= Math::toRadians(180.0f))
		{
			m_fieldOfView += delta;
		}
		else m_fieldOfView = Math::toRadians(180.0f);
	}
	else m_fieldOfView = Math::toRadians(1.0f);
	m_needsUpdate = true;
}

void Camera::minusFieldOfView(float delta/* = 0.1f*/)
{
	if ((m_fieldOfView - delta) >= Math::toRadians(1.0f))
	{
		if ((m_fieldOfView - delta) <= Math::toRadians(180.0f))
		{
			m_fieldOfView -= delta;
		}
		else m_fieldOfView = Math::toRadians(180.0f);
	}
	else m_fieldOfView = Math::toRadians(1.0f);
	m_needsUpdate = true;
}

void Camera::plusAperture(float delta/* = 0.01f*/)
{
	m_aperture += delta;
	m_needsUpdate = true;
}

void Camera::minusAperture(float delta/* = 0.01f*/)
{
	if ((m_aperture - delta) >= 0.0f)
		m_aperture -= delta;
	else m_aperture = 0.0f;
	m_needsUpdate = true;
}

void Camera::plusFocusDistance(float delta/* = 0.01f*/)
{
	m_focusDistance += delta;
	m_needsUpdate = true;
}

void Camera::minusFocusDistance(float delta/* = 0.01f*/)
{
	if ((m_focusDistance - delta) >= MinFocusDistance)
		m_focusDistance -= delta;
	else m_focusDistance = MinFocusDistance;
	m_needsUpdate = true;
}

void Camera::setFocusDistance(float distance)
{
	m_focusDistance = distance;
	m_needsUpdate = true;
}	

vec3 Camera::getFocusPosition()
{
	return m_position + (m_direction * m_focusDistance);
}

void Camera::setFocusPosition(const vec3& pos)
{
	m_focusDistance = glm::length(m_position - pos);
	m_needsUpdate = true;
}

void Camera::animateFocusPosition(const vec3& pos, float duration)
{
	// TODO this is annoyingly complex. Make an animation system, and handle this case better (when no need for an animation after init).
	if (m_focusDistanceAnimator.isFinished())
	{
		if (isCloseEnough(m_focusDistance, glm::length(m_position - pos)) == false)
		{
			m_focusDistanceAnimator.init(m_focusDistance, glm::length(m_position - pos), duration);
			m_needsUpdate = true;
		}
	}
}

bool Camera::shouldWeAutoFocus() const
{
	return m_isContinuousAutoFocus;
}

} // end namespace rae

