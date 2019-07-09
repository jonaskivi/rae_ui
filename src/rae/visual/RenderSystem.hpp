#pragma once

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "rae/core/Types.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae_ray/RayTracer.hpp"

#include "rae/entity/Table.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"
#include "rae/visual/Shader.hpp"

#include "rae/image/ImageBuffer.hpp"

struct NVGcontext;

namespace rae
{

class Time;
class ScreenSystem;
class WindowSystem;
class SceneSystem;
class UISystem;
class AssetSystem;
struct Transform;
class Material;
class Mesh;
struct Entity;
class Input;
struct Rectangle;
class Window;

enum class RenderMode
{
	Rasterize,
	RayTrace,
	MixedRayTraceRasterize,
	Count
};

class RenderSystem : public ISystem
{
public:
	RenderSystem(
		const Time& time,
		Input& input,
		ScreenSystem& screenSystem,
		WindowSystem& windowSystem,
		AssetSystem& assetSystem,
		UISystem& uiSystem,
		SceneSystem& sceneSystem,
		RayTracer& rayTracer);
	~RenderSystem();

	void init();

	void checkErrors(const char *file, int line);

	UpdateStatus update() override;

	void beginFrame3D();
	void setViewport(const Rectangle& viewport, const Window& window);
	void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) override;
	void endFrame3D();

	void renderMeshes(const Scene& scene);
	void renderOutline(const Scene& scene);
	void renderNormals(const Scene& scene);

	void renderPicking(const Window& window);
	void render2dBackground(const Window& window);
	void renderRayTracerOutput(const Window& window);

	void beginFrame2D(const Window& window);
	void render2D(UIScene& uiScene, NVGcontext* nanoVG) override;
	void endFrame2D(const Window& window);

	// RAE_TODO TEMP:
	void renderImageBuffer(NVGcontext* vg, ImageBuffer<uint8_t>& readBuffer,
		float x, float y, float w, float h);
	ImageBuffer<uint8_t>& getBackgroundImage() { return m_backgroundImage; }

	void renderMesh(
		const Camera& camera,
		const Transform& transform,
		const Material& material,
		const Mesh& mesh);

	void renderMeshSingleColor(
		const Camera& camera,
		const Transform& transform,
		const Color& color,
		const Mesh& mesh);

	void renderMeshOutline(
		const Camera& camera,
		const Transform& transform,
		const Color& color,
		const Mesh& mesh);

	void renderMeshNormals(
		const Camera& camera,
		const Transform& transform,
		const Color& color,
		const Mesh& mesh);

	void renderMeshPicking(
		const Camera& camera,
		const Transform& transform,
		const Mesh& mesh,
		Id id);

	// Temp before we get keyboard Input class
	void clearImageRenderer();
	RenderMode toggleRenderMode()
	{
		return m_renderMode = (RenderMode) Utils::wrapEnum(((int)m_renderMode) + 1, (int)RenderMode::Count);
	}

	bool toggleRenderNormals() { return m_renderNormals = !m_renderNormals; }

	const String& fpsString() const { return m_fpsString; }

protected:
	BasicShader m_basicShader;
	SingleColorShader m_singleColorShader;
	OutlineShader m_outlineShader;
	PickingShader m_pickingShader;

	// dependencies
	const Time&			m_time;
	Input&				m_input;
	ScreenSystem&		m_screenSystem;
	WindowSystem&		m_windowSystem;
	AssetSystem&		m_assetSystem;
	UISystem&			m_uiSystem;
	SceneSystem&		m_sceneSystem;
	RayTracer&			m_rayTracer;

	Transform*		debugTransform = nullptr;
	Transform*		debugTransform2 = nullptr;

	int		m_nroFrames = 0;
	double	m_fpsTimer = 0.0;
	String	m_fpsString = "fps:";

	RenderMode	m_renderMode = RenderMode::Rasterize;
	bool m_renderNormals = false;

	ImageBuffer<uint8_t> m_backgroundImage;
};

}
