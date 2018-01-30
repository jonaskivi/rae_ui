#pragma once

#include <ciso646>

#include "rae/core/Types.hpp"

#include "core/Utils.hpp"
#include "rae/visual/Ray.hpp"

#include "rae/animation/Animator.hpp"

namespace rae
{

vec3 randomInUnitDisk();

const float MinFocusDistance = 0.01f;

class Camera
{
public:
	Camera();
	Camera(float fieldOfViewRadians, float setAspectRatio, float aperture, float focusDistance);

	// s and t are from 0.0f to 1.0f, s being x, and t being y coordinate.
	// Top left corner is 0.0f, 0.0f and center 0.5f, 0.5f.
	Ray getRay(float s, float t) const;
	Ray getExactRay(float s, float t) const;

	void calculateFrustum();

	glm::mat4 getProjectionAndViewMatrix() const { return m_projectionMatrix * m_viewMatrix; }
	const glm::mat4& viewMatrix() const { return m_viewMatrix; }
	const glm::mat4& projectionMatrix() const { return m_projectionMatrix; }

	// return true if frustum was updated
	bool update(double time);

	bool needsUpdate() const { return m_needsUpdate; }
	void setNeedsUpdate() { m_needsUpdate = true; }

	const vec3& position() const { return m_position; }
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

	float yaw() const { return m_yawAngle; }
	void setYaw(float set) { m_yawAngle = set; m_needsUpdate = true; }
	float pitch() const { return m_pitchAngle; }
	void setPitch(float set) { m_pitchAngle = set; m_needsUpdate = true; }

	float cameraSpeed() const;
	void setCameraSpeedUp(bool set) { m_cameraSpeedUp = set; }
	void setCameraSpeedDown(bool set) { m_cameraSpeedDown = set; }

	void setAspectRatio(float aspect) { m_aspectRatio = aspect; m_needsUpdate = true; }

	float fieldOfView() const { return m_fieldOfView; } // in radians
	// also handles negative input unlike other similar funcs
	void setFieldOfView(float radians) { m_fieldOfView = radians; }
	void setFieldOfViewDeg(float degrees) { m_fieldOfView = Math::toRadians(degrees); }
	void plusFieldOfView(float delta = 0.1f);
	void minusFieldOfView(float delta = 0.1f);

	float aperture() const { return m_aperture; }
	void setAperture(float set) { m_aperture = set; m_needsUpdate = true; }
	void plusAperture(float delta = 0.01f);
	void minusAperture(float delta = 0.01f);

	float focusDistance() const { return m_focusDistance; }
	void plusFocusDistance(float delta = 0.01f);
	void minusFocusDistance(float delta = 0.01f);
	void setFocusDistance(float distance);
	vec3 getFocusPosition() const;
	void setFocusPosition(const vec3& pos);
	void animateFocusPosition(const vec3& pos, float duration);

	float focusSpeed() const { return m_focusSpeed; }

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
	float m_aperture = 0.1f;
	float m_focusDistance = 10.0f;
	Animator<float> m_focusDistanceAnimator;
	float m_lensRadius;

	float m_yawAngle = 0.0f;
	float m_pitchAngle = 0.0f;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

	bool m_cameraSpeedUp = false;
	bool m_cameraSpeedDown = false;
	float m_cameraSpeed = 2.0f;
	float m_rotateSpeed = 1.0f;
	float m_focusSpeed = 0.5f;
	// Continuous auto focus when camera moves
	bool m_isContinuousAutoFocus = true;

	bool m_needsUpdate = true;
};

}
