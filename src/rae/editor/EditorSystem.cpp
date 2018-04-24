#include "rae/editor/EditorSystem.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "rae/core/Utils.hpp"
#include "rae/scene/SceneSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/visual/Plane.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/ui/UISystem.hpp"

using namespace rae;

const float AxisThickness = 0.05f;

vec3 axisVector(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return vec3(1.0f, 0.0f, 0.0f);
		case Axis::Y:
			return vec3(0.0f, 1.0f, 0.0f);
		case Axis::Z:
			return vec3(0.0f, 0.0f, 1.0f);
		default:
			assert(0);
			break;
	}
	return vec3(0.0f, 0.0f, 0.0f);
}

Color axisColor(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return Color(1.0f, 0.0f, 0.0f, 1.0f);
		case Axis::Y:
			return Color(0.0f, 1.0f, 0.0f, 1.0f);
		case Axis::Z:
			return Color(0.0f, 0.0f, 1.0f, 1.0f);
		default:
			assert(0);
			break;
	}
	return Color(1.0f, 1.0f, 1.0f, 1.0f);
}

qua axisRotation(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return qua();
		case Axis::Y:
			return qua(vec3(0.0f, 0.0f, Math::toRadians(90.0f)));
		case Axis::Z:
			return qua(vec3(0.0f, -Math::toRadians(90.0f), 0.0f));
		default:
			assert(0);
			break;
	}
	return qua();
}

Plane axisPlane(Axis axis, vec3 origin)
{
	return Plane(origin, axisVector(axis));
}

Plane computeMostPerpendicularAxisPlane(Axis axis, const vec3& gizmoOrigin, const vec3& rayDirection)
{
	Plane plane1;
	Plane plane2;

	// Check which other axis plane is the most perpendicular to the given ray.
	// For X axis consider Y and X axes.
	switch(axis)
	{
		case Axis::X:
			plane1 = axisPlane(Axis::Y, gizmoOrigin);
			plane2 = axisPlane(Axis::Z, gizmoOrigin);
			break;
		case Axis::Y:
			plane1 = axisPlane(Axis::X, gizmoOrigin);
			plane2 = axisPlane(Axis::Z, gizmoOrigin);
			break;
		case Axis::Z:
			plane1 = axisPlane(Axis::X, gizmoOrigin);
			plane2 = axisPlane(Axis::Y, gizmoOrigin);
			break;
		default:
			assert(0);
			break;
	}

	float perpendicular1 = glm::dot(rayDirection, plane1.normal());
	float perpendicular2 = glm::dot(rayDirection, plane2.normal());

	if (fabs(perpendicular1) >= fabs(perpendicular2))
	{
		return plane1;
	}
	else
	{
		return plane2;
	}
}

//RAE_TODO move to Ray.cpp that needs to be created:
bool rayPlaneIntersection(const Ray& ray, const Plane& plane, vec3& outContactPoint)
{
	float denom = glm::dot(plane.normal(), ray.direction());
	if (fabs(denom) > 0.0001f)
	{
		float length = glm::dot(plane.origin() - ray.origin(), plane.normal()) / denom;
		if (length >= 0)
		{
			outContactPoint = ray.origin() + (ray.direction() * length);
			return true;
		}
	}
	return false;
}

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

void LineGizmo::setGizmoMaterialId(Id id)
{
	m_materialId = id;
}

