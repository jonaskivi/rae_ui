#pragma once

#include "rae/core/ISystem.hpp"
#include "rae/core/Types.hpp"

struct NVGcontext;

namespace rae
{

struct Line
{
	Array<vec3> points;
	Color color;
};

struct DebugText
{
	DebugText(const String& text, const Color& color) :
		text(text),
		color(color)
	{
	}

	String text;
	Color color;
};

class DebugSystem : public ISystem
{
public:
	DebugSystem();

	void render3D(uint shaderProgramId);

	void drawLine(const Array<vec3>& points, const Color& color);

	void showDebugText(const String& text);
	void showDebugText(const String& text, const Color& color);
	void render2D(NVGcontext* nanoVG);

private:
	Color m_defaultTextColor = Color(0.5f, 0.5f, 0.5f, 0.75f);

	Array<Line>				m_lines;
	Array<DebugText>		m_debugTexts;
};

extern DebugSystem* g_debugSystem;

}
