/*
* The X11/MIT License
*
* Copyright (c) 2008-2018, Jonas Kivi
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#pragma once

#include "rae/ui/KeySym.hpp"

#include <cassert>
#include <vector>
#include <functional>

#include "rae/core/version.hpp"

#include "rae/core/ISystem.hpp"

namespace rae
{

enum class TouchPointState
{
	Undefined,
	Pressed, // finger touched the screen on last frame
	Released, // finger left the screen on last frame
	Moved, // finger position moved
	Stationary // i.e. didn't move from last frame
};

// Notice that:
// Gtk  MIDDLE = 2, RIGHT = 3
// GLFW MIDDLE = 3, RIGHT = 2
// Cocoa first = 1
// GLFW first = 0

enum class MouseButton
{
	Undefined = -1,
	First = 0,// = 1,
	Second,
	Middle,
	Fourth,
	Fifth,
	Sixth,
	Count
};

enum class EventType
{
	Undefined,
	MouseMotion,
	MouseButtonPress,
	MouseButtonRelease,
	// since multitouch has so many beginnings, updates and ends,
	// we don't do separate events, but you'll have to track them yourself,
	// from the states and lists of touchpoints.
	Touch,
	KeyPress,
	KeyRelease,
	KeyHold,
	MouseEnter,
	MouseLeave,
	Scroll
};

class ScreenSystem;
class Window;

struct InputEvent
{
	InputEvent(EventType type) :
		eventType(type)
	{
	}

	InputEvent(EventType type, MouseButton button, float x, float y) :
		eventType(type),
		mouseButton(button),
		x(x),
		y(y)
	{
	}

	EventType eventType;
	MouseButton mouseButton = MouseButton::Undefined;

	float x = 0.0f;
	float y = 0.0f;
};

// This is actually the processed/combined events of a UI scene during one frame, and some persistent input state.
struct InputState
{
	InputState()
	{
	}

	void clear()
	{
		mouse.clear();
	}

	void handleEvents(const Array<InputEvent>& events)
	{
		for (auto&& event : events)
		{
			if (event.eventType == EventType::MouseButtonRelease)
			{
				if (event.mouseButton != MouseButton::Undefined)
				{
					mouse.buttonClicked[(int)event.mouseButton] = true;
				}
			}
			else if (/*!isGrabbed() &&*/ event.eventType == EventType::MouseEnter)
			{
				mouseInside = true;
			}
			else if (/*!isGrabbed() &&*/ event.eventType == EventType::MouseLeave)
			{
				mouseInside = false;
			}
			else if (event.eventType == EventType::MouseMotion)
			{
				processMouseMotionEvent(event);
			}
			else if (event.eventType == EventType::Scroll)
			{
				processScrollEvent(event);
			}
		}
	}

	void processMouseMotionEvent(const InputEvent& event)
	{
		// This can't work well. The previous pos is sometimes going to be on the otherside of the window etc.
		// and that is going to generate really big mouseDelta on some frames. Unless leave and enter events
		// are handled properly.
		mouse.position = vec3(event.x, event.y, 0.0f);
		mouse.delta += mouse.position - mouse.previousPosition;
		mouse.previousPosition = mouse.position;
	}

	void processScrollEvent(const InputEvent& event)
	{
		//mouse.scroll
	}

	struct Mouse
	{
		void clear()
		{
			for (int i = 0; i < (int)MouseButton::Count; ++i)
			{
				buttonClicked[i] = false;
			}

			delta  = vec3(0.0f, 0.0f, 0.0f);
			scroll = vec2(0.0f, 0.0f);
		}

		bool anyButtonDown() const
		{
			for (int i = 0; i < (int)MouseButton::Count; ++i)
			{
				if (buttonState[i])
					return true;
			}
			return false;
		}

		bool isButtonDown(MouseButton button) const
		{
			return buttonState[(int)button];
		}

		bool wasButtonClicked(MouseButton button) const
		{
			return buttonClicked[(int)button];
		}

		// Currently this is really wasButtonReleased...
		bool buttonClicked[(int)MouseButton::Count] = { false, false, false, false, false, false };
		bool buttonState[(int)MouseButton::Count]   = { false, false, false, false, false, false };

		vec3 position         = vec3(0.0f, 0.0f, 0.0f);
		vec3 previousPosition = vec3(0.0f, 0.0f, 0.0f);
		vec3 delta            = vec3(0.0f, 0.0f, 0.0f);
		vec2 scroll           = vec2(0.0f, 0.0f);

		// If we are hovering something in UIScene, this is the local coordinates of that UI element. From 0 to 1.
		vec3 localPositionNormalized = vec3(0.0f, 0.0f, 0.0f);
	};

	Mouse mouse;

	// If the UI scene had events during this frame.
	bool	hadEvents = false;
	// If the mouse position is inside the window where this UI scene is.
	bool	mouseInside = false;

	bool isGrabbed() const { return m_grabbedId != InvalidId; }
	void grab(Id id) { m_grabbedId = id; }
	void clearGrab() { m_grabbedId = InvalidId; }

	bool isKeyboardFocus() const { return m_keyboardFocusId != InvalidId; }
	void setKeyboardFocus(Id id) { m_keyboardFocusId = id; }
	Id getKeyboardFocus() const { return m_keyboardFocusId; }
	void clearKeyboardFocus() { m_keyboardFocusId = InvalidId; }

