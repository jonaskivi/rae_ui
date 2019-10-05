#include "rae/editor/EditorSystem.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "rae/core/Utils.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/visual/Plane.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/ui/UISystem.hpp"

using namespace rae;

TransformTool::TransformTool()
{
	m_gizmos.emplace_back(&m_translateGizmo);
	m_gizmos.emplace_back(&m_rotateGizmo);
	m_gizmos.emplace_back(&m_scaleGizmo);
}

bool TransformTool::hover(const Ray& mouseRay, const Camera& camera)
{
	LOG_F(INFO, "TransformTool hover.");

	if (m_translateGizmo.isVisible())
	{
		return m_translateGizmo.hover(mouseRay, camera);
	}

	if (m_rotateGizmo.isVisible())
	{
		return m_rotateGizmo.hover(mouseRay, camera);
	}

	return false;
}

HandleStatus TransformTool::handleInput(
	Input& input,
	const InputState& inputState,
	const Camera& camera,
	SelectionSystem& selectionSystem)
{
	// RAE_TODO REMOVE:
	//g_debugSystem->drawLine(m_translateGizmo.m_debugIntersectionLine);
	//g_debugSystem->drawLine(m_translateGizmo.m_debugLine);

	if (!anyGizmoVisible())
		return HandleStatus::NotHandled;

	m_translateGizmo.setPosition(selectionSystem.selectionWorldPosition());
	m_rotateGizmo.setPosition(selectionSystem.selectionWorldPosition());

	m_previousMouseRay = m_mouseRay;
	m_mouseRay = camera.getExactRay(
		inputState.mouse.localPositionNormalized.x,
		inputState.mouse.localPositionNormalized.y);

	bool isActive = false;

	if (m_transformToolMode == TransformToolMode::Translate)
	{
		isActive = m_translateGizmo.isActive();
	}
	else if (m_transformToolMode == TransformToolMode::Rotate)
	{
		isActive = m_rotateGizmo.isActive();
	}
	else if (m_transformToolMode == TransformToolMode::Scale)
	{
		isActive = m_scaleGizmo.isActive();
	}

	if (isActive)
	{
		if (input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonRelease)
		{
			m_translateGizmo.deactivate();
			m_rotateGizmo.deactivate();
			m_rotateGizmo.clearAccumulatedRotation();
			m_scaleGizmo.deactivate();
		}
		else if (input.mouse.isButtonDown(MouseButton::First))
		{
			if (m_transformToolMode == TransformToolMode::Translate)
			{
				vec3 delta = m_translateGizmo.activeAxisDelta(camera, m_mouseRay, m_previousMouseRay);

				translateSelected(delta, selectionSystem);
				// RAE_TODO: This is a bit hacky. We should instead read the position from the selection.
				m_translateGizmo.setPosition(m_translateGizmo.position() + delta);
			}
			else if (m_transformToolMode == TransformToolMode::Rotate)
			{
				bool snapEnabled = input.getKeyState(KeySym::Shift_L) || input.getKeyState(KeySym::Shift_R);
				float snapAngleStep = Math::toRadians(45.0f);
				bool precisionModifier = false;

				qua rotateDelta = m_rotateGizmo.getRotateAxisDelta(
					glm::vec2(inputState.mouse.delta),
					camera,
					vec3(), // RAE_TODO quicktransform mode?
					snapEnabled,
					snapAngleStep,
					precisionModifier);

				rotateSelected(rotateDelta, selectionSystem);
				// RAE_TODO: This is a bit hacky. We should instead read the rotation from the selection.
				// RAE_TODO something like this for local transform mode: m_rotateGizmo.addToRotation(rotateDelta);
			}
			else if (m_transformToolMode == TransformToolMode::Scale)
			{
				// RAE_TODO scaling.
			}
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
				m_rotateGizmo.activateHovered();
				m_scaleGizmo.activateHovered();
			}
			return HandleStatus::Handled;
		}
	}

	return HandleStatus::NotHandled;
}

void TransformTool::update(Scene& scene)
{
	const Camera& camera = scene.cameraSystem().currentCamera();
	auto& shapeRenderer = scene.modifyShapeRenderer();

	if (m_rotateGizmo.isVisible())
	{
		m_rotateGizmo.render3D(camera, shapeRenderer);
	}
}

