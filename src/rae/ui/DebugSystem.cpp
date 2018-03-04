#include "rae/ui/DebugSystem.hpp"

#include "nanovg.h"

using namespace rae;

rae::DebugSystem* rae::g_debugSystem = nullptr;

DebugSystem::DebugSystem()
{
	g_debugSystem = this;
}

void DebugSystem::render3D(uint shaderProgramId)
{
	/*
	RAE_TODO 
	for (auto&& line : m_lines)
	{
		Mesh lineMesh;
		lineMesh.generateLinesFromVertices(line);
		lineMesh.createVBOs();
		lineMesh.render(shaderProgramId);
		lineMesh.freeVBOs();
	}
	*/
}

void DebugSystem::drawLine(const Array<vec3>& points, const Color& color)
{
	m_lines.emplace_back(Line{ points, color });
}

void DebugSystem::showDebugText(const String& text)
{
	showDebugText(text, m_defaultTextColor);
}

void DebugSystem::showDebugText(const String& text, const Color& color)
{
	m_debugTexts.emplace_back(DebugText(text, color));
}

void DebugSystem::render2D(NVGcontext* nanoVG)
{
	nvgFontFace(nanoVG, "sans");

	float vertPos = 10.0f;

	nvgFontSize(nanoVG, 18.0f);
	nvgTextAlign(nanoVG, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	//RAE_TODO REMOVE nvgFillColor(nanoVG, nvgRGBA(128, 128, 128, 192));

	for (auto&& text : m_debugTexts)
	{
		nvgFillColor(nanoVG, nvgRGBAf(text.color.r, text.color.g, text.color.b, text.color.a));
		nvgText(nanoVG, 10.0f, vertPos, text.text.c_str(), nullptr);
		vertPos += 20.0f;
	}
	m_debugTexts.clear();
}
