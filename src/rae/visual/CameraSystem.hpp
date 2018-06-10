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

class CameraSystem : public ISystem
{
public:
	CameraSystem(const Time& time, EntitySystem& entitySystem, TransformSystem& transformSystem, Input& input);

	UpdateStatus update() override;

	void onMouseEvent(const Input& input);
	void onKeyEvent(const Input& input);

	void setNeedsUpdate() { currentCamera().setNeedsUpdate(); }
	void setAspectRatio(float aspect) { currentCamera().setAspectRatio(aspect); }

	const Camera& currentCamera() const { return getCamera(m_currentCamera); }
	Camera& currentCamera() { return getCamera(m_currentCamera); }

	Id createCamera();
	void addCamera(Id id, Camera&& comp);
	const Camera& getCamera(Id id) const;
	Camera& getCamera(Id id);

	void connectCameraUpdatedEventHandler(std::function<void(const Camera&)> handler);

private:
	const Time& m_time;
	EntitySystem& m_entitySystem;
	TransformSystem& m_transformSystem;
	Input& m_input;

	Id m_currentCamera;
	Table<Camera> m_cameras;

	void emitCameraUpdatedEvent();
	std::vector<std::function<void(const Camera&)>> cameraUpdatedEvent;
};

}
