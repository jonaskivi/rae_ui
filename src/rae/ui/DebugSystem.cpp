#include "rae/ui/DebugSystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "nanovg.h"

#include "rae/visual/CameraSystem.hpp"
#include "rae/visual/Mesh.hpp"

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


DebugSystem::DebugSystem(const CameraSystem& cameraSystem) :
	m_cameraSystem(cameraSystem)
{
	g_debugSystem = this;

	loguru::add_callback("user_callback", loguruLoggerCallback, nullptr,
		loguru::Verbosity_INFO, loguruCallbackClose, loguruCallbackFlush);

	LOG_F(INFO, "Added loguru logging callback.");

	if (m_singleColorShader.load() == 0)
	{
		exit(0);
	}

	LOG_F(INFO, "Loaded shader.");
}

DebugSystem::~DebugSystem()
{
	loguru::remove_callback("user_callback");
}

void DebugSystem::render3D()
{
	m_singleColorShader.use();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	const Camera& camera = m_cameraSystem.getCurrentCamera();

	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix();// * modelMatrix;

	m_singleColorShader.pushModelViewMatrix(combinedMatrix);

	if (m_lineMeshes.size() < m_lines.size())
	{
		m_lineMeshes.reserve(m_lines.size());
	}

	
	for (int i = 0; i < (int)m_lines.size(); ++i)
	{
		auto&& line = m_lines[i];

		if (m_lineMeshes.size() <= i)
		{
			m_lineMeshes.emplace_back();
		}

		auto&& lineMesh = m_lineMeshes[i];

		m_singleColorShader.pushColor(line.color);
		lineMesh.generateLinesFromVertices(line.points);
		lineMesh.createVBOs(GL_DYNAMIC_DRAW);
		lineMesh.renderLines(m_singleColorShader.getProgramId());
	}
	m_lines.clear();
}

void DebugSystem::drawLine(const Array<vec3>& points, const Color& color)
{
	m_lines.emplace_back(Line{ points, color });
}

void DebugSystem::drawLine(const Line& line)
{
	m_lines.emplace_back(line);
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

void DebugSystem::render2D(NVGcontext* nanoVG)
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

	int i = Utils::clamp(int(m_logTexts.size()) - logLines, 0, (int)m_logTexts.size()-1);
	for (; i < (int)m_logTexts.size(); ++i)
	{
		auto&& text = m_logTexts[i];

		nvgFillColor(nanoVG, nvgRGBAf(text.color.r, text.color.g, text.color.b, text.color.a));
		nvgText(nanoVG, 10.0f, vertPos, text.text.c_str(), nullptr);
		vertPos += lineHeight;
	}
}
