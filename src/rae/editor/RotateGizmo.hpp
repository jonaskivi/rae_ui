#pragma once

#include "rae/editor/IGizmo.hpp"
#include "rae/core/Types.hpp"
#include "rae/visual/Mesh.hpp"

namespace rae
{

class Ray;
class Camera;
class ShapeRenderer;
class RenderSystem;

class RotateGizmo : public IGizmo
{
public:
	RotateGizmo()
	{
	}

	bool hover(const Ray& mouseRay, const Camera& camera);
	void render3D(const Camera& camera, ShapeRenderer& shapeRenderer) const;

	qua getRotateAxisDelta(
			const vec2& mouseDelta,
			const Camera& camera,
			const vec3& gizmoPosition,
			GizmoAxis gizmoAxis,
			bool snapEnabled,
			float snapAngleStep,
			bool precisionModifier = false
		);

	void clearAccumulatedRotation()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_accumulatedAngles[i] = 0.0f;
		}

		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_rotatedAngles[i] = 0.0f;
		}
	}

private:
	vec3 getTangentPosition(const vec3& gizmoPosition) const { return gizmoPosition + m_tangentRelativePosition; }

	// When snapping we use these accumulated angles to determine if we have crossed over to the
	// next snap step. When snapping is off, this is the same as m_rotatedAngles.
	std::array<float, 3>		m_accumulatedAngles = { 0.0f, 0.0f, 0.0f };
	// The actual active rotation we have done. When snapping, this is locked to the current snap step.
	std::array<float, 3>		m_rotatedAngles = { 0.0f, 0.0f, 0.0f };

	// The tangent position is the hovered and clicked position on the axis circle. It determines
	// the direction where the user must move the mouse cursor to affect the delta rotation.
	// Because our hovering uses Capsule intersection, the tangent position is not accurate, but
	// it is just the center of the hovered capsule. Its accuracy is then determined by
	// t_rotateGizmoHoverResolution, which determines how many capsules we use.
	// It must be defined relative to the gizmo center which usually is m_position
	// (except in quicktransform mode it is camera target position), because the gizmo might move
	// while rotating.
	vec3		m_tangentRelativePosition = vec3(0.0f);
	qua			m_tangentRotation;
	// The active tangent angle in relation to disk forward vector.
	float		m_tangentAngle = 0.0f;
	// We need to store the rotation that the active axis disk (or plane) has at the time when we
	// start the rotation. Then we can show a visualization for the rotatedAngles.
	qua			m_originalActiveDiskRotation;

	float m_gizmoSizeMultiplier = 0.1f; // This should be global to all gizmos.
	float m_rotateGizmoRadius = 1.5f;
	// m_rotateGizmoHandleThickness (visual) is multiplied by this for hovering.
	float m_proximityMultiplier = 6.0f;

	// How many capsules we should use for hovering the rotate handles.
	// Also affects tangent arrow centers and its accuracy.
	int m_rotateGizmoHoverResolution = 16;

	// Visual thickness of the axis line. RAE_TODO thick rendering.
	float m_rotateGizmoHandleThickness = 0.05f;

	float m_rotateGizmoTangentArrowLength = 0.5f;
	float m_rotateGizmoTangentArrowTipSize = 0.2f;
	float m_rotateGizmoTangentArrowTipWidthMultiplier = 0.5f;

	// Mouse input get's multiplied by this value for rotation speed.
	float m_rotateGizmoSpeedMul = 0.005f;
	// Holding Ctrl will make the speed this much slower.
	float m_rotateGizmoPrecisionMul = 0.1f;

	// Debug visualization
	int m_debugRotateGizmoHoverAxis = -1; // -1 for none. 0 for all. 1-3 for individual axes.
};

}
