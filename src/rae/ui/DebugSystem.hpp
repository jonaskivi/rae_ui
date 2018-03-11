#pragma once

#include "rae/core/ISystem.hpp"
#include "rae/core/Types.hpp"
#include "rae/visual/Shader.hpp"
#include "rae/visual/Material.hpp"
#include "rae/visual/Mesh.hpp"

struct NVGcontext;

namespace rae
{

class CameraSystem;

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
	DebugSystem(const CameraSystem& cameraSystem);

	void render3D() override;

	void drawLine(const Array<vec3>& points, const Color& color);
	void drawLine(const Line& line);

	void showDebugText(const String& text);
	void showDebugText(const String& text, const Color& color);
	void render2D(NVGcontext* nanoVG);

private:
	const CameraSystem& m_cameraSystem;

	Color m_defaultTextColor = Color(0.5f, 0.5f, 0.5f, 0.75f);

	Material m_material;
	SingleColorShader m_singleColorShader;

	Array<Line>				m_lines;
	Array<Mesh>				m_lineMeshes;
	Array<DebugText>		m_debugTexts;
};

extern DebugSystem* g_debugSystem;

}