bool LineGizmo::hover(const Ray& mouseRay, const Camera& camera)
{
	const float MinHoverDistance = 0.0f;
	const float MaxHoverDistance = 900000.0f;

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);
	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier * m_hoverMarginMultiplier;

	// Sort line handles
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		m_sortedLineHandles[i].axisIndex = i;
		m_sortedLineHandles[i].tipPosition = m_position + (axisVector(Axis(i)) * gizmoCameraFactor);
		m_sortedLineHandles[i].distanceFromCamera =
			glm::length(m_sortedLineHandles[i].tipPosition - camera.position());
	}

	// Sort from closest to farthest
	std::sort(m_sortedLineHandles.begin(), m_sortedLineHandles.end(),
		[](const LineHandle& a, const LineHandle& b)
	{
		return a.distanceFromCamera < b.distanceFromCamera;
	});

	// Clear hovers
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		m_axisHovers[i] = false;
	}

	for (auto&& lineHandle : m_sortedLineHandles)
	{
		int i = lineHandle.axisIndex;

		Transform transform = m_axisTransforms[i];
		transform.scale = transform.scale *
			gizmoCameraFactor * vec3(1.0f, m_hoverThicknessMultiplier, m_hoverThicknessMultiplier);
		// The handle's box needs to be centered with that 0.5
		transform.position = m_position + (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor);

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

void LineGizmo::render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem)
{
	auto& material = assetSystem.getMaterial(m_materialId);

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);
	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier;

	const auto hoverColor = Utils::createColor8bit(255, 165, 0);
	const auto activeColor = Utils::createColor8bit(255, 215, 0);

	// Draw from farthest to closest, so iterate backwards
	for (int index = m_sortedLineHandles.size()-1; index >= 0; --index)
	{
		int i = m_sortedLineHandles[index].axisIndex;

		Color color = m_axisActives[i] ? activeColor : (m_axisHovers[i] ? hoverColor : axisColor(Axis(i)));

		// Draw line as a box
		Transform transform = m_axisTransforms[i];
		transform.scale = transform.scale * gizmoCameraFactor;
		// The handle's box needs to be centered with that 0.5
		transform.position = m_position + (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor);
		renderSystem.renderMeshSingleColor(
			camera,
			transform,
			color,
			material,
			m_lineMesh);

		// Draw cone
		Transform coneTransform = m_axisTransforms[i];
		float coneSize = m_gizmoSizeMultiplier;
		coneTransform.scale = vec3(coneSize, coneSize * m_coneLengthMultiplier, coneSize) * gizmoCameraFactor;
		coneTransform.rotation = coneTransform.rotation *
			glm::angleAxis(-Math::TAU * 0.25f, vec3(0.0f, 0.0f, 1.0f));
		coneTransform.position = m_position + (axisVector(Axis(i)) * gizmoCameraFactor);
		renderSystem.renderMeshSingleColor(
			camera,
			coneTransform,
			color,
			material,
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

//-----------------------------------------------------------

bool TransformTool::hover(const Ray& mouseRay, const Camera& camera)
{
	if (m_translateGizmo.isVisible())
	{
		return m_translateGizmo.hover(mouseRay, camera);
	}
	return false;
}

HandleStatus TransformTool::handleInput(Input& input, const Camera& camera, SelectionSystem& selectionSystem)
{
	// RAE_TODO REMOVE:
	//g_debugSystem->drawLine(m_translateGizmo.m_debugIntersectionLine);
	//g_debugSystem->drawLine(m_translateGizmo.m_debugLine);

	if (not m_translateGizmo.isVisible())
		return HandleStatus::NotHandled;

	m_translateGizmo.setPosition(selectionSystem.selectionPosition());

	m_previousMouseRay = m_mouseRay;
	m_mouseRay = camera.getExactRay(input.mouse.normalizedWindowX(), input.mouse.normalizedWindowY());

	bool isActive = m_translateGizmo.isActive();
	if (isActive)
	{
		if (input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonRelease)
		{
			m_translateGizmo.deactivate();
		}
		else if (input.mouse.button(MouseButton::First))
		{
			vec3 delta = m_translateGizmo.activeAxisDelta(camera, m_mouseRay, m_previousMouseRay);

			translateSelected(delta, selectionSystem);
			m_translateGizmo.setPosition(m_translateGizmo.position() + delta);
		}
		return HandleStatus::Handled;
	}
	else
	{
		bool isHover = hover(m_mouseRay, camera);
		if (isHover)
		{
			if (input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
			{
				m_translateGizmo.activateHovered();
			}
			return HandleStatus::Handled;
		}
	}

	return HandleStatus::NotHandled;
}

void TransformTool::render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem)
{
	if (m_translateGizmo.isVisible())
	{
		// Don't depth test when rendering the gizmos
		glDepthFunc(GL_ALWAYS);
		// Alternative way would be to clear the depth buffer,
		// but that resulted in a slowdown and jerky rendering
		//glClear(GL_DEPTH_BUFFER_BIT);

		m_translateGizmo.render3D(camera, renderSystem, assetSystem);

		glDepthFunc(GL_LEQUAL);
	}
}

void TransformTool::setGizmoMaterialId(Id id)
{
	m_translateGizmo.setGizmoMaterialId(id);
}

void TransformTool::onSelectionChanged(SelectionSystem& selectionSystem)
{
	if (selectionSystem.isSelection())
	{
		m_translateGizmo.show();
		m_translateGizmo.setPosition(selectionSystem.selectionPosition());
	}
	else
	{
		m_translateGizmo.hide();
	}
}

void TransformTool::translateSelected(const vec3& delta, SelectionSystem& selectionSystem)
{
	//RAE_TODO vec3 axisVector = m_translateGizmo.getActiveAxisVector();
	//RAE_TODO selectionSystem.translateSelected(axisVector * delta * 0.01f);

	selectionSystem.translateSelected(delta);
}

EditorSystem::EditorSystem(
	SceneSystem& sceneSystem,
	RenderSystem& renderSystem,
	AssetSystem& assetSystem,
	Input& input) :
		m_sceneSystem(sceneSystem),
		m_renderSystem(renderSystem),
		m_assetSystem(assetSystem),
		m_input(input)
{
	LOG_F(INFO, "Init %s", name().c_str());

	Id gizmoMaterialID = m_assetSystem.createMaterial(Color(1.0f, 1.0f, 1.0f, 1.0f));
	m_transformTool.setGizmoMaterialId(gizmoMaterialID);

	Scene& scene = m_sceneSystem.activeScene();
	auto& selectionSystem = scene.selectionSystem();

	using std::placeholders::_1;
	selectionSystem.onSelectionChanged.connect(std::bind(
		&TransformTool::onSelectionChanged, &m_transformTool, _1));
}

UpdateStatus EditorSystem::update()
{
	if (!m_sceneSystem.hasActiveScene())
		return UpdateStatus::NotChanged;

	Scene& scene = m_sceneSystem.activeScene();
	auto& selectionSystem = scene.selectionSystem();

	HandleStatus transformToolStatus = m_transformTool.handleInput(m_input,
																   scene.cameraSystem().currentCamera(),
																   selectionSystem);

	if (transformToolStatus == HandleStatus::NotHandled)
	{
		if (m_input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
		{
			Id hoveredId = selectionSystem.hovered();
			// RAE_TODO: Needs to have a higher level function
			// in Input where we can ask for modifier states for Control.
			if (m_input.getKeyState(KeySym::Control_L) ||
				m_input.getKeyState(KeySym::Control_R) ||
				m_input.getKeyState(KeySym::Super_L) ||
				m_input.getKeyState(KeySym::Super_R))
			{
				if (hoveredId != InvalidId)
					selectionSystem.toggleSelected(hoveredId);
			}
			else if (hoveredId == InvalidId)
			{
				selectionSystem.clearSelection();
			}
			else
			{
				selectionSystem.setSelection({ hoveredId });
			}
		}
	}

	return UpdateStatus::NotChanged; // for now.
}

void EditorSystem::render3D(const Scene& scene)
{
	const Camera& camera = scene.cameraSystem().currentCamera();
	m_transformTool.render3D(camera, m_renderSystem, m_assetSystem);
}
