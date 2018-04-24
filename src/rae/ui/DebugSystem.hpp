#pragma once

#include "loguru/loguru.hpp"

#include "rae/core/ISystem.hpp"
#include "rae/core/Types.hpp"
#include "rae/visual/Shader.hpp"
#include "rae/visual/Material.hpp"
#include "rae/visual/Mesh.hpp"

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

class Scene;

class DebugSystem : public ISystem
{
public:
	DebugSystem();
	~DebugSystem();

	void render3D(const Scene& scene) override;

	void drawLine(const Array<vec3>& points, const Color& color);
	void drawLine(const Line& line);

	void showDebugText(const String& text);
	void showDebugText(const String& text, const Color& color);
	void render2D(NVGcontext* nanoVG) override;

	void log(const String& text);
	void log(const String& text, const Color& color);

private:
	static void loguruLoggerCallback(void* user_data, const loguru::Message& message);
	static void loguruCallbackFlush(void* user_data);
	static void loguruCallbackClose(void* user_data);

	Color m_defaultTextColor = Color(0.5f, 0.5f, 0.5f, 0.75f);
	Color m_defaultLogColor = Color(1.0f, 0.0f, 1.0f, 0.75f);

	Material m_material;
	SingleColorShader m_singleColorShader;

	Array<Line>				m_lines;
	Array<Mesh>				m_lineMeshes;
	Array<DebugText>		m_debugTexts;
	Array<DebugText>		m_logTexts;
};

extern DebugSystem* g_debugSystem;

}
