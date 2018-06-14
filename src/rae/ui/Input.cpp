#include "rae/ui/Input.hpp"

#include "loguru/loguru.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/WindowSystem.hpp"

using namespace rae;

void Input::onFrameEnd()
{
	ISystem::onFrameEnd();

	m_changed = UpdateStatus::NotChanged;
	key.clearFrame();

	// Clear button events
	for (int i = 0; i < (int)MouseButton::Count; ++i)
	{
		MouseButton iButton = intToMouseButton(i);
		mouse.setButtonEvent(iButton, EventType::Undefined);
	}
}

bool Input::getKeyState(int keyValue) const
{
	if (keyValue < key.keyStatesSize)
		return key.keyStates[keyValue];
	return false;
}

bool Input::getKeyPressed(int keyValue) const
{
	if (keyValue < key.keyStatesSize)
		return key.pressedThisFrame[keyValue];
	return false;
}

bool Input::getKeyReleased(int keyValue) const
{
	if (keyValue < key.keyStatesSize)
		return key.releasedThisFrame[keyValue];
	return false;
}

void Input::addTouchPoint(TouchPointState setState, int setId, float xPixels, float yPixels)
{
	isHandled = false;

	eventType = EventType::Touch;

	//touch.addTouchPoint(setId, xPixels, yPixels);

	#ifdef DebugTouch
		LOG_F(INFO, "Input::addTouchPoint: x: %f y: %f", xPixels, yPixels);
	#endif

	if (setId < touch.touchPoints_size)
	{
		touch.eventId = setId;
	}
	else
	{
		touch.eventId = 0;
		LOG_F(ERROR, "ERROR: Input::addTouchPoint. ID is over 20. id: %i", setId);
	}

	for (int i = 0; i < touch.touchPoints_size; i++ )
	{
		if (setId == touch.touchPoints[i].id)
		{
			touch.touchPoints[i].state = setState;

			touch.touchPoints[i].xDeltaP = xPixels - touch.touchPoints[i].xP;
			touch.touchPoints[i].yDeltaP = yPixels - touch.touchPoints[i].yP;

			touch.touchPoints[i].xP = xPixels;
			touch.touchPoints[i].yP = yPixels;

			return;
		}
	}
	LOG_F(ERROR, "ERROR: Input::addTouchPoint. Touch ID not found. id: %i", setId);
}

void Input::osScrollEvent(const Window& window, float deltaX, float deltaY)
{
	m_changed = UpdateStatus::Changed;
	isHandled = false;
	eventType = EventType::Scroll;

	mouse.scrollX = deltaX;
	mouse.scrollY = deltaY;

	emitScrollEvent();
}

void Input::osKeyEvent(const Window& window, EventType setEventType, int setKey, int32_t setUnicode)
{
	//LOG_F(INFO, "Input.keyEvent() setKey: %i", setKey);

	m_changed = UpdateStatus::Changed;
	isHandled = false;
	//window = set_window;
	eventType = setEventType;

	key.value = setKey;
	//key.unicode = setUnicode;

	/*
	version(gtk)
	{
		key.value = setKey;
		key.unicode = keyValueToUnicode(setKey);
	}
	version(glfw)
	{
		key.value = setKey;
		key.unicode = cast(dchar) setKey;
	}
	*/

	if (setEventType == EventType::KeyPress)
	{
		//LOG_F(INFO, "KEY_PRESS: %i unicode: %i", setKey, setUnicode);
		if (key.value < key.keyStatesSize)
		{
			key.keyStates[key.value] = true;
			key.pressedThisFrame[key.value] = true;
		}
		else LOG_F(ERROR, "key.value: %i is bigger than keyStatesSize.", key.value);
	}
	else if (setEventType == EventType::KeyRelease)
	{
		//LOG_F(INFO, "KEY_RELEASE: %i unicode: %i", setKey, setUnicode);
		if (key.value < key.keyStatesSize)
		{
			key.keyStates[key.value] = false;
			key.releasedThisFrame[key.value] = true;
		}
		else LOG_F(ERROR, "key.value: %i is bigger than keyStatesSize.", key.value);
	}

	emitKeyEvent();
}

void Input::osMouseEvent(const Window& window, EventType setEventType, int setButton,
	float rawXPixels, float rawYPixels, float setAmount)
{
	//assert(setButton >= 0); // "Mouse button is smaller than 0. Platform not supported yet."

	float xPixels = rawXPixels;
	float yPixels = rawYPixels;

	//LOG_F(INFO, "Input::osMouseEvent: xPixels: %f yPixels: %f xHeight: %f yHeight: %f", xPixels, yPixels,
	//	xHeight, yHeight);

	m_changed = UpdateStatus::Changed;
	isHandled = false;
	//eventWindow = setWindow;
	eventType = setEventType;

	MouseButton mouseButton = intToMouseButton(setButton);
	mouse.eventButton = mouseButton;

	mouse.amount = setAmount;

	mouse.doubleClickButton = 0; // Zero this, we can only emit one double click button per event...
	// Is that bad? Propably ok,
	// as there can only be one eventButton per event as well.

	if (eventType == EventType::MouseMotion)
	{
		/////doubleClickValid1 = false;
	}
	else if (eventType == EventType::MouseButtonPress)
	{
		mouse.setButtonEvent(mouseButton, eventType);
		mouse.setButton(mouseButton, true);
		mouse.xOnButtonPressP[setButton] = xPixels;
		mouse.yOnButtonPressP[setButton] = yPixels;
	}
	else if (eventType == EventType::MouseButtonRelease)
	{
		/*
		//Double click handling is on button release.
		//We measure the time between mouse button releases.
		if( setButton == MouseButton.LEFT )
		{

			if( doubleClickValid1 == true )
			{
				if( doubleClickTimer1.stop() <= 0.75 )//Currently 0.75 sec. TODO preferences in class Rae.
				{
					//It's a double click.
					debug(mouse) Trace.formatln("Input.Double click LEFT.");
					mouse.doubleClickButton = setButton;
				}
				else
				{
					debug(mouse) Trace.formatln("Input.NO .... Double click LEFT." );
				}

				doubleClickTimer1.start();
			}
			else
			{
				debug(mouse) Trace.formatln("Input.Start double click timer.");
				doubleClickValid1 = true;
				doubleClickTimer1.start();
			}
		}
		*/
		mouse.setButtonEvent(mouseButton, eventType);
		mouse.setButton((MouseButton)setButton, false);
	}
	//else
	//{
		//LOG_F(INFO, "Input NO PRESS.");
	//}

	mouse.xDeltaP = xPixels - mouse.xP;
	mouse.yDeltaP = yPixels - mouse.yP;

	//

	mouse.xP = xPixels;
	mouse.yP = yPixels;

	mouse.xMM = m_screenSystem.pixelsToMM(rawXPixels);
	mouse.yMM = m_screenSystem.pixelsToMM(rawYPixels);

	mouse.xDeltaMM = m_screenSystem.pixelsToMM(mouse.xDeltaP);
	mouse.yDeltaMM = m_screenSystem.pixelsToMM(mouse.yDeltaP);

	mouse.xNormalizedWindow = window.xPixelsToNormalizedWindow(rawXPixels);
	mouse.yNormalizedWindow = window.yPixelsToNormalizedWindow(rawYPixels);

	if (eventType == EventType::MouseMotion)
	{
		emitMouseMotionEvent();
	}
	else if (eventType == EventType::MouseButtonPress)
	{
		emitMouseButtonPressEvent();
	}
	else if (eventType == EventType::MouseButtonRelease)
	{
		emitMouseButtonReleaseEvent();
	}
}
