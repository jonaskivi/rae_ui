#pragma once

#include <vector>
#include <functional>

#include "rae/visual/Camera.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

#include "rae/entity/EntitySystem.hpp"
#include "rae/scene/TransformSystem.hpp"

namespace rae
{

class Time;
class Input;
struct InputState;

class CameraSystem : public ISystem
{
public:
	CameraSystem(const Time& time, EntitySystem& entitySystem, TransformSystem& transformSystem, Input& input);

	UpdateStatus update() override;

	void onFrameEnd() override;

	void onMouseEvent(const InputState& inputState);
	void onKeyEvent(const Input& input);

	void setNeedsUpdate() { modifyCurrentCamera().setNeedsUpdate(); }
	void setAspectRatio(float aspect) { modifyCurrentCamera().setAspectRatio(aspect); }

	const Camera& currentCamera() const { return getCamera(m_currentCamera); }
	Camera& modifyCurrentCamera() { return modifyCamera(m_currentCamera); }

	Id createCamera();
	void addCamera(Id id, const Camera& camera);
	const Camera& getCamera(Id id) const;
	Camera& modifyCamera(Id id);

	void connectCameraUpdatedEventHandler(std::function<void(const Camera&)> handler);

	bool hasCameraUpdated() const { return m_cameraUpdated; }

private:
	const Time& m_time;
	EntitySystem& m_entitySystem;
	TransformSystem& m_transformSystem;
	Input& m_input;

	Id m_currentCamera;
	Table<Camera> m_cameras;

	void emitCameraUpdatedEvent();
	std::vector<std::function<void(const Camera&)>> cameraUpdatedEvent;
	bool m_cameraUpdated = false;
};

}
