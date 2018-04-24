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
class SceneSystem;
class UISystem;
class AssetSystem;
struct Transform;
class Material;
class Mesh;
struct Entity;
class Input;
struct Rectangle;

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
		NVGcontext* nanoVG,
		const Time& time,
		GLFWwindow* setWindow,
		Input& input,
		ScreenSystem& screenSystem,
		AssetSystem& assetSystem,
		UISystem& uiSystem,
		SceneSystem& sceneSystem,
		RayTracer& rayTracer);
	~RenderSystem();

	String name() override { return "RenderSystem"; }

	void initNanoVG(NVGcontext* nanoVG = nullptr);
	void init();

	void checkErrors(const char *file, int line);

	UpdateStatus update() override;

	void beginFrame3D();
	void setViewport(const Rectangle& viewport);
	void render3D(const Scene& scene) override;
	void endFrame3D();

	void renderPicking();
	void render2dBackground();
	void renderRayTracerOutput();

	void beginFrame2D();
	void render2D(NVGcontext* nanoVG) override;
	void endFrame2D();

	// RAE_TODO TEMP:
	void renderImageBuffer(NVGcontext* vg, ImageBuffer<uint8_t>& readBuffer,
		float x, float y, float w, float h);
	ImageBuffer<uint8_t>& getBackgroundImage() { return m_backgroundImage; }

	void renderMesh(
		const Camera& camera,
		const Transform& transform,
		const Color& color,
		const Material& material,
		const Mesh& mesh,
		bool isSelected);

	void renderMeshSingleColor(
		const Camera& camera,
		const Transform& transform,
		const Color& color,
		const Material& material,
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

	const String& fpsString() const { return m_fpsString; }

	void osEventResizeWindow(int width, int height);
	void osEventResizeWindowPixels(int width, int height);

	NVGcontext* nanoVG() { return m_nanoVG; }

protected:
	BasicShader m_basicShader;
	SingleColorShader m_singleColorShader;
	PickingShader m_pickingShader;

	GLFWwindow* m_window;

	NVGcontext* m_nanoVG = nullptr;
	
	// dependencies
	const Time&			m_time;
	Input&				m_input;
	ScreenSystem&		m_screenSystem;
	AssetSystem&		m_assetSystem;
	UISystem&			m_uiSystem;
	SceneSystem&		m_sceneSystem;
	RayTracer&			m_rayTracer;

	Transform*		debugTransform = nullptr;
	Transform*		debugTransform2 = nullptr;

	int		m_nroFrames = 0;
	double	m_fpsTimer = 0.0;
	String	m_fpsString = "fps:";

	RenderMode	m_renderMode = RenderMode::MixedRayTraceRasterize;

	ImageBuffer<uint8_t> m_backgroundImage;
};

}
