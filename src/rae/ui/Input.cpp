#include "rae/ui/Input.hpp"

#include "rae/core/Log.hpp"
#include "rae/core/ScreenSystem.hpp"

using namespace rae;

void Input::onFrameEnd()
{
	ISystem::onFrameEnd();

	m_changed = UpdateStatus::NotChanged;
	key.clearFrame();

	// Clear button events
	for (int i = 0; i < 6; ++i)
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
	float xHeight = m_screenSystem.pixelsToHeight(xPixels);
	float yHeight = m_screenSystem.pixelsToHeight(yPixels);

	#ifdef DebugTouch
		RaeLog<<"addTouchPoint: x: "<<xPixels<<" y: "<<yPixels<<"\n";
	#endif

	if (setId < touch.touchPoints_size)
	{
		touch.eventId = setId;
	}
	else
	{
		touch.eventId = 0;
		std::cout << "ERROR: Rae.Input.addTouchPoint. ID is over 20. id: " << setId;
	}

	for (int i = 0; i < touch.touchPoints_size; i++ )
	{
		if (setId == touch.touchPoints[i].id)
		{
			touch.touchPoints[i].state = setState;

			touch.touchPoints[i].xRelP = xPixels - touch.touchPoints[i].xP;
			touch.touchPoints[i].yRelP = yPixels - touch.touchPoints[i].yP;
			touch.touchPoints[i].xRel = xHeight - touch.touchPoints[i].x;
			touch.touchPoints[i].yRel = yHeight - touch.touchPoints[i].y;

			touch.touchPoints[i].xP = xPixels;
			touch.touchPoints[i].yP = yPixels;
			touch.touchPoints[i].x = xHeight;
			touch.touchPoints[i].y = yHeight;

			/*
			//Do we need to init these?
			touch.touchPoints[i].xLocalP = xPixels;
			touch.touchPoints[i].yLocalP = yPixels;
			touch.touchPoints[i].xLocal = xHeight;
			touch.touchPoints[i].yLocal = yHeight;
			*/
			return;
		}
	}
	std::cout << "Rae::Input ERROR: Touch ID not found. id: " << setId << "\n";
}

void Input::osScrollEvent(float deltaX, float deltaY)
{
	m_changed = UpdateStatus::Changed;
	isHandled = false;
	eventType = EventType::Scroll;
	
	mouse.scrollX = deltaX;
	mouse.scrollY = deltaY;

	emitScrollEvent();
}

void Input::osKeyEvent(EventType setEventType, int setKey, int32_t setUnicode)
{
	//rae_log("Input.keyEvent() setKey:", setKey);

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
		//rae_log("KEY_PRESS: ", hex, setKey, dec, " unicode: ", setUnicode);
		if (key.value < key.keyStatesSize)
		{
			key.keyStates[key.value] = true;
			key.pressedThisFrame[key.value] = true;
		}
		else rae_log_error("ERROR: key.value: ", key.value, " is bigger than keyStatesSize.");
	}
	else if (setEventType == EventType::KeyRelease)
	{
		//rae_log("KEY_RELEASE: ", hex, setKey, dec, " unicode: ", setUnicode);
		if (key.value < key.keyStatesSize)
		{
			key.keyStates[key.value] = false;
			key.releasedThisFrame[key.value] = true;
		}
		else rae_log_error("ERROR: key.value: ", key.value, " is bigger than keyStatesSize.");
	}

	emitKeyEvent();
}

void Input::osMouseEvent(/*IRectangle* setWindow,*/ EventType setEventType, int setButton,
	float rawXPixels, float rawYPixels, float setAmount)
{
	//assert(setButton >= 0); // "Mouse button is smaller than 0. Platform not supported yet."

	// Convert from topleft (0.0f -> width) to centered pixel coordinates.
	const auto& window = m_screenSystem.window();
	float xPixels = rawXPixels - (window.pixelWidth() * 0.5f);
	float yPixels = rawYPixels - (window.pixelHeight() * 0.5f);

	// TODO: possibly split coordinate conversion functions to m_windowSystem, which knows
	// the conversion ratios per window.
	float xHeight = m_screenSystem.pixelsToHeight(xPixels);
	float yHeight = m_screenSystem.pixelsToHeight(yPixels);

	//rae_log("Input::osMouseEvent: xPixels: ", xPixels, " yPixels: ", yPixels,
	// " xHeight: ", xHeight, " yHeight: ", yHeight);

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
		mouse.xOnButtonPress[setButton] = xHeight;
		mouse.yOnButtonPress[setButton] = yHeight;
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
		//rae_log("Input NO PRESS.");
	//}
	
	mouse.xRelP = xPixels - mouse.xP;
	mouse.yRelP = yPixels - mouse.yP;
	mouse.xRel = xHeight - mouse.x;
	mouse.yRel = yHeight - mouse.y;
	
	//
	
	mouse.xP = xPixels;
	mouse.yP = yPixels;
	mouse.xLocalP = xPixels;
	mouse.yLocalP = yPixels;

	mouse.x = xHeight;
	mouse.y = yHeight;
	mouse.xLocal = xHeight;
	mouse.yLocal = yHeight;

	// These coordinate systems are bloody stupid. xPixels should be from 0 to width, instead of being centered.
	// x could then be centered (like it is already).
	mouse.xNormalizedWindow = m_screenSystem.xPixelsToNormalizedWindow(rawXPixels);
	mouse.yNormalizedWindow = m_screenSystem.yPixelsToNormalizedWindow(rawYPixels);

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
