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

	void drawLine(const Array<vec3>& points, const Color& color);
	void drawLine(const Line& line);
	void drawLineBox(const Box& box, const Color& color);

	void drawLineSegment(const LineSegment& line, const Color& color);

	void drawCircle(
		const vec3& center,
		float radius,
		const qua& rotation,
		const Color& color,
		int resolution = 32);

	void drawArch(
		const vec3& center,
		float angleStart,
		float angleEnd,
		float radius,
		const qua& rotation,
		const Color& color,
		int resolution = 32,
		bool connected = false);

	void drawCylinder(
		const vec3& center,
		const vec3& halfExtents,
		const qua& rotation,
		const Color& color,
		int resolution = 32);

private:
	Material m_material;

	Array<Line>				m_lines;
	Array<Mesh>				m_lineMeshes;
};

// Global active scene shapeRenderer for debug drawing.
extern ShapeRenderer* g_shapeRenderer;

}
