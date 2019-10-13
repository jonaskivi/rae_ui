#pragma once

#include <array>

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"

#include "rae/ui/Input.hpp"
#include "rae/ui/DebugSystem.hpp"

#include "rae/editor/LineGizmo.hpp"
#include "rae/editor/RotateGizmo.hpp"

namespace rae
{

class Camera;
class CameraSystem;
class scene;
class RenderSystem;
class SelectionSystem;
class Input;
struct InputEvent;
struct InputState;
class UISystem;

enum class HandleStatus
{
	NotHandled,
	Handled
};

enum class TransformToolMode
{
	Translate,
	Rotate,
	Scale
};

class TranslateGizmo : public LineGizmo
{
public:
	TranslateGizmo(){}
};

class TransformTool
{
public:
	TransformTool();

	HandleStatus handleInput(
		Input& input,
		const InputState& inputState,
		const Camera& camera,
		SelectionSystem& selectionSystem);
	bool hover(const Ray& mouseRay, const Camera& camera);
	// Update currently uses the ShapeRenderer to render RotateGizmo.
	void update(Scene& scene);
	void render3D(
		const Camera& camera,
		RenderSystem& renderSystem) const;

	void onSelectionChanged(const SelectionSystem& selectionSystem);

	bool anyGizmoVisible()
	{
		for (auto* gizmo : m_gizmos)
		{
			if (gizmo->isVisible())
				return true;
		}
		return false;
	}

	void hideAllGizmos()
	{
		for (auto* gizmo : m_gizmos)
		{
			gizmo->hide();
		}
	}

	void translateSelected(const vec3& delta, SelectionSystem& selectionSystem);
	void rotateSelected(const qua& delta, SelectionSystem& selectionSystem);

	void setTransformToolMode(TransformToolMode mode);

	void nextGizmoPivot(const SelectionSystem& selectionSystem)
	{
		int temp = int(m_gizmoPivot);
		temp++;
		if (temp == int(GizmoPivot::Count))
			temp = 0;
		m_gizmoPivot = GizmoPivot(temp);

		updateGizmoPosition(selectionSystem);
	}

	void nextGizmoAxis(const SelectionSystem& selectionSystem)
	{
		int temp = int(m_gizmoAxis);
		temp++;
		if (temp == int(GizmoAxis::Count))
			temp = 0;
		m_gizmoAxis = GizmoAxis(temp);

		updateGizmoPosition(selectionSystem);
	}

protected:

	void updateGizmoPosition(const SelectionSystem& selectionSystem);
	void updateGizmoRotation(const SelectionSystem& selectionSystem);
	void showCorrectToolMode();

	TransformToolMode m_transformToolMode = TransformToolMode::Rotate;
	GizmoPivot m_gizmoPivot = GizmoPivot::Auto;
	GizmoAxis m_gizmoAxis = GizmoAxis::World;

	TranslateGizmo m_translateGizmo;
	RotateGizmo m_rotateGizmo;
	LineGizmo m_scaleGizmo;

	Array<IGizmo*> m_gizmos;

	Ray m_mouseRay;
	Ray m_previousMouseRay;
};

class EditorSystem : public ISystem
{
public:
	EditorSystem(
		SelectionSystem& selectionSystem,
		Input& input);

	UpdateStatus update(Scene& scene);
	void render3D(
		const Scene& scene,
		const Window& window,
		RenderSystem& renderSystem) const;
	// NOT OVERRIDDEN because of const:
	//void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) override;
	void handleInput(const InputState& inputState, const Array<InputEvent>& events, Scene& scene);
	void hover(const InputState& inputState, Scene& scene);

	const TransformTool& getTransformTool() const { return m_transformTool; }
	TransformTool& modifyTransformTool() { return m_transformTool; }

	void setSelectionToolMode();
	void setTranslateToolMode();
	void setRotateToolMode();
	void setScaleToolMode();

protected:
	Input&				m_input;
	InputState			m_inputState; // Storing this might be stupid. We only do it for update.

	TransformTool		m_transformTool;
};

}
