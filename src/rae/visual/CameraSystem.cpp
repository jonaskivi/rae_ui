#include "rae/visual/CameraSystem.hpp"

#include "rae/ui/DebugSystem.hpp"

#include "rae/core/Utils.hpp"
#include "rae/core/Time.hpp"
#include "rae/ui/Input.hpp"

using namespace rae;

CameraSystem::CameraSystem(const Time& time, EntitySystem& entitySystem, TransformSystem& transformSystem, Input& input) :
	ISystem("CameraSystem"),
	m_time(time),
	m_entitySystem(entitySystem),
	m_transformSystem(transformSystem),
	m_input(input)
{
	m_currentCamera = createCamera();

	/* RAE_TODO HOW TO HANDLE INPUT:
	using std::placeholders::_1;
	m_input.connectMouseButtonPressEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectMouseButtonReleaseEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectMouseMotionEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectScrollEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectKeyEventHandler(std::bind(&CameraSystem::onKeyEvent, this, _1));
	*/
}

void CameraSystem::onMouseEvent(const InputState& inputState)
{
	auto& camera = modifyCurrentCamera();

	//if (input.eventType == EventType::MouseMotion)
	//{
	if (inputState.mouse.isButtonDown(MouseButton::Second))
	{
		const float rotateSpeedMul = 0.01f;

		// RAE_TODO Need to redo the speedMuls because xDeltaP used to be xDeltaHeightCoords from -1 to 1.
		camera.rotateYaw(inputState.mouse.delta.x * -1.0f * rotateSpeedMul);
		camera.rotatePitch(inputState.mouse.delta.y * -1.0f * rotateSpeedMul);
	}
	/*}
	else if (input.eventType == EventType::MouseButtonPress)
	{
	}
	else if (input.eventType == EventType::MouseButtonRelease)
	{
		if (input.mouse.eventButton == MouseButton::First)
		{
		}
	}

	if (input.eventType == EventType::Scroll)
	{
		const float scrollSpeedMul = -0.1f;
		camera.plusFieldOfView(input.mouse.scrollY * scrollSpeedMul);
	}
	*/
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
	// NOTE: This doesn't matter. Look at position member init.
	vec3 position = vec3(0.0f, 0.0f, -30.0f);
	Id id = m_entitySystem.createEntity();
	m_transformSystem.addTransform(id, Transform(position));

	float fieldOfView = Math::toRadians(50.0f);
	float aspect = 16.0f / 9.0f;
	float aperture = 0.1f;
	float focusDistance = 10.0f;
	addCamera(id, Camera(fieldOfView, aspect, aperture, focusDistance));

	return id;
}

void CameraSystem::addCamera(Id id, const Camera& camera)
{
	m_cameras.assign(id, camera);
}

const Camera& CameraSystem::getCamera(Id id) const
{
	return m_cameras.get(id);
}

Camera& CameraSystem::modifyCamera(Id id)
{
	return m_cameras.modify(id);
}

void CameraSystem::connectCameraUpdatedEventHandler(std::function<void(const Camera&)> handler)
{
	cameraUpdatedEvent.push_back(handler);
}

void CameraSystem::emitCameraUpdatedEvent()
{
	auto& camera = currentCamera();

	for (auto&& handler : cameraUpdatedEvent)
	{
		handler(camera);
	}

	m_cameraUpdated = true;
}

UpdateStatus CameraSystem::update()
{
	// RAE_TODO: m_screenInfo??? from ScreenSystem???
	// RAE_TODO camera.setAspectRatio( float(m_windowPixelWidth) / float(m_windowPixelHeight) );

	auto& camera = modifyCurrentCamera();

	if (m_input.getKeyState(KeySym::Control_L))
		camera.setCameraSpeedDown(true);
	else camera.setCameraSpeedDown(false);

	if (m_input.getKeyState(KeySym::Shift_L))
		camera.setCameraSpeedUp(true);
	else camera.setCameraSpeedUp(false);

	// Rotation with arrow keys
	if (m_input.getKeyState(KeySym::Left))
		camera.rotateYaw(float(m_time.deltaTime()), +1);
	else if (m_input.getKeyState(KeySym::Right))
		camera.rotateYaw(float(m_time.deltaTime()), -1);

	if (m_input.getKeyState(KeySym::Up))
		camera.rotatePitch(float(m_time.deltaTime()), +1);
	else if (m_input.getKeyState(KeySym::Down))
		camera.rotatePitch(float(m_time.deltaTime()), -1);

	// Camera movement
	if (m_input.getKeyState(KeySym::W)) { camera.moveForward(	float(m_time.deltaTime())	); }
	if (m_input.getKeyState(KeySym::S)) { camera.moveBackward(	float(m_time.deltaTime())	); }
	if (m_input.getKeyState(KeySym::D)) { camera.moveRight(		float(m_time.deltaTime())	); }
	if (m_input.getKeyState(KeySym::A)) { camera.moveLeft(		float(m_time.deltaTime())	); }
	if (m_input.getKeyState(KeySym::E)) { camera.moveUp(		float(m_time.deltaTime())	); }
	if (m_input.getKeyState(KeySym::Q)) { camera.moveDown(		float(m_time.deltaTime())	); }

	if (m_input.getKeyState(KeySym::N)) { camera.minusAperture(); }
	if (m_input.getKeyState(KeySym::M)) { camera.plusAperture();  }

	if (m_input.getKeyState(KeySym::V)) { camera.minusFocusDistance(); }
	if (m_input.getKeyState(KeySym::B)) { camera.plusFocusDistance();  }

	UpdateStatus cameraUpdated = UpdateStatus::NotChanged;

	if (m_input.getKeyPressed(KeySym::U))
	{
		camera.toggleContinuousAutoFocus();
		cameraUpdated = UpdateStatus::Changed;
	}

	if (camera.update(m_time.time()))
	{
		cameraUpdated = UpdateStatus::Changed;
	}

	if (cameraUpdated == UpdateStatus::Changed)
	{
		emitCameraUpdatedEvent();
	}

	g_debugSystem->showDebugText("Camera position: " + Utils::toString(camera.position()), Colors::magenta);

	return cameraUpdated;
}

void CameraSystem::onFrameEnd()
{
	ISystem::onFrameEnd();

	m_cameraUpdated = false;
}
