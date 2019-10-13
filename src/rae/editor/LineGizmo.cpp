#include "rae/editor/LineGizmo.hpp"

#include "rae/visual/Ray.hpp"
#include "rae/visual/Camera.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/visual/Plane.hpp"

namespace rae
{

// To IGizmo.cpp:
String gizmoPivotToString(GizmoPivot value)
{
	switch(value)
	{
		case GizmoPivot::Auto: return "Auto";
		case GizmoPivot::Center: return "Center";
		case GizmoPivot::First: return "First";
		case GizmoPivot::Last: return "Last";
		case GizmoPivot::Workplane: return "Workplane";
		case GizmoPivot::Count: return "Count";
	}
	return "Invalid";
}

String gizmoAxisToString(GizmoAxis value)
{
	switch(value)
	{
		case GizmoAxis::World: return "World";
		case GizmoAxis::Local: return "Local";
		case GizmoAxis::Workplane: return "Workplane";
		case GizmoAxis::Screen: return "Screen";
		case GizmoAxis::Count: return "Count";
	}
	return "Invalid";
}

const float AxisThickness = 0.05f;

LineGizmo::LineGizmo()
{
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		m_axisTransforms[i].position = vec3(0.0f, 0.0f, 0.0f);
		m_axisTransforms[i].rotation = axisRotation(Axis(i));
		m_axisTransforms[i].scale = vec3(1.0f, AxisThickness, AxisThickness);
	}

	m_lineMesh.generateCube();
	m_lineMesh.createVBOs();

	m_coneMesh.generateCone();
	m_coneMesh.createVBOs();
}

std::array<LineHandle, (int)Axis::Count> LineGizmo::sortLineHandles(
	float gizmoCameraFactor,
	const Camera& camera) const
{
	std::array<LineHandle, (int)Axis::Count> sortedLineHandles;

	// Sort line handles
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		sortedLineHandles[i].axisIndex = i;
		sortedLineHandles[i].tipPosition = m_position + (axisVector(Axis(i)) * gizmoCameraFactor);
		sortedLineHandles[i].distanceFromCamera =
			glm::length(sortedLineHandles[i].tipPosition - camera.position());
	}

	// Sort from closest to farthest
	std::sort(sortedLineHandles.begin(), sortedLineHandles.end(),
		[](const LineHandle& a, const LineHandle& b)
	{
		return a.distanceFromCamera < b.distanceFromCamera;
	});

	return sortedLineHandles;
}

bool LineGizmo::hover(const Ray& mouseRay, const Camera& camera)
{
	const float MinHoverDistance = 0.0f;
	const float MaxHoverDistance = 900000.0f;

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);
	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier * m_hoverMarginMultiplier;

	auto sortedLineHandles = sortLineHandles(gizmoCameraFactor, camera);

	clearHovers();

	for (auto&& lineHandle : sortedLineHandles)
	{
		int i = lineHandle.axisIndex;

		Transform transform = m_axisTransforms[i];
		transform.scale = transform.scale *
			gizmoCameraFactor * vec3(1.0f, m_hoverThicknessMultiplier, m_hoverThicknessMultiplier);
		// The handle's box needs to be centered with that 0.5
		transform.position = m_position + (m_rotation * (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor));
		transform.rotation = m_rotation * transform.rotation;

		Box axisBox = m_lineMesh.getAabb();
		axisBox.transform(transform);
		bool isHit = axisBox.hit(mouseRay, MinHoverDistance, MaxHoverDistance);
		m_axisHovers[i] = isHit;

		if (isHit)
		{
			return true;
		}
	}
	return false;
}

void LineGizmo::render3D(const Camera& camera, RenderSystem& renderSystem) const
{
	float gizmoCameraFactor = camera.screenSizeFactor(m_position) * m_gizmoSizeMultiplier;

	auto sortedLineHandles = sortLineHandles(gizmoCameraFactor, camera);

	const auto hoverColor = Utils::createColor8bit(255, 165, 0);
	const auto activeColor = Utils::createColor8bit(255, 215, 0);

	// Draw from farthest to closest, so iterate backwards
	for (int index = int(sortedLineHandles.size())-1; index >= 0; --index)
	{
		int i = sortedLineHandles[index].axisIndex;

		Color color = m_axisActives[i] ? activeColor : (m_axisHovers[i] ? hoverColor : axisColor(Axis(i)));

		// Draw line as a box
		Transform transform = m_axisTransforms[i];
		transform.scale = transform.scale * gizmoCameraFactor;
		// The handle's box needs to be centered with that 0.5
		transform.position = m_position + (m_rotation * (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor));
		transform.rotation = m_rotation * transform.rotation;
		renderSystem.renderMeshSingleColor(
			camera,
			transform,
			color,
			m_lineMesh);

		// Draw cone
		Transform coneTransform = m_axisTransforms[i];
		float coneSize = m_gizmoSizeMultiplier;
		coneTransform.scale = vec3(coneSize, coneSize * m_coneLengthMultiplier, coneSize) * gizmoCameraFactor;
		coneTransform.rotation = m_rotation * coneTransform.rotation *
			glm::angleAxis(-Math::QuarterTau, vec3(0.0f, 0.0f, 1.0f));
		coneTransform.position = m_position + (m_rotation * (axisVector(Axis(i)) * gizmoCameraFactor));
		renderSystem.renderMeshSingleColor(
			camera,
			coneTransform,
			color,
			m_coneMesh);
	}
}

vec3 LineGizmo::getActiveAxisVector() const
{
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		if (m_axisActives[i])
			return axisVector((Axis)i);
	}
	return vec3();
}

vec3 LineGizmo::activeAxisDelta(const Camera& camera, const Ray& mouseRay, const Ray& previousMouseRay)
{
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		if (not m_axisActives[i])
			continue;

		Axis axis = (Axis)i;
		Plane bestPlane = computeMostPerpendicularAxisPlane(axis, m_position,
			glm::normalize(mouseRay.direction()));

		vec3 intersection;
		bool hit = rayPlaneIntersection(mouseRay, bestPlane, intersection);
		if (hit)
		{
			vec3 previousIntersection;
			bool previousHit = rayPlaneIntersection(previousMouseRay, bestPlane, previousIntersection);
			if (previousHit)
			{
				vec3 deltaVec = intersection - previousIntersection;

				// RAE_TODO REMOVE:
				/*
				g_debugSystem->showDebugText("length: " + Utils::toString(glm::length(deltaVec)), Colors::magenta);
				g_debugSystem->drawLine({ intersection, previousIntersection }, Colors::magenta);
				g_debugSystem->drawLine({ mouseRay.origin(), mouseRay.direction() * 15.0f }, Colors::cyan);
				g_debugSystem->drawLine({ bestPlane.origin(), bestPlane.origin() + (bestPlane.normal() * 5.0f) }, Colors::red);
				*/

				float dotProduct = glm::dot(deltaVec, axisVector(axis));
				// RAE_TODO REMOVE g_debugSystem->showDebugText("dotProduct: " + Utils::toString(dotProduct), Colors::magenta);

				// RAE_TODO REMOVE g_debugSystem->drawLine({ intersection, intersection + (axisVector(axis) * dotProduct) }, Colors::white);

				m_debugIntersectionLine = Line { { intersection, intersection + (axisVector(axis) * dotProduct) }, Colors::cyan };
				m_debugLine = Line { { bestPlane.origin(), bestPlane.origin() + (bestPlane.normal() * 2.0f) }, Colors::yellow };

				return axisVector(axis) * dotProduct;
			}
		}
	}
	return vec3();
}

}
