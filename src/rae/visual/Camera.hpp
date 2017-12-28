#pragma once

#include <ciso646>

//#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;

#include "core/Utils.hpp"
#include "rae/visual/Ray.hpp"

#include "rae/animation/Animator.hpp"

namespace rae
{

vec3 randomInUnitDisk();

class Camera
{
public:
	Camera(float fieldOfViewRadians, float setAspectRatio, float aperture, float focusDistance);

	Ray getRay(float s, float t);
	Ray getExactRay(float s, float t);

	void calculateFrustum();

	glm::mat4 getProjectionAndViewMatrix() const { return m_projectionMatrix * m_viewMatrix; }
	const glm::mat4& viewMatrix() const { return m_viewMatrix; }
	const glm::mat4& projectionMatrix() const { return m_projectionMatrix; }

	// return true if frustum was updated
	bool update(double time, double deltaTime);

	bool needsUpdate() const { return m_needsUpdate; }
	void setNeedsUpdate() { m_needsUpdate = true; }

	const vec3& position() { return m_position; }
	void setPosition(vec3 pos);
	void moveForward(float delta);
	void moveBackward(float delta);
	void moveRight(float delta);
	void moveLeft(float delta);
	void moveUp(float delta);
	void moveDown(float delta);
	void rotateYaw(double delta_time, float dir);
	void rotatePitch(float delta_time, float dir);
	void rotateYaw(float delta);
	void rotatePitch(float delta);

	float yaw() { return m_yawAngle; }
	void setYaw(float set) { m_yawAngle = set; m_needsUpdate = true; }
	float pitch() { return m_pitchAngle; }
	void setPitch(float set) { m_pitchAngle = set; m_needsUpdate = true; }

	float cameraSpeed();
	void setCameraSpeedUp(bool set) { m_cameraSpeedUp = set; }
	void setCameraSpeedDown(bool set) { m_cameraSpeedDown = set; }

	void setAspectRatio(float aspect) { m_aspectRatio = aspect; m_needsUpdate = true; }

	float fieldOfView() { return m_fieldOfView; } // in radians
	// also handles negative input unlike other similar funcs
	void setFieldOfView(float radians) { m_fieldOfView = radians; }
	void setFieldOfViewDeg(float degrees) { m_fieldOfView = Math::toRadians(degrees); }
	void plusFieldOfView(float delta = 0.1f);
	void minusFieldOfView(float delta = 0.1f);

	float aperture() { return m_aperture; }
	void setAperture(float set) { m_aperture = set; m_needsUpdate = true; }
	void plusAperture(float delta = 0.01f);
	void minusAperture(float delta = 0.01f);

	float focusDistance() { return m_focusDistance; }
	void plusFocusDistance(float delta = 0.01f);
	const float MinFocusDistance = 0.01f;
	void minusFocusDistance(float delta = 0.01f);
	void setFocusDistance(float distance);
	vec3 getFocusPosition();
	void setFocusPosition(const vec3& pos);
	void animateFocusPosition(const vec3& pos, float duration);

	float focusSpeed() { return m_focusSpeed; }

	void toggleContinuousAutoFocus() { m_isContinuousAutoFocus = !m_isContinuousAutoFocus; }
	bool isContinuousAutoFocus() const { return m_isContinuousAutoFocus; }
	bool shouldWeAutoFocus() const;

protected:

	vec3 m_position = vec3(13.0f, 2.0f, 3.0f);
	vec3 m_direction = vec3(0.0f, 0.0f, -1.0f);
	vec3 m_right = vec3(1,0,0); // u
	vec3 m_up = vec3(0,1,0); // v

	vec3 m_worldUpAxis = vec3(0.0f, 1.0f, 0.0f);

	//vec3 m_lowerLeftCorner = vec3(-2.0f, -1.0f, -1.0f);
	vec3 m_topLeftCorner = vec3(-2.0f, 1.0f, -1.0f);
	vec3 m_horizontal = vec3(4.0f, 0.0f, 0.0f);
	vec3 m_vertical = vec3(0.0f, 2.0f, 0.0f);

	float m_fieldOfView; // in radians
	float m_aspectRatio;
	float m_aperture;
	float m_focusDistance;
	Animator<float> m_focusDistanceAnimator;
	float m_lensRadius;

	float m_yawAngle;
	float m_pitchAngle;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

	bool m_cameraSpeedUp = false;
	bool m_cameraSpeedDown = false;
	float m_cameraSpeed;
	float m_rotateSpeed;
	float m_focusSpeed = 0.5f;
	// Continuous auto focus when camera moves
	bool m_isContinuousAutoFocus = true;

	bool m_needsUpdate = true;
};

}
