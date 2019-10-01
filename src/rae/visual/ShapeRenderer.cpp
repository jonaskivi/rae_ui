#include "rae/visual/ShapeRenderer.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "rae/visual/Mesh.hpp"
#include "rae/visual/Camera.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/scene/SceneSystem.hpp"

using namespace rae;

rae::ShapeRenderer* rae::g_shapeRenderer = nullptr;

ShapeRenderer::ShapeRenderer()
{
	g_shapeRenderer = this;
}

void ShapeRenderer::updateWhenDisabled()
{
	m_lines.clear();
}

void LineBatch::prepareRender3D()
{
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

		lineMesh.generateLinesFromVertices(line.points);
		lineMesh.createVBOs(GL_DYNAMIC_DRAW);
	}
}

void ShapeRenderer::prepareRender3D(Scene& scene)
{
	m_lines.prepareRender3D();
	m_noDepthLines.prepareRender3D();
}

void LineBatch::render3D(SingleColorShader& singleColorShader) const
{
	assert(m_lines.size() <= m_lineMeshes.size());

	for (int i = 0; i < (int)m_lines.size(); ++i)
	{
		const auto& line = m_lines[i];
		const auto& lineMesh = m_lineMeshes[i];

		singleColorShader.pushColor(line.color);
		lineMesh.renderLines(singleColorShader.getProgramId());
	}
}

void ShapeRenderer::render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) const
{
	const Camera& camera = scene.cameraSystem().currentCamera();

	auto& singleColorShader = renderSystem.modifySingleColorShader();
	singleColorShader.use();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix();

	singleColorShader.pushModelViewMatrix(combinedMatrix);

	m_lines.render3D(singleColorShader);

	glDisable(GL_DEPTH_TEST);

	m_noDepthLines.render3D(singleColorShader);
}

void ShapeRenderer::onFrameEnd()
{
	m_lines.clear();
	m_noDepthLines.clear();
}

void LineBatch::drawLine(const Array<vec3>& points, const Color& color)
{
	m_lines.emplace_back(Line{ points, color });
}

void LineBatch::drawLine(const Line& line)
{
	m_lines.emplace_back(line);
}

void ShapeRenderer::drawLine(const Array<vec3>& points, const Color& color, DrawType drawType)
{
	if (drawType == DrawType::Normal)
	{
		m_lines.drawLine(points, color);
	}
	else if (drawType == DrawType::NoDepth)
	{
		m_noDepthLines.drawLine(points, color);
	}
}

void ShapeRenderer::drawLine(const Line& line, DrawType drawType)
{
	if (drawType == DrawType::Normal)
	{
		m_lines.drawLine(line);
	}
	else if (drawType == DrawType::NoDepth)
	{
		m_noDepthLines.drawLine(line);
	}
}

void ShapeRenderer::drawLineSegment(const LineSegment& line, const Color& color, DrawType drawType)
{
	drawLine({line.start(), line.end()}, color, drawType);
}

void ShapeRenderer::drawLineBox(const Box& box, const Color& color, DrawType drawType)
{
	// 4 is the minimum amount of lines for the edges of the cube.
	// https://math.stackexchange.com/questions/253253/tracing-the-edges-of-a-cube-with-the-minimum-pencil-lifts
	drawLine(
	{
		box.corner(0),
		box.corner(1),
		box.corner(3),
		box.corner(2),
		box.corner(0),
		box.corner(4),
		box.corner(5),
		box.corner(7),
		box.corner(6),
		box.corner(4)
	}, color, drawType);
	drawLine(
	{
		box.corner(2),
		box.corner(6)
	}, color, drawType);
	drawLine(
	{
		box.corner(3),
		box.corner(7)
	}, color, drawType);
	drawLine(
	{
		box.corner(1),
		box.corner(5)
	}, color, drawType);
}

void ShapeRenderer::drawCircle(
	const vec3& center,
	float radius,
	const qua& rotation,
	const Color& color,
	DrawType drawType,
	int resolution)
{
	if (resolution <= 1)
		return;

	Array<vec3> points;
	points.reserve(resolution);

	for (int i = 0; i < resolution; ++i)
	{
		float angle = Math::Tau * (float(i) / float(resolution - 1));
		// Circle lies on the YZ plane.
		vec3 direction = vec3(0.0f, sinf(angle), cosf(angle));
		vec3 position = center + (glm::toMat3(rotation) * (radius * direction));
		points.emplace_back(position);
	}

	drawLine(points, color, drawType);
}

void ShapeRenderer::drawArch(
	const vec3& center,
	float angleStart,
	float angleEnd,
	float radius,
	const qua& rotation,
	const Color& color,
	DrawType drawType,
	int resolution,
	bool connected)
{
	float angle = (angleEnd - angleStart);

	Array<vec3> points;
	points.reserve(resolution);
	for (int i = 0; i < resolution; ++i)
	{
		float angleStep = angle * (float(i) / float(resolution - 1));
		// Notice that we invert Y here
		vec3 direction = vec3(0.0f, -sinf(angleStep + angleStart), cosf(angleStep + angleStart));
		vec3 position = center + (glm::toMat3(rotation) * (radius * direction));
		points.emplace_back(position);
	}

	if (points.size() > 1)
	{
		drawLine(points, color, drawType);

		if (connected)
		{
			drawLine({center, points[0]}, color, drawType);
			drawLine({center, points[points.size() - 1]}, color, drawType);
		}
	}
}

void ShapeRenderer::drawCylinder(
	const vec3& center,
	const vec3& halfExtents,
	const qua& rotation,
	const Color& color,
	DrawType drawType,
	int resolution)
{
	float radius = halfExtents.y;

	drawCircle(center + (rotation * vec3( halfExtents.x, 0, 0)), radius, rotation, color, drawType, resolution);
	drawCircle(center + (rotation * vec3(-halfExtents.x, 0, 0)), radius, rotation, color, drawType, resolution);

	vec3 p0 = center + (rotation * vec3( halfExtents.x,  halfExtents.y, 0));
	vec3 p1 = center + (rotation * vec3(-halfExtents.x,  halfExtents.y, 0));
	vec3 p2 = center + (rotation * vec3( halfExtents.x, -halfExtents.y, 0));
	vec3 p3 = center + (rotation * vec3(-halfExtents.x, -halfExtents.y, 0));

	vec3 p4 = center + (rotation * vec3( halfExtents.x, 0,  halfExtents.z));
	vec3 p5 = center + (rotation * vec3(-halfExtents.x, 0,  halfExtents.z));
	vec3 p6 = center + (rotation * vec3( halfExtents.x, 0, -halfExtents.z));
	vec3 p7 = center + (rotation * vec3(-halfExtents.x, 0, -halfExtents.z));

	drawLine({vec3(p0.x, p0.y, p0.z), vec3(p1.x, p1.y, p1.z)}, color, drawType);
	drawLine({vec3(p2.x, p2.y, p2.z), vec3(p3.x, p3.y, p3.z)}, color, drawType);
	drawLine({vec3(p4.x, p4.y, p4.z), vec3(p5.x, p5.y, p5.z)}, color, drawType);
	drawLine({vec3(p6.x, p6.y, p6.z), vec3(p7.x, p7.y, p7.z)}, color, drawType);
}