void TransformTool::render3D(
	const Camera& camera,
	RenderSystem& renderSystem) const
{
	if (m_translateGizmo.isVisible())
	{
		// Don't depth test when rendering the gizmos
		glDepthFunc(GL_ALWAYS);
		// Alternative way would be to clear the depth buffer,
		// but that resulted in a slowdown and jerky rendering
		//glClear(GL_DEPTH_BUFFER_BIT);

		m_translateGizmo.render3D(camera, renderSystem);

		glDepthFunc(GL_LEQUAL);
	}

	/*if (m_rotateGizmo.isVisible())
	{
		// Don't depth test when rendering the gizmos
		glDepthFunc(GL_ALWAYS);
		m_rotateGizmo.render3D(camera, shapeRenderer, renderSystem);
		glDepthFunc(GL_LEQUAL);
	}

	if (m_scaleGizmo.isVisible())
	{
		// Don't depth test when rendering the gizmos
		glDepthFunc(GL_ALWAYS);
		m_scaleGizmo.render3D(camera, renderSystem);
		glDepthFunc(GL_LEQUAL);
	}
	*/
}

void TransformTool::setTransformToolMode(TransformToolMode mode)
{
	if (mode == m_transformToolMode)
		return;

	m_transformToolMode = mode;

	if (anyGizmoVisible())
	{
		hideAllGizmos();
	}

	showCorrectToolMode();
}

void TransformTool::showCorrectToolMode()
{
	if (m_transformToolMode == TransformToolMode::Translate)
	{
		m_translateGizmo.show();
	}
	else if (m_transformToolMode == TransformToolMode::Rotate)
	{
		m_rotateGizmo.show();
	}
	else if (m_transformToolMode == TransformToolMode::Scale)
	{
		m_scaleGizmo.show();
	}
}

void TransformTool::onSelectionChanged(SelectionSystem& selectionSystem)
{
	if (selectionSystem.isSelection())
	{
		m_translateGizmo.setPosition(selectionSystem.selectionWorldPosition());
		m_translateGizmo.setRotation(qua());
		//m_translateGizmo.setRotation(selectionSystem.selectionWorldRotation());
		m_rotateGizmo.setPosition(selectionSystem.selectionWorldPosition());
		m_rotateGizmo.setRotation(qua());
		//m_rotateGizmo.setRotation(selectionSystem.selectionWorldRotation());

		showCorrectToolMode();
	}
	else
	{
		hideAllGizmos();
	}
}

void TransformTool::translateSelected(const vec3& delta, SelectionSystem& selectionSystem)
{
	//RAE_TODO vec3 axisVector = m_translateGizmo.getActiveAxisVector();
	//RAE_TODO selectionSystem.translateSelected(axisVector * delta * 0.01f);

	selectionSystem.translateSelected(delta);
}

void TransformTool::rotateSelected(const qua& delta, SelectionSystem& selectionSystem)
{
	selectionSystem.rotateSelected(delta, m_rotateGizmo.position());
}

EditorSystem::EditorSystem(
	SelectionSystem& selectionSystem,
	Input& input) :
		ISystem("EditorSystem"),
		m_input(input)
{
	//LOG_F(INFO, "Init %s", name().c_str());

	using std::placeholders::_1;
	selectionSystem.onSelectionChanged.connect(std::bind(
		&TransformTool::onSelectionChanged, &m_transformTool, _1));
}

UpdateStatus EditorSystem::update(Scene& scene)
{
	const auto hoverColor = Utils::createColor8bit(255, 165, 0);
	const auto selectedColor = Utils::createColor8bit(0, 255, 165);

	// It is actually strange that this selection rendering is in update. But we'll have to fix this later, probably
	// when debugSystem becomes aware of the scenes and viewports.
	// Also we need a line rendering system that is independent of debugSystem,
	// because now these line boxes are off by default.
	auto& selectionSystem = scene.selectionSystem();
	auto& shapeRenderer = scene.modifyShapeRenderer();

	if (selectionSystem.isSelection())
	{
		Box selectionAabb = selectionSystem.selectionAABB();
		shapeRenderer.drawLineBox(selectionAabb, selectedColor);
	}

	if (selectionSystem.isAnyHovered())
	{
		Box selectionAabb = selectionSystem.hoveredAABB();
		shapeRenderer.drawLineBox(selectionAabb, hoverColor);
	}

	m_transformTool.update(scene);

	/* RAE_TODO THIS USED TO DO SOMETHING, BEFORE THE HANDLEINPUT InputState STUFF.

	HandleStatus transformToolStatus =
		m_transformTool.handleInput(
			m_input,
			m_inputState,
			scene.cameraSystem().currentCamera(),
			selectionSystem);

	if (transformToolStatus == HandleStatus::NotHandled)
	{
		if (m_input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
		{
			Id hoveredId = selectionSystem.pixelHovered();
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
	*/

	return UpdateStatus::NotChanged; // for now.
}

void EditorSystem::render3D(
	const Scene& scene,
	const Window& window,
	RenderSystem& renderSystem) const
{
	const Camera& camera = scene.cameraSystem().currentCamera();
	m_transformTool.render3D(camera, renderSystem);
}

