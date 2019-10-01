#include "rae/ui/DebugSystem.hpp"

#include "nanovg.h"

#include "rae/core/Math.hpp"

using namespace rae;

rae::DebugSystem* rae::g_debugSystem = nullptr;

void DebugSystem::loguruLoggerCallback(void* user_data, const loguru::Message& message)
{
	//printf("Custom callback: %s%s\n", message.prefix, message.message);
	//reinterpret_cast<CallbackTester*>(user_data)->num_print += 1;
	g_debugSystem->log(message.message);
}

void DebugSystem::loguruCallbackFlush(void* user_data)
{
	//printf("Custom callback flush\n");
	//reinterpret_cast<CallbackTester*>(user_data)->num_flush += 1;
}

void DebugSystem::loguruCallbackClose(void* user_data)
{
	//printf("Custom callback close\n");
	//reinterpret_cast<CallbackTester*>(user_data)->num_close += 1;
}

DebugSystem::DebugSystem()
{
	setIsEnabled(false);

	g_debugSystem = this;

	loguru::add_callback("user_callback", loguruLoggerCallback, nullptr,
		loguru::Verbosity_INFO, loguruCallbackClose, loguruCallbackFlush);

	//LOG_F(INFO, "Added loguru logging callback.");
}

DebugSystem::~DebugSystem()
{
	loguru::remove_callback("user_callback");

	g_debugSystem = nullptr;
}

void DebugSystem::showDebugText(const String& text)
{
	showDebugText(text, m_defaultTextColor);
}

void DebugSystem::showDebugText(const String& text, const Color& color)
{
	m_debugTexts.emplace_back(DebugText(text, color));
}

void DebugSystem::log(const String& text)
{
	log(text, m_defaultLogColor);
}

void DebugSystem::log(const String& text, const Color& color)
{
	m_logTexts.emplace_back(DebugText(text, color));
}

void DebugSystem::updateWhenDisabled()
{
	m_debugTexts.clear();
}

void DebugSystem::render2D(UIScene& uiScene, NVGcontext* nanoVG)
{
	const float lineHeight = 18.0f;

	nvgFontFace(nanoVG, "sans");

	float vertPos = 10.0f;

	nvgFontSize(nanoVG, 18.0f);
	nvgTextAlign(nanoVG, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	//RAE_TODO REMOVE nvgFillColor(nanoVG, nvgRGBA(128, 128, 128, 192));

	for (auto&& text : m_debugTexts)
	{
		nvgFillColor(nanoVG, nvgRGBAf(text.color.r, text.color.g, text.color.b, text.color.a));
		nvgText(nanoVG, 10.0f, vertPos, text.text.c_str(), nullptr);
		vertPos += lineHeight;
	}
	m_debugTexts.clear();

	const int logLines = 24;

	int i = Math::clamp(int(m_logTexts.size()) - logLines, 0, (int)m_logTexts.size()-1);
	for (; i < (int)m_logTexts.size(); ++i)
	{
		auto&& text = m_logTexts[i];

		nvgFillColor(nanoVG, nvgRGBAf(text.color.r, text.color.g, text.color.b, text.color.a));
		nvgText(nanoVG, 10.0f, vertPos, text.text.c_str(), nullptr);
		vertPos += lineHeight;
	}
}
