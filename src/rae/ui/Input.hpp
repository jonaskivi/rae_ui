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

#ifdef version_cocoa
enum class MouseButton
{
	Undefined = 0,
	First, // = 1,
	Middle,
	Second,
	Fourth,
	Fifth
};
#endif
#ifdef version_glfw
enum class MouseButton
{
	Undefined = -1,
	First = 0,// = 1,
	Second,
	Middle,
	Fourth,
	Fifth
};
#endif

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
	EnterNotify,
	LeaveNotify,
	Scroll
};

class ScreenSystem;

class Input : public ISystem
{
public:
	Input(ScreenSystem& screenSystem) :
		m_screenSystem(screenSystem),
		eventType(EventType::Undefined),
		isHandled(false)
	{
	}

	String name() override { return "InputSystem"; }

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
	
		bool button(MouseButton but) const
		{
			return m_button[(int)but];
		}

		void setButton(MouseButton but, bool set)
		{
			m_button[(int)but] = set;
		}

		MouseButton eventButton = MouseButton::Undefined;

		EventType buttonEvent(MouseButton but) const
		{
			return m_buttonEvent[(int)but];
		}

		void setButtonEvent(MouseButton but, EventType set)
		{
			m_buttonEvent[(int)but] = set;
		}

		// "normalized" from 0.0f to 1.0f
		float normalizedWindowX() { return xNormalizedWindow; }
		float normalizedWindowY() { return yNormalizedWindow; }

	protected:
		bool m_button[6] = { false, false, false, false, false, false };
		EventType m_buttonEvent[6]; // the event on this frame. Will be cleared on end of frame.

		float xNormalizedWindow = 0.0f;
		float yNormalizedWindow = 0.0f;
	public:
		uint doubleClickButton = 0u;
		// In height coordinates:
		float x = 0.0f;
		float y = 0.0f;
		float xLocal = 0.0f; // These change all the time.
		float yLocal = 0.0f;
		float xRel = 0.0f;
		float yRel = 0.0f;
		float xRelLocal = 0.0f;
		float yRelLocal = 0.0f;
		// In pixels:
		float xP = 0.0f;
		float yP = 0.0f;
		float xLocalP = 0.0f; // These change all the time.
		float yLocalP = 0.0f;
		float xRelP = 0.0f;
		float yRelP = 0.0f;
		float xRelLocalP = 0.0f;
		float yRelLocalP = 0.0f;
		
		float amount = 0.0f;

		// onButtonPress locations:
		float xOnButtonPress[6]		= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float yOnButtonPress[6]		= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float xRelOnButtonPress[6]	= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float yRelOnButtonPress[6]	= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		// In pixels:
		float xOnButtonPressP[6]	= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float yOnButtonPressP[6]	= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float xRelOnButtonPressP[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		float yRelOnButtonPressP[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

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

		float x = 0.0f;
		float y = 0.0f;
		float xLocal = 0.0f; // These change all the time.
		float yLocal = 0.0f;
		float xRel = 0.0f;
		float yRel = 0.0f;
		float xRelLocal = 0.0f;
		float yRelLocal = 0.0f;
		// In pixels:
		float xP = 0.0f;
		float yP = 0.0f;
		float xLocalP = 0.0f; // These change all the time.
		float yLocalP = 0.0f;
		float xRelP = 0.0f;
		float yRelP = 0.0f;
		float xRelLocalP = 0.0f;
		float yRelLocalP = 0.0f;

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
	void osScrollEvent(float deltaX, float deltaY);
	void osKeyEvent(EventType setEventType, int setKey, int32_t setUnicode);
	void osMouseEvent(/*IRectangle* setWindow,*/ EventType setEventType, int setButton,
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

protected:

	#ifdef version_cocoa
	MouseButton intToMouseButton(int button)
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
		}
	}
	#endif
	#ifdef version_glfw
	MouseButton intToMouseButton(int button)
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
		}
	}
	#endif

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
