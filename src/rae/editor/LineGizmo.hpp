#pragma once

#include "rae/editor/IGizmo.hpp"
#include "rae/core/Types.hpp"
#include "rae/visual/Mesh.hpp"

namespace rae
{

class Ray;
class Camera;
class RenderSystem;

struct LineHandle
{
	int axisIndex = 0;
	vec3 tipPosition = vec3(0.0f, 0.0f, 0.0f);
	float distanceFromCamera = 0.0f;
};

class LineGizmo : public IGizmo
{
public:
	LineGizmo();

	bool hover(const Ray& mouseRay, const Camera& camera);
	void render3D(const Camera& camera, RenderSystem& renderSystem) const;

	vec3 getActiveAxisVector() const;
	vec3 activeAxisDelta(const Camera& camera, const Ray& mouseRay, const Ray& previousMouseRay);// const;

	//debug:
	Line m_debugIntersectionLine;
	Line m_debugLine;

protected:

	// Returns an array of sorted linehandles, based on their position from the camera.
	std::array<LineHandle, (int)Axis::Count> sortLineHandles(float gizmoCameraFactor, const Camera& camera) const;

	float m_gizmoSizeMultiplier = 0.1f; // This should be global to all gizmos.
	float m_hoverMarginMultiplier = 1.4f;
	float m_hoverThicknessMultiplier = 3.0f;
	float m_coneLengthMultiplier = 4.0f;
	Mesh m_lineMesh;
	Mesh m_coneMesh;
};

}
