#include "rae/editor/EditorSystem.hpp"

#include <cmath>

#include "rae/core/Log.hpp"
#include "rae/core/Utils.hpp"
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
			return qua(vec3(0.0f, Math::toRadians(90.0f), 0.0f));
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

	if (fabs(perpendicular1) <= fabs(perpendicular2))
	{
		return plane1;
	}
	else
	{
		return plane2;
	}
}


bool rayPlaneIntersection(const Ray& ray, const Plane& plane, vec3& outContactPoint)
{
	float denom = glm::dot(plane.normal(), ray.direction());
	if (abs(denom) > 0.0001f)
	{
		float t = glm::dot(plane.origin() - ray.origin(), plane.normal()) / denom;
		if (t >= 0)
		{
			outContactPoint = ray.origin() + glm::normalize(ray.direction()) * t;
			return true;
		}
	}
	return false;
}


/*
bool rayPlaneIntersection(const Ray& ray, const Plane& plane, vec3& outContactPoint)
{
	// get d value
	float d = glm::dot(plane.normal(), plane.origin());

	if (glm::dot(plane.normal(), ray.direction()) == 0)
	{
		return false; // No intersection, the line is parallel to the plane
	}

	// Compute the X value for the directed line ray intersecting the plane
	float x = (d - glm::dot(plane.normal(), ray.origin())) / glm::dot(plane.normal(), ray.direction());

	outContactPoint = ray.origin() + glm::normalize(ray.direction()) * x;
	return true;
}
*/

TranslateGizmo::TranslateGizmo()
{
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		m_axisTransforms[i].position = vec3(0.0f, 0.0f, 0.0f);
		m_axisTransforms[i].rotation = axisRotation(Axis(i));
		m_axisTransforms[i].scale = vec3(1.0f, AxisThickness, AxisThickness);
	}

	m_lineMesh.generateBox();
	m_lineMesh.createVBOs();
}

void TranslateGizmo::setGizmoMaterialId(Id id)
{
	m_materialId = id;
}

bool TranslateGizmo::hover(const Ray& mouseRay, const Camera& camera)
{
	const float MinHoverDistance = 0.0f;
	const float MaxHoverDistance = 8000.0f;

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);
	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier;

	// Clear hovers
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		m_axisHovers[i] = false;
	}

	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		Transform transform = m_axisTransforms[i];
		transform.scale = transform.scale * gizmoCameraFactor;
		transform.position = m_position + (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor);

		Box axisBox = m_lineMesh.getAabb();
		axisBox.transform(transform);
		bool isHit = axisBox.hit(mouseRay, MinHoverDistance, MaxHoverDistance);
		m_axisHovers[i] = isHit;

		// RAE_TODO: Now we just return on first hit, but we would actually want to sort them by distance.
		if (isHit)
		{
			return true;
		}
	}
	return false;
}

void TranslateGizmo::render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem)
{
	auto& material = assetSystem.getMaterial(m_materialId);

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);
	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier;

	const auto hoverColor = Utils::createColor8bit(255, 165, 0);
	const auto activeColor = Utils::createColor8bit(255, 215, 0);

	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		Transform transform = m_axisTransforms[i];

		Color color = m_axisActives[i] ? activeColor : (m_axisHovers[i] ? hoverColor : axisColor(Axis(i)));

		transform.scale = transform.scale * gizmoCameraFactor;
		transform.position = m_position + (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor);
		renderSystem.renderMesh(camera,
			transform,
			color,
			material, m_lineMesh, false);
	}
}

vec3 TranslateGizmo::getActiveAxisVector() const
{
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		if (m_axisActives[i])
			return axisVector((Axis)i);
	}
	return vec3();
}

vec3 TranslateGizmo::activeAxisDelta(const Ray& mouseRay, const Ray& previousMouseRay) const
{
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		if (not m_axisActives[i])
			continue;

		Axis axis = (Axis)i;
		Plane bestPlane = computeMostPerpendicularAxisPlane(axis, m_position,
			glm::normalize(m_position - mouseRay.origin()));

		vec3 intersection;
		bool hit = rayPlaneIntersection(mouseRay, bestPlane, intersection);
		if (hit)
		{
			vec3 previousIntersection;
			bool previousHit = rayPlaneIntersection(previousMouseRay, bestPlane, previousIntersection);
			if (previousHit)
			{
				vec3 deltaVec = intersection - previousIntersection;

				g_debugSystem->showDebugText("length: " + Utils::toString(glm::length(deltaVec)), Colors::magenta);
				g_debugSystem->drawLine({ intersection, previousIntersection }, Colors::cyan);

				float dotProduct = glm::dot(deltaVec, axisVector(axis));

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
			//translateSelected((m_previousMouseRay.origin() - m_mouseRay.origin()).length(), selectionSystem);

			vec3 delta = m_translateGizmo.activeAxisDelta(m_mouseRay, m_previousMouseRay);

			translateSelected(delta, selectionSystem);
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
		m_translateGizmo.render3D(camera, renderSystem, assetSystem);
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
		//m_translateGizmo.setPosition(selectionSystem.selectionPosition());
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

EditorSystem::EditorSystem(CameraSystem& cameraSystem, RenderSystem& renderSystem, AssetSystem& assetSystem,
	SelectionSystem& selectionSystem, Input& input, UISystem& uiSystem) :
		m_cameraSystem(cameraSystem),
		m_renderSystem(renderSystem),
		m_assetSystem(assetSystem),
		m_selectionSystem(selectionSystem),
		m_input(input),
		m_uiSystem(uiSystem)
{
	Id gizmoMaterialID = m_assetSystem.createMaterial(Color(1.0f, 1.0f, 1.0f, 1.0f));
	m_transformTool.setGizmoMaterialId(gizmoMaterialID);

	using std::placeholders::_1;
	m_selectionSystem.onSelectionChanged.connect(std::bind(&TransformTool::onSelectionChanged, &m_transformTool, _1));
}

UpdateStatus EditorSystem::update()
{
	HandleStatus transformToolStatus = m_transformTool.handleInput(m_input,
																   m_cameraSystem.getCurrentCamera(),
																   m_selectionSystem);

	if (transformToolStatus == HandleStatus::NotHandled)
	{
		if (m_input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
		{
			Id hoveredId = m_selectionSystem.hovered();
			// RAE_TODO: Needs to have a higher level function in Input where we can ask for modifier states for Control.
			if (m_input.getKeyState(KeySym::Control_L) ||
				m_input.getKeyState(KeySym::Control_R) ||
				m_input.getKeyState(KeySym::Super_L) ||
				m_input.getKeyState(KeySym::Super_R))
			{
				if (hoveredId != InvalidId)
					m_selectionSystem.toggleSelected(hoveredId);
			}
			else if (hoveredId == InvalidId)
			{
				m_selectionSystem.clearSelection();
			}
			else
			{
				m_selectionSystem.setSelection({ hoveredId });
			}
		}
	}

	return UpdateStatus::NotChanged; // for now.
}

void EditorSystem::render3D()
{
	m_transformTool.render3D(m_cameraSystem.getCurrentCamera(), m_renderSystem, m_assetSystem);
}
