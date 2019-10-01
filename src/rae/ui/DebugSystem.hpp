#pragma once

#include "loguru/loguru.hpp"
#include "rae/core/ISystem.hpp"

struct NVGcontext;

namespace rae
{

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

class UIScene;

class DebugSystem : public ISystem
{
public:
	DebugSystem();
	~DebugSystem();

	void updateWhenDisabled() override;

	void render2D(UIScene& uiScene, NVGcontext* nanoVG) override;

	void showDebugText(const String& text);
	void showDebugText(const String& text, const Color& color);

	void log(const String& text);
	void log(const String& text, const Color& color);

private:
	static void loguruLoggerCallback(void* user_data, const loguru::Message& message);
	static void loguruCallbackFlush(void* user_data);
	static void loguruCallbackClose(void* user_data);

	Color m_defaultTextColor = Color(0.5f, 0.5f, 0.5f, 0.75f);
	Color m_defaultLogColor = Color(1.0f, 0.0f, 1.0f, 0.75f);

	Array<DebugText>		m_debugTexts;
	Array<DebugText>		m_logTexts;
};

extern DebugSystem* g_debugSystem;

}