protected:
	Id		m_grabbedId = InvalidId;
	Id		m_keyboardFocusId = InvalidId;
};

class Input : public ISystem
{
public:
	Input(ScreenSystem& screenSystem) :
		ISystem("InputSystem"),
		m_screenSystem(screenSystem),
		eventType(EventType::Undefined),
		isHandled(false)
	{
	}

	UpdateStatus update() override
	{
		return m_changed;
	}

	// Must be called every frame to clear key up and down states
	void onFrameEnd() override;

	EventType eventType;
	//IRectangle* eventWindow;
	bool isHandled;
	UpdateStatus m_changed = UpdateStatus::NotChanged;

	struct Mouse
	{
		friend class Input;

		Mouse()
		{
		}

		bool anyButtonDown() const
		{
			for (int i = 0; i < (int)MouseButton::Count; ++i)
			{
				if (m_button[i])
					return true;
			}
			return false;
		}

		bool isButtonDown(MouseButton button) const
		{
			assert(int(button) != -1);
			return m_button[(int)button];
		}

		void setButtonDown(MouseButton button, bool set)
		{
			assert(int(button) != -1);
			m_button[(int)button] = set;
		}

		MouseButton eventButton = MouseButton::Undefined;

		EventType buttonEvent(MouseButton button) const
		{
			assert(int(button) != -1);
			return m_buttonEvent[(int)button];
		}

		void setButtonEvent(MouseButton button, EventType set)
		{
			assert(int(button) != -1);
			m_buttonEvent[(int)button] = set;
		}

		// "normalized" from 0.0f to 1.0f
		float normalizedWindowX() { return xNormalizedWindow; }
		float normalizedWindowY() { return yNormalizedWindow; }

	protected:
		bool m_button[(int)MouseButton::Count] = { false, false, false, false, false, false };
		EventType m_buttonEvent[(int)MouseButton::Count]; // the event on this frame. Will be cleared on end of frame.

		float xNormalizedWindow = 0.0f;
		float yNormalizedWindow = 0.0f;
	public:
		uint doubleClickButton = 0u;

		// In pixels:
		float xP = 0.0f;
		float yP = 0.0f;
		float xDeltaP = 0.0f;
		float yDeltaP = 0.0f;

		// In millimeters
		float xMM = 0.0f;
		float yMM = 0.0f;
		float xDeltaMM = 0.0f;
		float yDeltaMM = 0.0f;

		float amount = 0.0f;

		// onButtonPress locations:
		float xOnButtonPress[(int)MouseButton::Count] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float yOnButtonPress[(int)MouseButton::Count] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		// In pixels:
		float xOnButtonPressP[(int)MouseButton::Count]      = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float yOnButtonPressP[(int)MouseButton::Count]      = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float xDeltaOnButtonPressP[(int)MouseButton::Count] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float yDeltaOnButtonPressP[(int)MouseButton::Count] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		// Scroll events:
		float scrollX = 0.0f;
		float scrollY = 0.0f;
	};

	Mouse mouse;

	struct TouchPoint
	{
		TouchPoint()
		{
		}

		int id = 0;

		TouchPointState state = TouchPointState::Undefined;

		// In pixels. Y-down. Origin in top-left of the window. Simple raw pixels.
		float xP = 0.0f;
		float yP = 0.0f;
		float xDeltaP = 0.0f;
		float yDeltaP = 0.0f;

		// In millimeters
		float xMM = 0.0f;
		float yMM = 0.0f;

		void printInfo()
		{
			//RaeLog<<"TouchPoint: id: "<<id<<" "<<TouchPointState::toString(state)<<" x: "<<x<<" y: "<<y<<" xLocal: "<<xLocal<<" yLocal: "<<yLocal
			/*
			RaeLog<<"TouchPoint: id: "<<id<<" x: "<<x<<" y: "<<y<<" xLocal: "<<xLocal<<" yLocal: "<<yLocal
				<<" xRel: "<<xRel<<" yRel: "<<yRel<<" xRelLocal: "<<xRelLocal<<" yRelLocal: "<<yRelLocal
				<<" xP: "<<xP<<" yP: "<<yP<<" xLocalP: "<<xLocalP<<" yLocalP: "<<yLocalP
				<<" xRelP: "<<xRelP<<" yRelP: "<<yRelP<<" xRelLocalP: "<<xRelLocalP<<" yRelLocalP: "<<yRelLocalP<<"\n";
				*/
		}
	};

