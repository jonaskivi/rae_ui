#pragma once

#include "rae/core/Types.hpp"

namespace rae
{

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

	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
};

struct Draggable
{
};

struct StackLayout
{
	StackLayout(){}

	StackLayout(OrientationType orientationType) :
		orientationType(orientationType)
	{
	}

	OrientationType orientationType = OrientationType::Vertical;
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

struct Text
{
	Text(){}
	Text(const String& text) :
		text(text)
	{
	}

	operator String&() { return text; }
	operator String() const { return text; }

	String text;
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
