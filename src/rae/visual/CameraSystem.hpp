#pragma once

#include <vector>
#include <functional>

#include "rae/visual/Camera.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

#include "rae/entity/EntitySystem.hpp"
#include "rae/visual/TransformSystem.hpp"

namespace rae
{

class Input;

class CameraSystem : public ISystem
{
public:
	CameraSystem(EntitySystem& entitySystem, TransformSystem& transformSystem, Input& input);

	String name() override { return "CameraSystem"; }

	bool update(double time, double delta_time) override;

	void onMouseEvent(const Input& input);
	void onKeyEvent(const Input& input);

	void setNeedsUpdate() { getCurrentCamera().setNeedsUpdate(); }
	void setAspectRatio(float aspect) { getCurrentCamera().setAspectRatio(aspect); }

	const Camera& getCurrentCamera() const { return getCamera(m_currentCamera); }
	Camera& getCurrentCamera() { return getCamera(m_currentCamera); }

	Id createCamera();
	void addCamera(Id id, Camera&& comp);
	const Camera& getCamera(Id id) const;
	Camera& getCamera(Id id);

	void connectCameraChangedEventHandler(std::function<void(const Camera&)> handler);

private:
	EntitySystem& m_entitySystem;
	TransformSystem& m_transformSystem;
	Input& m_input;

	Id m_currentCamera;
	Table<Camera> m_cameras;

	void emitCameraChangedEvent();
	std::vector<std::function<void(const Camera&)>> cameraChangedEvent;
};

}
