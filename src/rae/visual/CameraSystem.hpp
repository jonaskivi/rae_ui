#pragma once

#include <vector>
#include <functional>

#include "Camera.hpp"
#include "rae/core/ISystem.hpp"

namespace rae
{

class Input;

class CameraSystem : public ISystem
{
public:
	CameraSystem(Input& input);

	String name() override { return "CameraSystem"; }

	bool update(double time, double delta_time) override;
	void destroyEntities(const Array<Id>& entities) override;
	void defragmentTables() override {}

	void onMouseEvent(const Input& input);
	void onKeyEvent(const Input& input);

	void setNeedsUpdate() { m_camera.setNeedsUpdate(); }
	void setAspectRatio(float aspect) { m_camera.setAspectRatio(aspect); }

	const Camera& getCurrentCamera() const { return m_camera; }
	//JONDE TODO use only const version 
	Camera& getCurrentCamera() { return m_camera; }

	void connectCameraChangedEventHandler(std::function<void(const Camera&)> handler);

private:
	Input& m_input;
	Camera m_camera; // TODO Table of cameras

	void emitCameraChangedEvent();
	std::vector<std::function<void(const Camera&)>> cameraChangedEvent;
};

}