	struct Touch
	{
		Touch()
		{
			touchPoints_size = 20;

			eventId = 0;

			for (int i = 0; i < touchPoints_size; i++ )
			{
				touchPoints[i].id = i;
			}
		}

		TouchPoint* currentTouchPoint()
		{
			return &touchPoints[eventId];
		}

		int eventId; // current touch event id that we are handling.

		int touchPoints_size;// = 20
		TouchPoint touchPoints[20]; // TODO replace with std::array...
	};

	Touch touch;

	struct Keyboard
	{
		Keyboard()
		{
			for (uint i = 0; i < keyStatesSize; i++ )
			{
				keyStates[i] = false;
				pressedThisFrame[i] = false;
				releasedThisFrame[i] = false;
			}
		}

		void clearFrame()
		{
			for (uint i = 0; i < keyStatesSize; i++ )
			{
				pressedThisFrame[i] = false;
				releasedThisFrame[i] = false;
			}
		}

		static const uint keyStatesSize = 256 * 2;

		uint value = 0;
		//string unicode = 0;
		char* unicode;

		bool keyStates[keyStatesSize];
		// Cleared every frame
		bool pressedThisFrame[keyStatesSize];
		bool releasedThisFrame[keyStatesSize];
	};

	Keyboard key;

	bool getKeyState(int keyValue) const; // The current state of the key, true is down, false is up.
	bool getKeyPressed(int keyValue) const; // If the key was pressed during this frame
	bool getKeyReleased(int keyValue) const; // If the key was released during this frame

	void addTouchPoint(TouchPointState setState, int setId, float xPixels, float yPixels);
	void osScrollEvent(const Window& window, float deltaX, float deltaY);
	void osKeyEvent(const Window& window, EventType setEventType, int setKey, int32_t setUnicode);
	void osMouseEvent(const Window& window, EventType setEventType, int setButton,
	float rawXPixels, float rawYPixels, float setAmount = 0.0f);

	void connectMouseButtonPressEventHandler(std::function<void(const Input&)> set)
	{
		mouseButtonPressEvent.push_back(set);
	}

	void connectMouseButtonReleaseEventHandler(std::function<void(const Input&)> set)
	{
		mouseButtonReleaseEvent.push_back(set);
	}

	void connectMouseMotionEventHandler(std::function<void(const Input&)> set)
	{
		mouseMotionEvent.push_back(set);
	}

	void connectScrollEventHandler(std::function<void(const Input&)> set)
	{
		scrollEvent.push_back(set);
	}

	void connectKeyEventHandler(std::function<void(const Input&)> set)
	{
		keyEvent.push_back(set);
	}

	#ifdef version_cocoa
	static MouseButton intToMouseButton(int button)
	{
		switch(button)
		{
			default:
			case 0: return MouseButton::Undefined;
			case 1: return MouseButton::First;
			case 2: return MouseButton::Second;
			case 3: return MouseButton::Middle;
			case 4: return MouseButton::Fourth;
			case 5: return MouseButton::Fifth;
			case 6: return MouseButton::Sixth;
		}
	}
	#endif
	#ifdef version_glfw
	static MouseButton intToMouseButton(int button)
	{
		switch(button)
		{
			default:
			case -1: return MouseButton::Undefined;
			case 0: return MouseButton::First;
			case 1: return MouseButton::Second;
			case 2: return MouseButton::Middle;
			case 3: return MouseButton::Fourth;
			case 4: return MouseButton::Fifth;
			case 5: return MouseButton::Sixth;
		}
	}
	#endif

protected:

	ScreenSystem& m_screenSystem;

	void emitMouseButtonPressEvent() { for (auto&& event : mouseButtonPressEvent) event(*this); }
	std::vector<std::function<void(const Input&)>> mouseButtonPressEvent;
	void emitMouseButtonReleaseEvent() { for (auto&& event : mouseButtonReleaseEvent) event(*this); }
	std::vector<std::function<void(const Input&)>> mouseButtonReleaseEvent;
	void emitMouseMotionEvent() { for (auto&& event : mouseMotionEvent) event(*this); }
	std::vector<std::function<void(const Input&)>> mouseMotionEvent;
	void emitScrollEvent() { for (auto&& event : scrollEvent) event(*this); }
	std::vector<std::function<void(const Input&)>> scrollEvent;
	void emitKeyEvent() { for (auto&& event : keyEvent) event(*this); }
	std::vector<std::function<void(const Input&)>> keyEvent;
}; //end Input

} // end Rae
