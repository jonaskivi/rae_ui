#include "CameraSystem.hpp"

#include "core/Utils.hpp"
#include "ui/Input.hpp"

using namespace rae;

CameraSystem::CameraSystem(Input& input)
: m_input(input),
m_camera(/*fieldOfView*/Math::toRadians(20.0f), /*aspect*/16.0f / 9.0f, /*aperture*/0.1f, /*focusDistance*/10.0f)
{
	using std::placeholders::_1;
	m_input.connectMouseButtonPressEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectMouseButtonReleaseEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectMouseMotionEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectScrollEventHandler(std::bind(&CameraSystem::onMouseEvent, this, _1));
	m_input.connectKeyEventHandler(std::bind(&CameraSystem::onKeyEvent, this, _1));
}

void CameraSystem::onMouseEvent(const Input& input)
{
	if (input.eventType == EventType::MouseMotion)
	{
		if (input.mouse.button(MouseButton::Second))
		{
			//cout << "RenderSystem mouse motion. x: " << input->mouse.xRel
			//	<< " y: " << input->mouse.yRel << endl;

			const float rotateSpeedMul = 5.0f;

			m_camera.rotateYaw(input.mouse.xRel * -1.0f * rotateSpeedMul);
			m_camera.rotatePitch(input.mouse.yRel * -1.0f * rotateSpeedMul);
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
		m_camera.plusFieldOfView(input.mouse.scrollY * scrollSpeedMul);
	}
}

void CameraSystem::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KeyPress)
	{
		switch (input.key.value)
		{
			//case KeySym::P: m_objectFactory.measure(); break;
			default:
			break;
		}
	}
}

void CameraSystem::connectCameraChangedEventHandler(std::function<void(const Camera&)> handler)
{
	cameraChangedEvent.push_back(handler);
}

void CameraSystem::emitCameraChangedEvent()
{
	for (auto&& handler : cameraChangedEvent)
	{
		handler(m_camera);
	}
}

bool CameraSystem::update(double time, double delta_time)
{
	//JONDE TODO: m_screenInfo??? from ScreenSystem???
	//JONDE m_camera.setAspectRatio( float(m_windowPixelWidth) / float(m_windowPixelHeight) );

	if (m_input.getKeyState(KeySym::Control_L))
		m_camera.setCameraSpeedDown(true);
	else m_camera.setCameraSpeedDown(false);

	if (m_input.getKeyState(KeySym::Shift_L))
		m_camera.setCameraSpeedUp(true);
	else m_camera.setCameraSpeedUp(false);

	// Rotation with arrow keys
	if (m_input.getKeyState(KeySym::Left))
		m_camera.rotateYaw(float(delta_time), +1);
	else if (m_input.getKeyState(KeySym::Right))
		m_camera.rotateYaw(float(delta_time), -1);

	if (m_input.getKeyState(KeySym::Up))
		m_camera.rotatePitch(float(delta_time), +1);
	else if (m_input.getKeyState(KeySym::Down))
		m_camera.rotatePitch(float(delta_time), -1);

	// Camera movement
	if (m_input.getKeyState(KeySym::W)) { m_camera.moveForward(float(delta_time));  }
	if (m_input.getKeyState(KeySym::S)) { m_camera.moveBackward(float(delta_time)); }
	if (m_input.getKeyState(KeySym::D)) { m_camera.moveRight(float(delta_time));    }
	if (m_input.getKeyState(KeySym::A)) { m_camera.moveLeft(float(delta_time));     }
	if (m_input.getKeyState(KeySym::E)) { m_camera.moveUp(float(delta_time));       }
	if (m_input.getKeyState(KeySym::Q)) { m_camera.moveDown(float(delta_time));     }

	if (m_input.getKeyState(KeySym::N)) { m_camera.minusAperture(); }
	if (m_input.getKeyState(KeySym::M)) { m_camera.plusAperture();  }

	if (m_input.getKeyState(KeySym::V)) { m_camera.minusFocusDistance(); }
	if (m_input.getKeyState(KeySym::B)) { m_camera.plusFocusDistance();  }

	bool cameraChanged = false;

	if (m_input.getKeyPressed(KeySym::F))
	{
		m_camera.toggleContinuousAutoFocus();
		cameraChanged = true;
	}

	if (m_camera.update(time, delta_time))
	{
		cameraChanged = true;
	}

	if (cameraChanged)
		emitCameraChangedEvent();

	return cameraChanged;
}

