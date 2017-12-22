/*
* The X11/MIT License
* 
* Copyright (c) 2008-2011, Jonas Kivi
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

#ifndef _rae_ui_Input_hpp_
#define _rae_ui_Input_hpp_

#include "ui/KeySym.hpp"

#include <cassert>
#include <iostream>
#include <vector>
#include <functional>

#include "core/version.hpp"

#include "core/ScreenSystem.hpp"

#include "System.hpp"

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

/*JONDE REMOVE
struct MouseButtonState
{
	MouseButtonState(MouseButton set)
	: button(set)
	{
	}

	MouseButton button;
	EventType event;
};
*/

class Input : public System
{
public:
	Input(ScreenSystem& screenSystem)
	: m_screenSystem(screenSystem),
	eventType(EventType::Undefined),
	isHandled(false)
	{
	
	}

	String name() override { return "InputSystem"; }

	bool update(double time, double deltaTime, std::vector<Entity>& entities) override
	{
		return m_changed;
	}

	// Must be called every frame to clear key up and down states
	void onFrameEnd() override
	{
		m_changed = false;
		key.clearFrame();

		// Clear button events
		for (int i = 0; i < 6; ++i)
		{
			MouseButton iButton = intToMouseButton(i);
			mouse.setButtonEvent(iButton, EventType::Undefined);
		}
	}

	EventType eventType;
	//IRectangle* eventWindow;	
	bool isHandled;
	bool m_changed = false;
	