void EditorSystem::handleInput(const InputState& inputState, const Array<InputEvent>& events, Scene& scene)
{
	m_inputState = inputState;

	bool hadEvents = !events.empty();

	if (!hadEvents)
		return;

	scene.modifySelectionSystem().clearHovers();

	if (inputState.mouse.buttonClicked[(int)MouseButton::First] == true)
	{
		//LOG_F(INFO, "EditorSystem handleInput 1st button clicked. x:%f y:%f",
		//	inputState.mouse.localPositionNormalized.x,
		//	inputState.mouse.localPositionNormalized.y);
	}

	// Don't handle transform tool when second mouse button is down. It handles camera movement.
	bool cameraInput = inputState.mouse.isButtonDown(MouseButton::Second);

	HandleStatus transformToolStatus = HandleStatus::NotHandled;

	if (cameraInput == false)
	{
		transformToolStatus =
			m_transformTool.handleInput(m_input,
				inputState,
				scene.cameraSystem().currentCamera(),
				scene.modifySelectionSystem());
	}

	if (cameraInput == false && transformToolStatus == HandleStatus::NotHandled)
	{
		hover(inputState, scene);

		Id hoveredId = scene.selectionSystem().hovered();

		if (hoveredId != InvalidId)
		{
			//LOG_F(INFO, "EditorSystem handleInput Something was hovered. OMG.");
		}

		if (inputState.mouse.buttonClicked[(int)MouseButton::First])
		{
			// RAE_TODO: Needs to have a higher level function
			// in Input where we can ask for modifier states for Control.
			if (m_input.getKeyState(KeySym::Control_L) ||
				m_input.getKeyState(KeySym::Control_R) ||
				m_input.getKeyState(KeySym::Super_L) ||
				m_input.getKeyState(KeySym::Super_R))
			{
				if (hoveredId != InvalidId)
					scene.modifySelectionSystem().toggleSelected(hoveredId);
			}
			else if (hoveredId == InvalidId)
			{
				scene.modifySelectionSystem().clearSelection();
			}
			else
			{
				scene.modifySelectionSystem().setSelection({ hoveredId });
			}
		}
	}

	if (transformToolStatus == HandleStatus::NotHandled)
	{
		scene.modifyCameraSystem().onMouseEvent(inputState);
	}
}

void EditorSystem::hover(const InputState& inputState, Scene& scene)
{
	const float MinHoverDistance = 0.0f;
	const float MaxHoverDistance = 900000.0f;

	Ray mouseRay = scene.cameraSystem()
		.currentCamera()
		.getExactRay(
			inputState.mouse.localPositionNormalized.x,
			inputState.mouse.localPositionNormalized.y);

	Id topMostId = InvalidId;

	float closestDistance = FLT_MAX;

	query<Box>(scene.transformSystem().boxes(), [&](Id id, const Box& box)
	{
		if (scene.selectionSystem().isDisableHovering(id))
		{
			// Continue and skip the hit test.
		}
		else if (scene.transformSystem().hasWorldTransform(id))
		{
			const Transform& transform = scene.transformSystem().getWorldTransform(id);
			const Pivot& pivot = scene.transformSystem().getPivot(id);
			Box tbox = box;
			tbox.transform(transform);
			tbox.translatePivot(pivot);

			//if (tbox.hit(vec2(m_input.mouse.xMM, m_input.mouse.yMM)))
			if (tbox.hit(mouseRay, MinHoverDistance, MaxHoverDistance))
			{
				//LOG_F(INFO, "hit box id: %i", (int)id);

				float distance =
					glm::length(transform.position - mouseRay.origin());

				if (distance < closestDistance)
				{
					closestDistance = distance;
					topMostId = id;
				}
			}
		}
	});

	if (topMostId != InvalidId)
	{
		//LOG_F(INFO, "Hovered: id %i", (int)topMostId);
		scene.modifySelectionSystem().setHoveredHierarchy(topMostId, true);
	}
	else
	{
		//LOG_F(INFO, "NO HOVER. x%f y%f",
		//	inputState.mouse.localPositionNormalized.x,
		//	inputState.mouse.localPositionNormalized.y);
	}
}

void EditorSystem::setSelectionToolMode()
{
	// RAE_TODO.
}

void EditorSystem::setTranslateToolMode()
{
	m_transformTool.setTransformToolMode(TransformToolMode::Translate);
}

void EditorSystem::setRotateToolMode()
{
	m_transformTool.setTransformToolMode(TransformToolMode::Rotate);
}

void EditorSystem::setScaleToolMode()
{
	m_transformTool.setTransformToolMode(TransformToolMode::Scale);
}
