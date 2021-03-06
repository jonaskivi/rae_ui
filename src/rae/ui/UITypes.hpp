#pragma once

#include <functional>

#include "rae/core/Types.hpp"
#include "rae/visual/Box.hpp"

namespace rae
{

enum class HorizontalTextAlignment
{
	Left,
	Center,
	Right
};

enum class VerticalTextAlignment
{
	Top,
	Center,
	Bottom
};

enum class OrientationType
{
	Horizontal,
	Vertical
};

struct Rectangle
{
	Rectangle(){}
	Rectangle(float x, float y, float width, float height) :
		x(x), y(y), width(width), height(height)
	{}

	Rectangle& operator*=(float value)
	{
		x *= value;
		y *= value;
		width *= value;
		height *= value;
		return *this;
	}

	bool hit(const vec2& position) const
	{
		if (position.x <= right() &&
			position.x >= x &&
			position.y <= bottom() &&
			position.y >= y)
		{
			return true;
		}
		return false;
	}

	float left() const { return x; }
	float right() const { return x + width; }
	float top() const { return y; }
	float bottom() const { return y + height; }

	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
};

struct UIWidgetRenderer
{
	UIWidgetRenderer() {}

	UIWidgetRenderer(std::function<void(Id)> renderFunction) :
		renderFunction(renderFunction)
	{
	}

	void render(Id id) const
	{
		renderFunction(id);
	}

	std::function<void(Id)> renderFunction;
};

struct UIWidgetUpdater
{
	UIWidgetUpdater() {}

	UIWidgetUpdater(std::function<void(Id)> updateFunction) :
		updateFunction(updateFunction)
	{
	}

	void update(Id id) const
	{
		updateFunction(id);
	}

	std::function<void(Id)> updateFunction;
};

struct TextWidget {};
struct TextBox
{
	bool editing = false;
	int cursorIndex = 0;
	float cursorPixels = 0.0f;
};

struct Draggable {};

struct StackLayout
{
	StackLayout(){}

	StackLayout(OrientationType orientationType) :
		orientationType(orientationType)
	{
	}

	OrientationType orientationType = OrientationType::Vertical;
};

struct GridLayout
{
	GridLayout(){}
	GridLayout(int xCells, int yCells) :
		xCells(xCells),
		yCells(yCells)
	{
	}

	int xCells = 2;
	int yCells = 2;
};

enum class MaximizerState
{
	Normal,
	Maximized
};

// A maximizer is a sort of layout helper, which has two states: Normal and maximized.
// When changing to maximized state, it stores the entity's previous transform. Useful for example
// to maximize a viewport within a window.
struct Maximizer
{
	bool isMaximized() const { return maximizerState == MaximizerState::Maximized; }

	MaximizerState maximizerState = MaximizerState::Normal;
	vec3 storedNormalStatePosition;
	Box storedNormalStateBox;
	Pivot storedNormalStatePivot;
};

struct Active
{
	Active(){}
	Active(bool active) :
		active(active)
	{
	}

	operator bool&() { return active; }
	operator bool() const { return active; }

	bool active = false;
};

struct Visible
{
	Visible(){}
	Visible(bool visible) :
		visible(visible)
	{
	}

	operator bool&() { return visible; }
	operator bool() const { return visible; }

	bool visible = true;
};

struct Text
{
	Text(){}
	Text(const String& text,
		float fontSize = 18.0f,
		HorizontalTextAlignment horizontalAlignment = HorizontalTextAlignment::Center,
		VerticalTextAlignment verticalAlignment = VerticalTextAlignment::Center) :
			text(text),
			fontSize(fontSize),
			horizontalAlignment(horizontalAlignment),
			verticalAlignment(verticalAlignment)
	{
	}

	operator String&() { return text; }
	operator String() const { return text; }

	String text;
	float fontSize = 18.0f;
	HorizontalTextAlignment horizontalAlignment = HorizontalTextAlignment::Center;
	VerticalTextAlignment verticalAlignment = VerticalTextAlignment::Center;
};

struct Margin
{
	Margin(){}

	Margin(float left, float right, float up, float down) :
		left(left),
		right(right),
		up(up),
		down(down)
	{
	}

	Margin(float all) :
		left(all),
		right(all),
		up(all),
		down(all)
	{
	}

	Margin(float leftRight, float upDown) :
		left(leftRight),
		right(leftRight),
		up(upDown),
		down(upDown)
	{
	}

	float left = 0.0f;
	float right = 0.0f;
	float up = 0.0f;
	float down = 0.0f;
};

struct Keyline
{
	Keyline(){}

	Keyline(OrientationType orientationType, float relativePosition) :
		orientationType(orientationType),
		relativePosition(relativePosition)
	{
	}

	OrientationType orientationType = OrientationType::Vertical;
	float relativePosition = 0.5f;
};

struct KeylineLink
{
	KeylineLink(){}
	KeylineLink(Id keylineId) :
		keylineId(keylineId)
	{
	}

	Id keylineId;
};

// This is a proxy entity for the actual window which lives in the WindowSystem
struct WindowEntity
{
};

struct Panel
{
};

struct Viewport
{
	Viewport(){}
	Viewport(int sceneIndex) :
		sceneIndex(sceneIndex)
	{}

	int sceneIndex = 0;
	bool active = false;
};

using ImageLink = asset::Id;

enum class PanelThemeColorKey
{
	Background,
	Hover,
	Count
};

enum class ViewportThemeColorKey
{
	Line,
	LineActive,
	Count
};

}
