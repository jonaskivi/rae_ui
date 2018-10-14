#pragma once

#include <array>

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"

#include "rae/ui/Input.hpp"
#include "rae/ui/DebugSystem.hpp"

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

enum class Axis
{
	X,
	Y,
	Z,
	Count
};

enum class HandleStatus
{
	NotHandled,
	Handled
};

class IGizmo
{
public:
	const vec3& position() const { return m_position; }
	void setPosition(const vec3& position) { m_position = position; }

	bool isVisible() const { return m_visible; }
	void show() { m_visible = true; }
	void hide() { m_visible = false; }

	bool isHovered() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisHovers[i])
				return true;
		}
		return false;
	}

	bool isActive() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisActives[i])
				return true;
		}
		return false;
	}

	Axis getActiveAxis() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisActives[i])
				return (Axis)i;
		}
		assert(0);
		return Axis::X;
	}

	void activateHovered()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_axisActives[i] = m_axisHovers[i];
		}
	}

	void deactivate()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_axisActives[i] = false;
		}
	}

protected:
	bool m_visible = false;

	vec3 m_position;

	std::array<Transform,	(int)Axis::Count>	m_axisTransforms;
	std::array<bool,		(int)Axis::Count>	m_axisHovers = { false, false, false };
	std::array<bool,		(int)Axis::Count>	m_axisActives = { false, false, false };
};

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

	float m_gizmoSizeMultiplier = 0.1f;
	float m_hoverMarginMultiplier = 1.4f;
	float m_hoverThicknessMultiplier = 3.0f;
	float m_coneLengthMultiplier = 4.0f;
	Mesh m_lineMesh;
	Mesh m_coneMesh;
};

class TranslateGizmo : public LineGizmo
{
public:
	TranslateGizmo(){}
};

class TransformTool
{
public:
	HandleStatus handleInput(
		Input& input,
		const InputState& inputState,
		const Camera& camera,
		SelectionSystem& selectionSystem);
	bool hover(const Ray& mouseRay, const Camera& camera);
	void render3D(const Camera& camera, RenderSystem& renderSystem) const;

	void onSelectionChanged(SelectionSystem& selectionSystem);

	void translateSelected(const vec3& delta, SelectionSystem& selectionSystem);

protected:
	TranslateGizmo m_translateGizmo;

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
	void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) const;
	//void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) override;
	void handleInput(const InputState& inputState, const Array<InputEvent>& events, Scene& scene);
	void hover(const InputState& inputState, Scene& scene);

protected:
	Input&				m_input;
	InputState			m_inputState; // Storing this might be stupid. We only do it for update.

	TransformTool		m_transformTool;
};

}
