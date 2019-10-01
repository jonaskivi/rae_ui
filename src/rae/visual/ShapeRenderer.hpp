#pragma once

#include "rae/core/ISystem.hpp"
#include "rae/core/Types.hpp"
#include "rae/visual/Shader.hpp"
#include "rae/visual/Material.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Plane.hpp" // LineSegment Capsule

namespace rae
{

class Scene;
class RenderSystem;
class SingleColorShader;

struct LineBatch
{
	void prepareRender3D();
	void render3D(SingleColorShader& singleColorShader) const;
	void clear() { m_lines.clear(); }
	void drawLine(const Array<vec3>& points, const Color& color);
	void drawLine(const Line& line);

	Array<Line>				m_lines;
	Array<Mesh>				m_lineMeshes;
};

enum DrawType
{
	Normal,
	NoDepth
};

class ShapeRenderer : public ISystem
{
public:
	ShapeRenderer();

	void updateWhenDisabled() override;

	void prepareRender3D(Scene& scene);
	void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) const;
	// NOT OVERRIDDEN because of const:
	//void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) override;
	void onFrameEnd() override;

	void drawLine(const Array<vec3>& points, const Color& color, DrawType drawType = DrawType::Normal);
	void drawLine(const Line& line, DrawType drawType = DrawType::Normal);
	void drawLineBox(const Box& box, const Color& color, DrawType drawType = DrawType::Normal);

	void drawLineSegment(const LineSegment& line, const Color& color, DrawType drawType = DrawType::Normal);

	void drawCircle(
		const vec3& center,
		float radius,
		const qua& rotation,
		const Color& color,
		DrawType drawType = DrawType::Normal,
		int resolution = 32);

	void drawArch(
		const vec3& center,
		float angleStart,
		float angleEnd,
		float radius,
		const qua& rotation,
		const Color& color,
		DrawType drawType = DrawType::Normal,
		int resolution = 32,
		bool connected = false);

	void drawCylinder(

		const vec3& center,
		const vec3& halfExtents,
		const qua& rotation,
		const Color& color,
		DrawType drawType = DrawType::Normal,
		int resolution = 32);

private:
	Material m_material;

	LineBatch m_lines;
	LineBatch m_noDepthLines;
};

// Global active scene shapeRenderer for debug drawing.
extern ShapeRenderer* g_shapeRenderer;

}
