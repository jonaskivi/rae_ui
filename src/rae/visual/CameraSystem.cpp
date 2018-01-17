#include "CameraSystem.hpp"

#include "core/Utils.hpp"
#include "ui/Input.hpp"

using namespace rae;

CameraSystem::CameraSystem(EntitySystem& entitySystem, TransformSystem& transformSystem, Input& input) :
	m_entitySystem(entitySystem),
	m_transformSystem(transformSystem),
	m_input(input)
{
	createCamera();

	using std::placeholders::_1;
	m_input.connectMouseButtonPressEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectMouseButtonReleaseEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectMouseMotionEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectScrollEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectKeyEventHandler(std::bind(&CameraSystem::onKeyEvent, this, _1));
}

void CameraSystem::onMouseEvent(const Input& input)
{
	auto& camera = getCurrentCamera();

	if (input.eventType == EventType::MouseMotion)
	{
		if (input.mouse.button(MouseButton::Second))
		{
			//cout << "RenderSystem mouse motion. x: " << input->mouse.xRel
			//	<< " y: " << input->mouse.yRel << endl;

			const float rotateSpeedMul = 5.0f;

			camera.rotateYaw(input.mouse.xRel * -1.0f * rotateSpeedMul);
			camera.rotatePitch(input.mouse.yRel * -1.0f * rotateSpeedMul);
		}
	}
	else if (input.eventType == EventType::MouseButtonPress)
	{
		//cout << "RenderSystem mouse press. x: " << input->mouse.x
		//	<< " y: " << input->mouse.y << endl;
	}
	else if (input.eventType == EventType::MouseButtonRelease)
	{
		if (input.mouse.eventButton == MouseButton::First)
		{
			//cout << "RenderSystem mouse release. x: " << input->mouse.x
			//	<< " y: " << input->mouse.y << endl;
		}
	}

	if (input.eventType == EventType::Scroll)
	{
		const float scrollSpeedMul = -0.1f;
		camera.plusFieldOfView(input.mouse.scrollY * scrollSpeedMul);
	}
}

void CameraSystem::onKeyEvent(const Input& input)
{
	/*
	if (input.eventType == EventType::KeyPress)
	{
		switch (input.key.value)
		{
			//case KeySym::P: m_entitySystem.measure(); break;
			default:
			break;
		}
	}
	*/
}

Id CameraSystem::createCamera()
{
	vec3 position = vec3(0.0f, 0.0f, 0.0f);
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	float fieldOfView = Math::toRadians(20.0f);
	float aspect = 16.0f / 9.0f;
	float aperture = 0.1f;
	float focusDistance = 10.0f;
	Camera camera(fieldOfView, aspect, aperture, focusDistance);
	addCamera(id, std::move(camera));

	return id;
}

void CameraSystem::addCamera(Id id, Camera&& comp)
{
	m_cameras.assign(id, std::move(comp));
}

const Camera& CameraSystem::getCamera(Id id) const
{
	return m_cameras.get(id);
}

Camera& CameraSystem::getCamera(Id id)
{
	return m_cameras.get(id);
}

void CameraSystem::connectCameraChangedEventHandler(std::function<void(const Camera&)> handler)
{
	cameraChangedEvent.push_back(handler);
}

void CameraSystem::emitCameraChangedEvent()
{
	auto& camera = getCurrentCamera();

	for (auto&& handler : cameraChangedEvent)
	{
		handler(camera);
	}
}

bool CameraSystem::update(double time, double delta_time)
{
	// RAE_TODO: m_screenInfo??? from ScreenSystem???
	// RAE_TODO camera.setAspectRatio( float(m_windowPixelWidth) / float(m_windowPixelHeight) );

	auto& camera = getCurrentCamera();

	if (m_input.getKeyState(KeySym::Control_L))
		camera.setCameraSpeedDown(true);
	else camera.setCameraSpeedDown(false);

	if (m_input.getKeyState(KeySym::Shift_L))
		camera.setCameraSpeedUp(true);
	else camera.setCameraSpeedUp(false);

	// Rotation with arrow keys
	if (m_input.getKeyState(KeySym::Left))
		camera.rotateYaw(float(delta_time), +1);
	else if (m_input.getKeyState(KeySym::Right))
		camera.rotateYaw(float(delta_time), -1);

	if (m_input.getKeyState(KeySym::Up))
		camera.rotatePitch(float(delta_time), +1);
	else if (m_input.getKeyState(KeySym::Down))
		camera.rotatePitch(float(delta_time), -1);

	// Camera movement
	if (m_input.getKeyState(KeySym::W)) { camera.moveForward(float(delta_time));  }
	if (m_input.getKeyState(KeySym::S)) { camera.moveBackward(float(delta_time)); }
	if (m_input.getKeyState(KeySym::D)) { camera.moveRight(float(delta_time));    }
	if (m_input.getKeyState(KeySym::A)) { camera.moveLeft(float(delta_time));     }
	if (m_input.getKeyState(KeySym::E)) { camera.moveUp(float(delta_time));       }
	if (m_input.getKeyState(KeySym::Q)) { camera.moveDown(float(delta_time));     }

	if (m_input.getKeyState(KeySym::N)) { camera.minusAperture(); }
	if (m_input.getKeyState(KeySym::M)) { camera.plusAperture();  }

	if (m_input.getKeyState(KeySym::V)) { camera.minusFocusDistance(); }
	if (m_input.getKeyState(KeySym::B)) { camera.plusFocusDistance();  }

	bool cameraChanged = false;

	if (m_input.getKeyPressed(KeySym::F))
	{
		camera.toggleContinuousAutoFocus();
		cameraChanged = true;
	}

	if (camera.update(time, delta_time))
	{
		cameraChanged = true;
	}

	if (cameraChanged)
		emitCameraChangedEvent();

	return cameraChanged;
}