	struct Mouse
	{
		Mouse()
		: //button((bool[]){false, false, false, false, false, false}),
			doubleClickButton(0),
			x(0.0f),
			y(0.0f),
			xLocal(0.0f),
			yLocal(0.0f),
			xRel(0.0f),
			yRel(0.0f),
			xRelLocal(0.0f),
			yRelLocal(0.0f),
			xP(0.0f),
			yP(0.0f),
			xLocalP(0.0f),
			yLocalP(0.0f),
			xRelP(0.0f),
			yRelP(0.0f),
			xRelLocalP(0.0f),
			yRelLocalP(0.0f),
			amount(0.0f),
			//onButtonPress locations:
			/*
			xOnButtonPress((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			yOnButtonPress((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			xRelOnButtonPress((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			yRelOnButtonPress((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			xOnButtonPressP((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			yOnButtonPressP((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			xRelOnButtonPressP((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			yRelOnButtonPressP((float[]){0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}),
			*/
			scrollX(0.0f),
			scrollY(0.0f)
		{
			for(uint i = 0; i < 6; i++)
				m_button[i] = false;

			for(uint i = 0; i < 6; i++)
				xOnButtonPress[i] = 0.0f;
			for(uint i = 0; i < 6; i++)
				yOnButtonPress[i] = 0.0f;
			for(uint i = 0; i < 6; i++)
				xRelOnButtonPress[i] = 0.0f;
			for(uint i = 0; i < 6; i++)
				yRelOnButtonPress[i] = 0.0f;
			for(uint i = 0; i < 6; i++)
				xOnButtonPressP[i] = 0.0f;
			for(uint i = 0; i < 6; i++)
				yOnButtonPressP[i] = 0.0f;
			for(uint i = 0; i < 6; i++)
				xRelOnButtonPressP[i] = 0.0f;
			for(uint i = 0; i < 6; i++)
				yRelOnButtonPressP[i] = 0.0f;
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

	protected:
		bool m_button[6];
		EventType m_buttonEvent[6]; // the event on this frame. Will be cleared on end of frame.
	public:
		uint doubleClickButton;// = 0;
		//in height coordinates:
		float x;// = 0.0;
		float y;// = 0.0;
		float xLocal;// = 0.0;//These change all the time.
		float yLocal;// = 0.0;
		float xRel;// = 0.0;
		float yRel;// = 0.0;
		float xRelLocal;// = 0.0;
		float yRelLocal;// = 0.0;
		//in pixels:
		float xP;// = 0.0;
		float yP;// = 0.0;
		float xLocalP;// = 0.0;//These change all the time.
		float yLocalP;// = 0.0;
		float xRelP;// = 0.0;
		float yRelP;// = 0.0;
		float xRelLocalP;// = 0.0;
		float yRelLocalP;// = 0.0;
		
		float amount;// = 0.0f;
		//onButtonPress locations:
		float xOnButtonPress[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		float yOnButtonPress[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		float xRelOnButtonPress[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		float yRelOnButtonPress[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		
		//in pixels:
		float xOnButtonPressP[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		float yOnButtonPressP[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		float xRelOnButtonPressP[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		float yRelOnButtonPressP[6];// = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
		
		//Scroll events:
		float scrollX;
		float scrollY;
	};
	
	Mouse mouse;

	struct TouchPoint
	{
		TouchPoint()
		: id(0),
			state(TouchPointState::Undefined),
			x(0.0f),
			y(0.0f),
			xLocal(0.0f),
			yLocal(0.0f),
			xRel(0.0f),
			yRel(0.0f),
			xRelLocal(0.0f),
			yRelLocal(0.0f),
			xP(0.0f),
			yP(0.0f),
			xLocalP(0.0f),
			yLocalP(0.0f),
			xRelP(0.0f),
			yRelP(0.0f),
			xRelLocalP(0.0f),
			yRelLocalP(0.0f)
		{
		}

		int id;

		TouchPointState state;

		float x;// = 0.0;
		float y;// = 0.0;
		float xLocal;// = 0.0;//These change all the time.
		float yLocal;// = 0.0;
		float xRel;// = 0.0;
		float yRel;// = 0.0;
		float xRelLocal;// = 0.0;
		float yRelLocal;// = 0.0;
		//in pixels:
		float xP;// = 0.0;
		float yP;// = 0.0;
		float xLocalP;// = 0.0;//These change all the time.
		float yLocalP;// = 0.0;
		float xRelP;// = 0.0;
		float yRelP;// = 0.0;
		float xRelLocalP;// = 0.0;
		float yRelLocalP;// = 0.0;

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

			for(int i = 0; i < touchPoints_size; i++ )
			{
				touchPoints[i].id = i;
			}
		}

		TouchPoint* currentTouchPoint()
		{
			return &touchPoints[eventId];
		}

		int eventId;//current touch event id that we are handling.

		int touchPoints_size;// = 20
		TouchPoint touchPoints[20];
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
		//string unicode;// = 0;
		char* unicode;
		
		bool keyStates[keyStatesSize];
		// Cleared every frame
		bool pressedThisFrame[keyStatesSize];
		bool releasedThisFrame[keyStatesSize];
	};
	Keyboard key;

	bool getKeyState(int keyValue) const
	{
		if (keyValue < key.keyStatesSize)
			return key.keyStates[keyValue];
		return false;
	}

	bool getKeyPressed(int keyValue) const
	{
		if (keyValue < key.keyStatesSize)
			return key.pressedThisFrame[keyValue];
		return false;
	}

	// return true if the key was released during this frame
	bool getKeyReleased(int keyValue) const
	{
		if (keyValue < key.keyStatesSize)
			return key.releasedThisFrame[keyValue];
		return false;
	}

	void addTouchPoint( TouchPointState set_state, int set_id, float set_x_pixels, float set_y_pixels )
	{
		isHandled = false;

		eventType = EventType::Touch;

		//touch.addTouchPoint(set_id, set_x_pixels, set_y_pixels);
		set_x_height = m_screenSystem.pixelsToHeight(set_x_pixels);
		set_y_height = m_screenSystem.pixelsToHeight(set_y_pixels);

		#ifdef DebugTouch
			RaeLog<<"addTouchPoint: x: "<<set_x_pixels<<" y: "<<set_y_pixels<<"\n";
		#endif

		if( set_id < touch.touchPoints_size )
		{
			touch.eventId = set_id;
		}
		else
		{
			touch.eventId = 0;
			cout << "ERROR: Rae.Input.addTouchPoint. ID is over 20. id: " << set_id;
		}

		for( int i = 0; i < touch.touchPoints_size; i++ )
		{
			if( set_id == touch.touchPoints[i].id )
			{
				touch.touchPoints[i].state = set_state;

				touch.touchPoints[i].xRelP = set_x_pixels - touch.touchPoints[i].xP;
				touch.touchPoints[i].yRelP = set_y_pixels - touch.touchPoints[i].yP;
				touch.touchPoints[i].xRel = set_x_height - touch.touchPoints[i].x;
				touch.touchPoints[i].yRel = set_y_height - touch.touchPoints[i].y;

				touch.touchPoints[i].xP = set_x_pixels;
				touch.touchPoints[i].yP = set_y_pixels;
				touch.touchPoints[i].x = set_x_height;
				touch.touchPoints[i].y = set_y_height;

				/*
				//Do we need to init these?
				touch.touchPoints[i].xLocalP = set_x_pixels;
				touch.touchPoints[i].yLocalP = set_y_pixels;
				touch.touchPoints[i].xLocal = set_x_height;
				touch.touchPoints[i].yLocal = set_y_height;
				*/
				return;//success
			}
		}
		cout << "Rae::Input ERROR: Touch ID not found. id: " << set_id << "\n";
	}

	void osScrollEvent( float set_delta_x, float set_delta_y )
	{
		m_changed = true;
		isHandled = false;
		eventType = EventType::Scroll;
		
		mouse.scrollX = set_delta_x;
		mouse.scrollY = set_delta_y;

		emitScrollEvent();
	}
	
	void osKeyEvent( EventType set_event_type, int set_key, int32_t set_unicode )
	{
		//JONDE DEBUG:
		std::cout << "Input.keyEvent() set_key: " << set_key << "\n";

		m_changed = true;
		isHandled = false;
		//window = set_window;
		eventType = set_event_type;
		
		key.value = set_key;
		//key.unicode = set_unicode;
		
		/*
		version(gtk)
		{
			key.value = set_key;
			key.unicode = keyValueToUnicode(set_key);
		}
		version(glfw)
		{
			key.value = set_key;
			key.unicode = cast(dchar) set_key;
		}
		*/
		
		if( set_event_type == EventType::KeyPress )
		{
			//cout << "KEY_PRESS: " << hex << set_key << dec << " unicode: " << set_unicode <<"\n";
			if( key.value < key.keyStatesSize )
			{
				key.keyStates[key.value] = true;
				key.pressedThisFrame[key.value] = true;
			}
			else cout << "ERROR: key.value: " << key.value << " is bigger than keyStatesSize: " << key.keyStatesSize << "\n";
			//Trace.formatln("PRESS. {}", key.keyStates[set_key]);
		}
		else if( set_event_type == EventType::KeyRelease )
		{
			//cout << "KEY_RELEASE: " << hex << set_key << dec << " unicode: " << set_unicode <<"\n";
			if( key.value < key.keyStatesSize )
			{
				key.keyStates[key.value] = false;
				key.releasedThisFrame[key.value] = true;
			}
			else cout << "ERROR: key.value: " << key.value << " is bigger than keyStatesSize: " << key.keyStatesSize << "\n";
			//Trace.formatln("RELEASE. {}", key.keyStates[set_key]);
		}

		emitKeyEvent();
	}

	float set_x_height;
	float set_y_height;
	void osMouseEvent( /*IRectangle* set_window,*/ EventType set_event_type, int set_button, float set_x_pixels, float set_y_pixels, float set_amount = 0.0f )
	{
		assert(set_button >= 0); // "Mouse button is smaller than 0. Platform not supported yet."

		set_x_height = m_screenSystem.pixelsToHeight(set_x_pixels);
		set_y_height = m_screenSystem.pixelsToHeight(set_y_pixels);

		m_changed = true;
		isHandled = false;
		//eventWindow = set_window;
		eventType = set_event_type;
		
		MouseButton mouseButton = intToMouseButton(set_button);
		mouse.eventButton = mouseButton;

		mouse.amount = set_amount;
		
		mouse.doubleClickButton = 0;//Zero this, we can only emit one double click button per event... Is that bad? Propably ok,
		//as there can only be one eventButton per event as well.

		/*JONDE REMOVE
		for (int i = 0; i < 6; ++i)
		{
			MouseButton iButton = intToMouseButton(i);
			if (set_button == i)
			{
				if (eventType == EventType::MouseButtonPress)
				{	
					mouse.setButtonEvent(iButton, eventType);
					mouse.xOnButtonPressP[i] = set_x_pixels;
					mouse.yOnButtonPressP[i] = set_y_pixels;
					mouse.xOnButtonPress[i] = set_x_height;
					mouse.yOnButtonPress[i] = set_y_height;
				}
				else if (eventType == EventType::MouseButtonRelease)
				{
					mouse.setButtonEvent(iButton, eventType);
				}
			}
		}
		*/

		if( eventType == EventType::MouseMotion )
		{
			/////doubleClickValid1 = false;
		}
		else if (eventType == EventType::MouseButtonPress)
		{
			mouse.setButtonEvent(mouseButton, eventType);
			mouse.setButton(mouseButton, true);
			mouse.xOnButtonPressP[set_button] = set_x_pixels;
			mouse.yOnButtonPressP[set_button] = set_y_pixels;
			mouse.xOnButtonPress[set_button] = set_x_height;
			mouse.yOnButtonPress[set_button] = set_y_height;
		}
		else if( eventType == EventType::MouseButtonRelease )
		{
			/*
			//Double click handling is on button release.
			//We measure the time between mouse button releases.
			if( set_button == MouseButton.LEFT )
			{
			
				if( doubleClickValid1 == true )
				{
					if( doubleClickTimer1.stop() <= 0.75 )//Currently 0.75 sec. TODO preferences in class Rae.
					{
						//It's a double click.
						debug(mouse) Trace.formatln("Input.Double click LEFT.");
						mouse.doubleClickButton = set_button;
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
			mouse.setButton((MouseButton)set_button, false);
		}
		//else
		//{
			//Trace.formatln("Input NO PRESS.");
		//}
		
		mouse.xRelP = set_x_pixels - mouse.xP;
		mouse.yRelP = set_y_pixels - mouse.yP;
		mouse.xRel = set_x_height - mouse.x;
		mouse.yRel = set_y_height - mouse.y;
		
		//
		
		mouse.xP = set_x_pixels;
		mouse.yP = set_y_pixels;
		mouse.xLocalP = set_x_pixels;
		mouse.yLocalP = set_y_pixels;
		
		mouse.x = set_x_height;
		mouse.y = set_y_height;
		mouse.xLocal = set_x_height;
		mouse.yLocal = set_y_height;

		if( eventType == EventType::MouseMotion )
		{
			emitMouseMotionEvent();
		}
		else if( eventType == EventType::MouseButtonPress )
		{
			emitMouseButtonPressEvent();
		}
		else if( eventType == EventType::MouseButtonRelease )
		{
			emitMouseButtonReleaseEvent();
		}
	}

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

};//end Input

};//end Rae

#endif //_rae_ui_Input_hpp_

